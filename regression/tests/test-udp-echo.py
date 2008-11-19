#! /usr/bin/env python

"""Generic trace-comparison-type regression test."""

import os
import shutil
import tracediff

def run(verbose, generate):
    """Execute a test."""
    #print tracediff.env
    return tracediff.run_test(verbose, generate, "udp-echo")
