#!/usr/bin/env python3
"""Map GNU dead-section elimination to its Darwin linker equivalent."""
import os,sys
args=["-Wl,-dead_strip" if a=="-Wl,--gc-sections" else a for a in sys.argv[1:]]
os.execv("/usr/bin/clang", ["clang", *args])
