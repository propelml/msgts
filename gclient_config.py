#!/usr/bin/env python
# 1. Get depot tools.
# 2. ./gclient_config.py
# 3. gclient sync --no-history
import os
import subprocess

GCLIENT_SOLUTION = [
    {
        "name": "v8",
        "url": "https://chromium.googlesource.com/v8/v8.git",
        "deps_file": "DEPS",
        "custom_deps": {
            # These deps are unnecessary for building.
            "v8/test/benchmarks/data": None,
            "v8/testing/gmock": None,
            "v8/test/mozilla/data": None,
            "v8/test/test262/data": None,
            "v8/test/test262/harness": None,
            "v8/test/wasm-js": None,
            "v8/third_party/android_tools": None,
            "v8/third_party/catapult": None,
            "v8/third_party/colorama/src": None,
            "v8/third_party/icu": None,
            "v8/third_party/instrumented_libraries": None,
            "v8/tools/gyp": None,
            "v8/tools/luci-go": None,
            "v8/tools/swarming_client": None,
        },
        "custom_vars": {
            "build_for_node": True,
        },
    },
    {
        "name": "third_party/protobuf",
        "url": "https://github.com/ry/protobuf_chromium.git",
        "deps_file": "DEPS",
    },
    {
        "name": "tools/protoc_wrapper",
        "url": "https://chromium.googlesource.com/chromium/src/tools/protoc_wrapper",
    },
    {
        "name": "third_party/zlib",
        "url": "https://chromium.googlesource.com/chromium/src/third_party/zlib",
    },
]

spec = "solutions = %s" % GCLIENT_SOLUTION
env = os.environ.copy()
# gclient needs to have depot_tools in the PATH.
subprocess.check_call(["gclient", "config", "--spec", spec], env=env)

# Run sync if any of the dep dirs don't exist.

# Run gn gen out/Default if out doesn't exist.

# Run ninja -C out/Default 
