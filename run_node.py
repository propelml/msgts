#!/usr/bin/env python

"""
gn can only run python scripts.
Also Node programs except to be run with cwd = root_dir so it can look in
node_modules.
"""

import subprocess
import sys
import os

# cwd to the directory of this script.
root = os.path.dirname(os.path.abspath(__file__))
os.chdir(root)

args = ["node"] + sys.argv[1:]
sys.exit(subprocess.call(args))
