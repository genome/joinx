#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfCompareGt(IntegrationTest, unittest.TestCase):

    def test_filter_both_both(self):
        input_files = self.inputFiles("vcf-compare-gt/[ab].vcf")
        expected_file = self.inputFiles("vcf-compare-gt/expected-b-b.txt")[0]
        output_file = self.tempFile("output.txt")
        params = ["vcf-compare-gt", "-F both -F both -n a -n b",
                "-o", output_file] + input_files
        print "Executing", " ".join(params)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file)

    def test_filter_f_u(self):
        input_files = self.inputFiles("vcf-compare-gt/[ab].vcf")
        expected_file = self.inputFiles("vcf-compare-gt/expected-f-u.txt")[0]
        output_file = self.tempFile("output.txt")
        params = ["vcf-compare-gt", "-F filtered -F unfiltered -n a -n b",
                "-o", output_file] + input_files
        print "Executing", " ".join(params)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()

