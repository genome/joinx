#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfNormalizeIndels(IntegrationTest, unittest.TestCase):
    def test_normalize(self):
        input_file = self.inputFiles("vcf-normalize-indels/input.vcf")[0]
        fasta_file = self.inputFiles("vcf-normalize-indels/ref.fa")[0]
        expected_file = self.inputFiles("vcf-normalize-indels/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = ["vcf-normalize-indels", "-f", fasta_file, "-i", input_file,
                "-o", output_file]
        rv, err = self.execute(params)
        if err:
            print "STDERR:", err

        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
