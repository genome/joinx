#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfFilter(IntegrationTest, unittest.TestCase):
    def test_merge(self):
        input_file = self.inputFiles("vcf-site-filter/input.vcf")[0]
        output_file = self.tempFile("output.vcf")
        for i in ['0.3', '0.6', '0.9']:
            expected_file = self.inputFiles("vcf-site-filter/expected-f%s.vcf" % i)[0]

            params = ["vcf-site-filter", input_file, "-f", str(i), "-o", output_file]
            rv, err = self.execute(params)
            if err:
                print "STDERR:", err

            self.assertEqual(0, rv)
            self.assertEqual('', err)
            self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
