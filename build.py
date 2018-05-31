#!/usr/bin/env python
# Get Depot Tools and make sure it's in your path.
# http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up
# Use .gclient to modify the deps.
import os
import sys
import subprocess


def main():
    # Run sync if any of the dep dirs don't exist.
    # Or the user supplied the --sync flag.
    if "--sync" in sys.argv or dirsMissing():
        run("gclient sync --no-history")

    # Run gn gen out/Default if out doesn't exist.
    if not os.path.exists("out/Default"):
        run("gn gen out/Default")

    # Always run ninja.
    run("ninja -C out/Default")


def run(cmd):
    print cmd
    args = cmd.split(" ")
    env = os.environ.copy()
    subprocess.check_call(args, env=env)


def dirsMissing():
    dirsToLoad = [
        "v8",
        "third_party/protobuf",
        "tools/protoc_wrapper",
        "third_party/zlib",
    ]
    for d in dirsToLoad:
        if not os.path.exists(d):
            return True
    return False


if '__main__' == __name__:
    main()
