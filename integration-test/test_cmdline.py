#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest


class TestCmdline(IntegrationTest, unittest.TestCase):

    def get_commands(self):
        rv, err = self.execute([])
        cmds = [x[1:].split(" - ")[0] for x in err.split("\n") if x and x[0] == '\t']
        return cmds

    def test_help(self):
        params = [ "-h" ]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

    def test_version(self):
        params = [ "-v" ]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

    def test_command_help(self):
        cmds = self.get_commands()

        for cmd in cmds:
            params = [cmd, '-h']
            print params
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertEqual('', err)

if __name__ == "__main__":
    main()

