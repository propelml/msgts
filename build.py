#!/usr/bin/env python
# Get Depot Tools and make sure it's in your path.
# http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up
# Use .gclient to modify the deps.
import os
import sys
import subprocess
import argparse

TARGET = "xhello"

parser = argparse.ArgumentParser(description="build.py")
parser.add_argument('--debug', dest='debug', action='store_true')
parser.add_argument('--use_ccache', dest='use_ccache', action='store_true')
parser.add_argument('--sync', dest='sync', action='store_true')
parser.set_defaults(debug=False, use_ccache=False, sync=False)
args = parser.parse_args()


def main():
    buildDir = "out/Debug" if args.debug else "out/Default"
    # Run sync if any of the dep dirs don't exist.
    # Or the user supplied the --sync flag.
    if args.sync or dirsMissing():
        run("gclient sync --no-history")

    # Run gn gen out/Default if out doesn't exist.
    if not os.path.exists(buildDir):
        gn_gen = "gn gen " + buildDir
        gn_args = []
        if args.debug:
            gn_args.append("is_debug=true")
        if args.use_ccache:
            gn_args.append("cc_wraper=\"ccache\"")
        gn_gen += " --args='%s' " % " ".join(gn_args)
        run(gn_gen)

    # Always run ninja.
    run("ninja -C " + buildDir + " " + TARGET)


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
