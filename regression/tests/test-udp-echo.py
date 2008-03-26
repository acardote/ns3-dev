#! /usr/bin/env python

"""Regression test udp-echo."""

import os

def rmdir(dir):
    if os.path.exists(dir):
        files = os.listdir(dir)
        for file in files:
            if file == '.' or file == '..':
                continue
            path = dir + os.sep + file
            os.remove(path)
        os.rmdir(dir)

def run(verbose, generate):
    """Execute a test."""

    os.system("./waf --cwd regression/traces --run udp-echo >& /dev/null")

    if generate:
        rmdir("knowns")
        os.rename("traces", "knowns")
        os.system("tar -cjf tests/test-udp-echo.bz2 knowns/")
        rmdir("knowns")
        return 0
    else:
        rmdir("knowns")
        os.system("tar -xjf tests/test-udp-echo.bz2 knowns/")
        return os.system("diff -q knowns traces >& /dev/null")
