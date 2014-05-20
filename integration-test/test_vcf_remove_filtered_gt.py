#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfRemoveFilteredGt(IntegrationTest, unittest.TestCase):

    def test_whitelist(self):
        input_file = self.inputFiles("vcf-remove-filtered-gt/input.vcf")[0]
        expected_file = self.inputFiles("vcf-remove-filtered-gt/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = ["vcf-remove-filtered-gt", "-o", output_file,
                "-i", input_file, "-w", "OK"]

        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

        self.assertFilesEqual(expected_file, output_file, filter_regex="##annotation")

if __name__ == "__main__":
    main()

