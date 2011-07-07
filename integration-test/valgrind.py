#!/usr/bin/env python

from subprocess import Popen, PIPE
import os
import re

class Valgrind:
    valgrind_path = None

    def __init__(self, command, vglog_file):
        self.command = command
        self.vglog_file = vglog_file
        if self.valgrind_path == None:
            for d in os.environ["PATH"].split(os.pathsep):
                path = os.path.join(d, "valgrind")
                if os.path.exists(path) and os.access(path, os.X_OK):
                    self.valgrind_path = path
                    break

    def run(self):
        cmd = self.command

        if self.valgrind_path != None:
            cmd[:0] = [
                "valgrind",
                "--leak-check=full",
                "--log-file=%s" %self.vglog_file,
            ]

        p = Popen(cmd, stderr=PIPE, close_fds=True)
        out, err = p.communicate(None)
        if not self.leak_free():
            raise RuntimeError(
                "Possible memory leaks detected in command %s" %(" ".join(cmd))
            )
        return p.returncode, err

    def leak_free(self):
        if self.valgrind_path == None:
            return True

        log_contents = open(self.vglog_file).read()
        m = re.search("no leaks are possible", log_contents)
        return m != None 
