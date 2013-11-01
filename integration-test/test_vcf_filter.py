#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfFilter(IntegrationTest, unittest.TestCase):
    def test_filter(self):
        input_file = self.inputFiles("vcf-filter/input.vcf")[0]
        output_file = self.tempFile("output.vcf")
        for i in [2, 5, 6, 10]:
            expected_file = self.inputFiles("vcf-filter/expected-d%d.vcf" % i)[0]

            params = ["vcf-filter", input_file, "-d", str(i), "-o", output_file]
            rv, err = self.execute(params)
            if err:
                print "STDERR:", err

            self.assertEqual(0, rv)
            self.assertEqual('', err)
            self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
