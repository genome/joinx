#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfMerge(IntegrationTest, unittest.TestCase):

    def test_vcf_merge(self):
        input_files = sorted(self.inputFiles("merge-[0-9].vcf"))
        expected_file = self.inputFiles("merged.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

if __name__ == "__main__":
    main()

