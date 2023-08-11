#!/bin/bash

# NOTE: I am just running these commands manually.

# Pretend to install the package config files for opentelemetry-cpp
su - # Need root perms to write to /usr/local/
cp /workspace/otel_pc/* /usr/local/lib64/pkgconfig/
ls /usr/local/lib64/pkgconfig/opentelemetry_* | xargs -I {} chmod 644 {}
exit

# For debugging with `pkg-config`, outside of our build scripts.
PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/h/google-cloud-cpp-installed/lib64/pkgconfig"
