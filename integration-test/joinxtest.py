#!/usr/bin/env python

from glob import glob
from subprocess import Popen, PIPE
import difflib
import os
import shlex
import shutil
import sys
import tempfile
import unittest


class JoinxTest():
    exe_path = None
    def setUp(self):
        self.test_dir = os.path.dirname(sys.argv[0])
        self.data_dir = os.path.join(self.test_dir, "data")
        self.tmp_dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmp_dir)

    def inputFiles(self, *names):
        rv = []
        for n in names:
            rv.extend(glob(os.path.join(self.data_dir, n)))
        if len(rv) == 0:
            raise IOError("No file matching %s not found in %s" %(
                ", ".join(names),
                self.data_dir)
            )
        return rv

    def joinx(self, args):
        cmdline = "%s %s" %(self.exe_path, " ".join(args))
        # Popen wants split args
        p = Popen(shlex.split(cmdline), stderr=PIPE, close_fds=True)
        out, err = p.communicate(None)
        return p.returncode, err

    def tempFile(self, name):
        return os.path.join(self.tmp_dir, name)

    def assertFilesEqual(self, first, second, msg=None):
       first_data = open(first).read()
       second_data = open(second).read()
       self.assertMultiLineEqual(first_data, second_data)

        
    def assertMultiLineEqual(self, first, second, msg=None):
        """Assert that two multi-line strings are equal.
        If they aren't, show a nice diff.
        """
        self.assertTrue(isinstance(first, str),
                'First argument is not a string')
        self.assertTrue(isinstance(second, str),
                'Second argument is not a string')

        if first != second:
            message = ''.join(difflib.ndiff(first.splitlines(True),
                                                second.splitlines(True)))
            if msg:
                message += " : " + msg
            self.fail("Multi-line strings are unequal:\n" + message)

def main():
    if len(sys.argv) < 2:
        print "Error: required argument (path to joinx executable) missing"
        sys.exit(1)
    JoinxTest.exe_path = sys.argv.pop()
    unittest.main()
