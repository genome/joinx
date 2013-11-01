#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestWig2Bed(IntegrationTest, unittest.TestCase):
    def test_wig2bed(self):
        input_file = self.inputFiles("wig2bed/input.wig")[0]
        output_file = self.tempFile("output.bed")
        expected_to_opts = {
            "expected.bed": [],
            "expected-c.bed": ['-c'],
            "expected-Z.bed": ['-Z'],
            "expected-Zc.bed": ['-Z', '-c'],
        }

        for expected_base, opts in expected_to_opts.iteritems():
            expected_file = self.inputFiles("wig2bed/%s" % expected_base)[0]

            params = ["wig2bed", input_file, "-o", output_file] + opts
            rv, err = self.execute(params)
            if err:
                print "STDERR:", err

            self.assertEqual(0, rv)
            self.assertEqual('', err)
            self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
