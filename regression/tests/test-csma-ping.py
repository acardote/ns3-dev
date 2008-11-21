#! /usr/bin/env python

"""Generic trace-comparison-type regression test."""

import os
import shutil
import tracediff

def run(verbose, generate):
    """Execute a test."""

    return tracediff.run_test(verbose, generate, "csma-ping")
