#!/bin/sh
clang-format -i -style Google *.cc *.h
gn format BUILD.gn
gn format .gn
yapf -i build.py
