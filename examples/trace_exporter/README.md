# Cloud Trace Exporter example

An example project that runs an application with OpenTelemetry tracing enabled
in the `google-cloud-cpp` library. The application exports the collected traces
to Cloud Trace.

## Run

```sh
bazel run //:quickstart -- my-project
```

Note that the `.bazelrc` file in this project is supplying the
`--@com_github_googleapis_google_cloud_cpp//:experimental-open_telemetry` flag.

## Why `talent`?

We pick `talent` because it is a simple CRUD library that will not immediately
incur any billing charges.
