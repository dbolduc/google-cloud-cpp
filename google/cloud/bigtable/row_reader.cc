// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigtable/row_reader.h"
#include "google/cloud/bigtable/table.h"
#include "google/cloud/grpc_error_delegate.h"
#include "google/cloud/internal/throw_delegate.h"
#include "google/cloud/log.h"
#include "absl/memory/memory.h"
#include <iterator>
#include <thread>

namespace google {
namespace cloud {
namespace bigtable {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
// RowReader::iterator must satisfy the requirements of an InputIterator.
static_assert(
    std::is_same<std::iterator_traits<RowReader::iterator>::iterator_category,
                 std::input_iterator_tag>::value,
    "RowReader::iterator should be an InputIterator");
static_assert(
    std::is_same<std::iterator_traits<RowReader::iterator>::value_type,
                 StatusOr<Row>>::value,
    "RowReader::iterator should be an InputIterator of Row");
static_assert(std::is_same<std::iterator_traits<RowReader::iterator>::pointer,
                           StatusOr<Row>*>::value,
              "RowReader::iterator should be an InputIterator of Row");
static_assert(std::is_same<std::iterator_traits<RowReader::iterator>::reference,
                           StatusOr<Row>&>::value,
              "RowReader::iterator should be an InputIterator of Row");
static_assert(std::is_copy_constructible<RowReader::iterator>::value,
              "RowReader::iterator must be CopyConstructible");
static_assert(std::is_move_constructible<RowReader::iterator>::value,
              "RowReader::iterator must be MoveConstructible");
static_assert(std::is_copy_assignable<RowReader::iterator>::value,
              "RowReader::iterator must be CopyAssignable");
static_assert(std::is_move_assignable<RowReader::iterator>::value,
              "RowReader::iterator must be MoveAssignable");
static_assert(std::is_destructible<RowReader::iterator>::value,
              "RowReader::iterator must be Destructible");
static_assert(
    std::is_convertible<decltype(*std::declval<RowReader::iterator>()),
                        RowReader::iterator::value_type>::value,
    "*it when it is of RowReader::iterator type must be convertible to "
    "RowReader::iterator::value_type>");
static_assert(std::is_same<decltype(++std::declval<RowReader::iterator>()),
                           RowReader::iterator&>::value,
              "++it when it is of RowReader::iterator type must be a "
              "RowReader::iterator &>");

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable

namespace bigtable_mocks {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

bigtable::RowReader MakeMockRowReader(
    std::vector<StatusOr<bigtable::Row>> rows) {
  class MockRowReader : public bigtable_internal::RowReaderImpl {
    public:
     explicit MockRowReader(std::vector<StatusOr<bigtable::Row>> rows)
         : rows_(std::move(rows)),
           iter_(std::make_move_iterator(rows_.begin())),
           end_(std::make_move_iterator(rows_.end())) {}

     ~MockRowReader() override = default;

     void Cancel() override { iter_ = end_; }

     StatusOr<OptionalRow> Advance() override {
       if (iter_ == end_) return make_status_or<OptionalRow>(absl::nullopt);
       auto sor = *iter_++;
       if (!sor) return std::move(sor).status();
       auto optional_row = absl::make_optional(*std::move(sor));
       return make_status_or<OptionalRow>(optional_row);
     }

    private:
      std::vector<StatusOr<bigtable::Row>> rows_;
      using move_iter =
          std::move_iterator<std::vector<StatusOr<bigtable::Row>>::iterator>;
      move_iter iter_;
      move_iter end_;
  };

  auto mock = std::make_shared<MockRowReader>(std::move(rows));
  return bigtable::RowReader(std::move(mock));
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END

}
}  // namespace cloud
}  // namespace google
