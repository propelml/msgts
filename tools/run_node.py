#!/usr/bin/env python
"""
gn can only run python scripts.
Also Node programs except to be run with cwd = root_dir so it can look in
node_modules.
"""

import subprocess
import sys
import os

root_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
os.chdir(root_path)

args = ["node"] + sys.argv[1:]
sys.exit(subprocess.call(args))
