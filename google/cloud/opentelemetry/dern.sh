# Run command
rb env GOOGLE_CLOUD_CPP_ENABLE_TRACING=rpc GOOGLE_CLOUD_CPP_TRACING_OPTIONS=single_line_mode=f GOOGLE_CLOUD_CPP_ENABLE_CLOG=1 bazel run quickstart:metrics -- histogram

# GCM Metrics Explorer :D
# https://screenshot.googleplex.com/9Rd8RNxSjxXnwWr

