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
]

spec = "solutions = %s" % GCLIENT_SOLUTION
env = os.environ.copy()
# gclient needs to have depot_tools in the PATH.
subprocess.check_call(["gclient", "config", "--spec", spec], env=env)
