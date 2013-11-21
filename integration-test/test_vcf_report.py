#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfReport(IntegrationTest, unittest.TestCase):
    def test_report(self):
        input_file = self.inputFiles("vcf-report/input.vcf")[0]
        expected_site_file = self.inputFiles("vcf-report/expected-site.txt")[0]
        expected_sample_file = self.inputFiles("vcf-report/expected-sample.txt")[0]

        output_site_file = self.tempFile("output-site.vcf")
        output_sample_file = self.tempFile("output-sample.vcf")

        params = ["vcf-report", "-i", input_file, "-s", output_site_file,
                "-S", output_sample_file]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertTrue('Skipping entry 8\t10' in err)
        self.assertFilesEqual(expected_site_file, output_site_file)
        self.assertFilesEqual(expected_sample_file, output_sample_file)

if __name__ == "__main__":
    main()
