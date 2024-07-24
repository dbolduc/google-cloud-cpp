// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "generator/internal/stub_rest_generator.h"
#include "generator/internal/codegen_utils.h"
#include "generator/internal/http_option_utils.h"
#include "generator/internal/longrunning.h"
#include "generator/internal/predicate_utils.h"
#include "generator/internal/printer.h"
#include "google/cloud/internal/absl_str_join_quiet.h"
#include "google/cloud/internal/algorithm.h"
#include "google/cloud/log.h"
#include "absl/strings/str_split.h"
#include <google/protobuf/compiler/cpp/names.h>
#include <google/protobuf/descriptor.h>

using ::google::protobuf::compiler::cpp::FieldName;

namespace google {
namespace cloud {
namespace generator_internal {
namespace {

std::string AdaptValue(std::string accessor,
                       protobuf::FieldDescriptor::CppType type) {
  if (type == protobuf::FieldDescriptor::CPPTYPE_STRING) return accessor;
  if (type == protobuf::FieldDescriptor::CPPTYPE_BOOL) {
    return "(" + accessor + R"""( ? "1" : "0"))""";
  }
  return "std::to_string(" + accessor + ")";
}

std::string QueryParameterCode(
    protobuf::FieldDescriptor::CppType type,
    google::protobuf::Descriptor const* proto, std::string const& name,
    std::string const& value, int depth,
    std::vector<std::string> const& param_field_names,
    std::string const& body) {
  auto const indent = std::string(2 * ++depth, ' ');
  if (internal::Contains(param_field_names, name)) {
    return {};
    // TODO(dbolduc) : Clean up.
    return indent + "// DEBUG : Skipping known field name: " + name + "\n";
  }

  if (depth >= 10) {
    return indent + "// Cutting off recursion...\n";
  }
  if (type != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    return indent + "params.push_back({\"" + name + "\", " +
           AdaptValue(value, type) + "});\n";
  }

  std::string code;
  for (auto i = 0; i != proto->field_count(); ++i) {
    auto const* field = proto->field(i);
    auto const type = field->cpp_type();
    std::string const sep = name.empty() ? "" : ".";
    std::string next_value = "v" + std::to_string(depth);
    auto const next_name = name + sep + field->name();
    if (next_name == body) {
      // This field is already sent in the body of the payload. We should not
      // include it or any subfields as query parameters.
      continue;
    }
    auto const accessor = FieldName(field) + "()";
    auto const* next_proto = field->message_type();
    if (field->options().deprecated()) {
      code += indent + "// Skipping deprecated field: " + next_name + "\n";
      continue;
    }

    std::string pre_code;
    std::string inner_code;
    std::string post_code;
    if (field->is_repeated()) {
      // TODO(#10176): Consider adding support for repeated simple fields.
      // TODO(dbolduc) : Consider `continue`-ing here to break up the PR.
      pre_code += indent + "for (auto const& " + next_value + " : " + value +
                  "." + accessor + ") {\n";
      post_code += indent + "}\n";
    } else if (type == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      // TODO(dbolduc) : Consider `continue`-ing here to break up the PR.
      pre_code += indent + "if (" + value + ".has_" + accessor + ") {\n";
      pre_code += indent + "  auto const& " + next_value + " = " + value + "." +
                  accessor + ";\n";
      post_code += indent + "}\n";
    } else {
      next_value = value + "." + accessor;
    }
    code += pre_code;
    code += QueryParameterCode(type, next_proto, next_name, next_value, depth,
                               param_field_names, body);
    code += post_code;
  }
  return code;
}

std::string QueryParameterCode(
    google::protobuf::MethodDescriptor const& method) {
  std::string code;
  code += "  std::vector<std::pair<std::string, std::string>> params;\n";

  auto info = ParseHttpExtension(method);
  // All request fields are included in the body of the HTTP request. None of
  // them should be query parameters.
  if (info.body == "*") return code;

  std::vector<std::string> param_field_names;
  param_field_names.reserve(info.field_substitutions.size());
  for (auto const& p : info.field_substitutions) {
    param_field_names.push_back(p.first);
  }

  return code + QueryParameterCode(protobuf::FieldDescriptor::CPPTYPE_MESSAGE,
                                   method.input_type(), "", "request", 0,
                                   param_field_names, info.body);
}

}  // namespace

StubRestGenerator::StubRestGenerator(
    google::protobuf::ServiceDescriptor const* service_descriptor,
    VarsDictionary service_vars,
    std::map<std::string, VarsDictionary> service_method_vars,
    google::protobuf::compiler::GeneratorContext* context)
    : ServiceCodeGenerator("stub_rest_header_path", "stub_rest_cc_path",
                           service_descriptor, std::move(service_vars),
                           std::move(service_method_vars), context) {}

Status StubRestGenerator::GenerateHeader() {
  HeaderPrint(CopyrightLicenseFileHeader());
  HeaderPrint(R"""(
// Generated by the Codegen C++ plugin.
// If you make any local changes, they will be lost.
// source: $proto_file_name$

#ifndef $header_include_guard$
#define $header_include_guard$
)""");

  HeaderPrint("\n");
  HeaderLocalIncludes({"google/cloud/internal/rest_client.h",
                       "google/cloud/completion_queue.h",
                       "google/cloud/internal/rest_context.h",
                       "google/cloud/status_or.h", "google/cloud/version.h"});
  std::vector<std::string> additional_pb_header_paths =
      absl::StrSplit(vars("additional_pb_header_paths"), absl::ByChar(','));
  HeaderSystemIncludes(additional_pb_header_paths);
  HeaderSystemIncludes({vars("proto_header_path"),
                        HasLongrunningMethod()
                            ? vars("longrunning_operation_include_header")
                            : "",
                        "memory"});

  auto result = HeaderOpenNamespaces(NamespaceType::kInternal);
  if (!result.ok()) return result;

  // Abstract interface Stub base class
  HeaderPrint(R"""(
class $stub_rest_class_name$ {
 public:
  virtual ~$stub_rest_class_name$() = default;
)""");

  for (auto const& method : methods()) {
    if (IsStreaming(method)) continue;
    if (!HasHttpAnnotation(method)) continue;
    if (IsLongrunningOperation(method)) {
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual future<StatusOr<$response_type$>> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) = 0;
)""");
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual StatusOr<$response_type$> $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) = 0;
)""");

    } else {
      if (IsResponseTypeEmpty(method)) {
        HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual Status $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) = 0;
)""");
      } else {
        HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual StatusOr<$response_type$> $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) = 0;
)""");
      }
    }
  }

  for (auto const& method : async_methods()) {
    // No streaming RPCs for REST, and Longrunning is already taken care of.
    if (IsStreaming(method) || IsLongrunningOperation(method)) continue;
    if (!HasHttpAnnotation(method)) continue;
    if (IsResponseTypeEmpty(method)) {
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual future<Status> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) = 0;
)""");
    } else {
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  virtual future<StatusOr<$response_type$>> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) = 0;
)""");
    }
  }

  if (HasLongrunningMethod()) {
    // long running operation support methods
    if (HasGRPCLongrunningOperation()) {
      HeaderPrint(
          R"""(
  virtual future<StatusOr<google::longrunning::Operation>> AsyncGetOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      google::longrunning::GetOperationRequest const& request) = 0;

  virtual future<Status> AsyncCancelOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      google::longrunning::CancelOperationRequest const& request) = 0;
)""");
    } else {
      HeaderPrint(
          R"""(
  virtual future<StatusOr<$longrunning_response_type$>> AsyncGetOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $longrunning_get_operation_request_type$ const& request) = 0;

  virtual future<Status> AsyncCancelOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $longrunning_cancel_operation_request_type$ const& request) = 0;
)""");
    }
  }

  // close abstract interface Stub base class
  HeaderPrint("};\n");

  // default stub class
  HeaderPrint(R"""(
class Default$stub_rest_class_name$ : public $stub_rest_class_name$ {
 public:
  ~Default$stub_rest_class_name$() override = default;

  explicit Default$stub_rest_class_name$(Options options);
  Default$stub_rest_class_name$(
      std::shared_ptr<rest_internal::RestClient> service,)""");
  if (HasLongrunningMethod()) {
    HeaderPrint(R"""(
      std::shared_ptr<rest_internal::RestClient> operations,)""");
  }
  HeaderPrint(R"""(
      Options options);
)""");

  for (auto const& method : methods()) {
    if (HasHttpAnnotation(method) && IsNonStreaming(method)) {
      if (IsLongrunningOperation(method)) {
        HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  future<StatusOr<$response_type$>> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) override;
)""");

        HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  StatusOr<$response_type$> $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) override;
)""");

      } else {
        if (IsResponseTypeEmpty(method)) {
          HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  Status $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) override;
)""");
        } else {
          HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  StatusOr<$response_type$> $method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options, $request_type$ const& request) override;
)""");
        }
      }
    }
  }

  for (auto const& method : async_methods()) {
    // No streaming RPCs for REST, and Longrunning is already taken care of.
    if (IsStreaming(method) || IsLongrunningOperation(method)) continue;
    if (IsResponseTypeEmpty(method)) {
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  future<Status> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) override;
)""");
    } else {
      HeaderPrintMethod(method, __FILE__, __LINE__, R"""(
  future<StatusOr<$response_type$>> Async$method_name$(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) override;
)""");
    }
  }

  if (HasLongrunningMethod()) {
    HeaderPrint(
        R"""(
  future<StatusOr<$longrunning_response_type$>> AsyncGetOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $longrunning_get_operation_request_type$ const& request) override;

  future<Status> AsyncCancelOperation(
      google::cloud::CompletionQueue& cq,
      std::unique_ptr<google::cloud::rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $longrunning_cancel_operation_request_type$ const& request) override;
)""");
  }

  // private members and close default stub class definition
  HeaderPrint(R"""(
 private:
  std::shared_ptr<rest_internal::RestClient> service_;)""");
  if (HasLongrunningMethod()) {
    HeaderPrint(R"""(
  std::shared_ptr<rest_internal::RestClient> operations_;)""");
  }
  HeaderPrint(R"""(
  Options options_;
};
)""");

  HeaderCloseNamespaces();
  // close header guard
  HeaderPrint("\n#endif  // $header_include_guard$\n");
  return {};
}

Status StubRestGenerator::GenerateCc() {
  CcPrint(CopyrightLicenseFileHeader());
  CcPrint(R"""(
// Generated by the Codegen C++ plugin.
// If you make any local changes, they will be lost.
// source: $proto_file_name$
)""");

  CcPrint("\n");
  CcLocalIncludes({vars("stub_rest_header_path"),
                   "google/cloud/common_options.h",
                   "google/cloud/internal/absl_str_cat_quiet.h",
                   "google/cloud/internal/rest_stub_helpers.h",
                   "google/cloud/status_or.h"});
  CcSystemIncludes({vars("proto_header_path"),
                    HasLongrunningMethod()
                        ? vars("longrunning_operation_include_header")
                        : "",
                    "memory", "utility"});

  auto result = CcOpenNamespaces(NamespaceType::kInternal);
  if (!result.ok()) return result;

  if (HasLongrunningMethod()) {
    CcPrint(R"""(
Default$stub_rest_class_name$::Default$stub_rest_class_name$(Options options)
    : service_(rest_internal::MakePooledRestClient(
          options.get<EndpointOption>(), options)),
      operations_(rest_internal::MakePooledRestClient()""");

    if (HasGRPCLongrunningOperation()) {
      CcPrint(R"""(
          options.get<rest_internal::LongrunningEndpointOption>(), options)),)""");
    } else {
      CcPrint(R"""(
          options.get<EndpointOption>(), options)),)""");
    }

    CcPrint(R"""(
      options_(std::move(options)) {}

Default$stub_rest_class_name$::Default$stub_rest_class_name$(
    std::shared_ptr<rest_internal::RestClient> service,
    std::shared_ptr<rest_internal::RestClient> operations,
    Options options)
    : service_(std::move(service)),
      operations_(std::move(operations)),
      options_(std::move(options)) {}
)""");
  } else {
    CcPrint(R"""(
Default$stub_rest_class_name$::Default$stub_rest_class_name$(Options options)
    : service_(rest_internal::MakePooledRestClient(
          options.get<EndpointOption>(), options)),
      options_(std::move(options)) {}

Default$stub_rest_class_name$::Default$stub_rest_class_name$(
    std::shared_ptr<rest_internal::RestClient> service,
    Options options)
    : service_(std::move(service)),
      options_(std::move(options)) {}
)""");
  }

  // default stub class member methods
  for (auto const& method : methods()) {
    if (IsStreaming(method)) continue;
    if (!HasHttpAnnotation(method)) continue;
    if (IsLongrunningOperation(method)) {
      CcPrintMethod(method, __FILE__, __LINE__, R"""(
future<StatusOr<$response_type$>>
Default$stub_rest_class_name$::Async$method_name$(
      CompletionQueue& cq,
      std::unique_ptr<rest_internal::RestContext> rest_context,
      google::cloud::internal::ImmutableOptions options,
      $request_type$ const& request) {
  promise<StatusOr<$response_type$>> p;
  future<StatusOr<$response_type$>> f = p.get_future();
  std::thread t{[](
          auto p, auto service, auto request, auto rest_context, auto options) {
)""" + QueryParameterCode(method) + R"""(
      p.set_value(rest_internal::$method_http_verb$<$response_type$>(
          *service, *rest_context, $request_resource$,
          $preserve_proto_field_names_in_json$,
          $method_rest_path_async$, std::move(params)));
    },
    std::move(p), service_, request, std::move(rest_context),
    std::move(options)};
  return f.then([t = std::move(t), cq](auto f) mutable {
    cq.RunAsync([t = std::move(t)]() mutable {
      t.join();
    });
    return f.get();
  });
}
)""");

      CcPrintMethod(method, __FILE__, __LINE__, R"""(
StatusOr<$response_type$>
Default$stub_rest_class_name$::$method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options,
      $request_type$ const& request) {
)""" + QueryParameterCode(method) + R"""(
  return rest_internal::$method_http_verb$<$response_type$>(
      *service_, rest_context, $request_resource$, $preserve_proto_field_names_in_json$,
      $method_rest_path$, std::move(params));
}
)""");

    } else {
      if (IsResponseTypeEmpty(method)) {
        CcPrintMethod(method, __FILE__, __LINE__,
                      R"""(
Status Default$stub_rest_class_name$::$method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options,
      $request_type$ const& request) {
)""" + QueryParameterCode(method) + R"""(
  return rest_internal::$method_http_verb$<google::cloud::rest_internal::EmptyResponseType>(
      *service_, rest_context, $request_resource$, $preserve_proto_field_names_in_json$,
      $method_rest_path$, std::move(params));
}
)""");
      } else {
        CcPrintMethod(method, __FILE__, __LINE__,
                      R"""(
StatusOr<$response_type$>
Default$stub_rest_class_name$::$method_name$(
      google::cloud::rest_internal::RestContext& rest_context,
      Options const& options,
      $request_type$ const& request) {
)""" + QueryParameterCode(method) +
                          R"""(
  return rest_internal::$method_http_verb$<$response_type$>(
      *service_, rest_context, $request_resource$, $preserve_proto_field_names_in_json$,
      $method_rest_path$, std::move(params));
}
)""");
      }
    }
  }

  for (auto const& method : async_methods()) {
    // No streaming RPCs for REST, and Longrunning is already taken care of.
    if (IsStreaming(method) || IsLongrunningOperation(method)) continue;
    if (!HasHttpAnnotation(method)) continue;
    if (IsResponseTypeEmpty(method)) {
      CcPrintMethod(method, __FILE__, __LINE__, R"""(
future<Status>
Default$stub_rest_class_name$::Async$method_name$(
    google::cloud::CompletionQueue& cq,
    std::unique_ptr<rest_internal::RestContext> rest_context,
    google::cloud::internal::ImmutableOptions options,
    $request_type$ const& request) {
  promise<StatusOr<google::protobuf::Empty>> p;
  future<StatusOr<google::protobuf::Empty>> f = p.get_future();
  std::thread t{[](
          auto p, auto service, auto request, auto rest_context, auto options) {
)""" + QueryParameterCode(method) + R"""(
      p.set_value(rest_internal::$method_http_verb$<google::protobuf::Empty>(
          *service, *rest_context, $request_resource$, $preserve_proto_field_names_in_json$,
          $method_rest_path_async$, std::move(params)));
    },
    std::move(p), service_, request, std::move(rest_context),
    std::move(options)};
  return f.then([t = std::move(t), cq](auto f) mutable {
    cq.RunAsync([t = std::move(t)]() mutable {
      t.join();
    });
    return f.get().status();
  });
}
)""");
    } else {
      CcPrintMethod(method, __FILE__, __LINE__, R"""(
future<StatusOr<$response_type$>>
Default$stub_rest_class_name$::Async$method_name$(
    google::cloud::CompletionQueue& cq,
    std::unique_ptr<rest_internal::RestContext> rest_context,
    google::cloud::internal::ImmutableOptions options,
    $request_type$ const& request) {
  promise<StatusOr<$response_type$>> p;
  future<StatusOr<$response_type$>> f = p.get_future();
  std::thread t{[](
          auto p, auto service, auto request, auto rest_context, auto options) {
)""" + QueryParameterCode(method) + R"""(
      p.set_value(rest_internal::$method_http_verb$<$response_type$>(
          *service, *rest_context, $request_resource$, $preserve_proto_field_names_in_json$,
          $method_rest_path_async$, std::move(params)));
    },
    std::move(p), service_, request, std::move(rest_context),
    std::move(options)};
  return f.then([t = std::move(t), cq](auto f) mutable {
    cq.RunAsync([t = std::move(t)]() mutable {
      t.join();
    });
    return f.get();
  });
}
)""");
    }
  }

  if (HasLongrunningMethod()) {
    CcPrint(
        R"""(
future<StatusOr<$longrunning_response_type$>>
Default$stub_rest_class_name$::AsyncGetOperation(
    google::cloud::CompletionQueue& cq,
    std::unique_ptr<rest_internal::RestContext> rest_context,
    google::cloud::internal::ImmutableOptions options,
    $longrunning_get_operation_request_type$ const& request) {
  promise<StatusOr<$longrunning_response_type$>> p;
  future<StatusOr<$longrunning_response_type$>> f = p.get_future();
  std::thread t{[](auto p, auto operations, auto request, auto rest_context, auto options) {
      p.set_value(rest_internal::Get<$longrunning_response_type$>(
          *operations, *rest_context, request, $preserve_proto_field_names_in_json$,
          $longrunning_get_operation_path_rest$));
    },
    std::move(p), operations_, request, std::move(rest_context),
    std::move(options)};
  return f.then([t = std::move(t), cq](auto f) mutable {
    cq.RunAsync([t = std::move(t)]() mutable {
      t.join();
    });
    return f.get();
  });
}

future<Status>
Default$stub_rest_class_name$::AsyncCancelOperation(
    google::cloud::CompletionQueue& cq,
    std::unique_ptr<rest_internal::RestContext> rest_context,
    google::cloud::internal::ImmutableOptions options,
    $longrunning_cancel_operation_request_type$ const& request) {
  promise<StatusOr<google::protobuf::Empty>> p;
  future<StatusOr<google::protobuf::Empty>> f = p.get_future();
  std::thread t{[](auto p, auto operations, auto request, auto rest_context, auto options) {
      p.set_value(rest_internal::Post<google::protobuf::Empty>(
          *operations, *rest_context, request, $preserve_proto_field_names_in_json$,
          $longrunning_cancel_operation_path_rest$));
    },
    std::move(p), operations_, request, std::move(rest_context),
    std::move(options)};
  return f.then([t = std::move(t), cq](auto f) mutable {
    cq.RunAsync([t = std::move(t)]() mutable {
      t.join();
    });
    return f.get().status();
  });
}
)""");
  }

  CcCloseNamespaces();
  return {};
}

}  // namespace generator_internal
}  // namespace cloud
}  // namespace google
