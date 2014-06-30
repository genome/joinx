#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestBedMerge(IntegrationTest, unittest.TestCase):
    def test_merge_shrinking_stop(self):
        input_file = self.inputFiles("bed-merge/shrinking-stop.bed")[0]
        output_file = self.tempFile("output.bed")
        expected_file = self.inputFiles("bed-merge/expected-shrinking-stop.bed")[0]

        params = ["bed-merge", input_file, "-o", output_file]
        rv, err = self.execute(params)
        if err:
            print "STDERR:", err

        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

    def test_merge(self):
        input_file = self.inputFiles("bed-merge/input.bed")[0]
        output_file = self.tempFile("output.bed")
        for i in [0, 1, 9, 10]:
            expected_file = self.inputFiles("bed-merge/out-d%d.bed" % i)[0]

            params = ["bed-merge", input_file, "-d", str(i), "-o", output_file]
            rv, err = self.execute(params)
            if err:
                print "STDERR:", err

            self.assertEqual(0, rv)
            self.assertEqual('', err)
            self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
