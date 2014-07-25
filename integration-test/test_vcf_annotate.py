#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfAnnotate(IntegrationTest, unittest.TestCase):

    def test_annotate_per_alt(self):
        input_file = self.inputFiles("vcf-annotate/input.vcf")[0]
        annot_file = self.inputFiles("vcf-annotate/annotation.vcf")[0]
        expected_file = self.inputFiles("vcf-annotate/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = ["vcf-annotate", "-o", output_file,
                "-i", input_file, "-a", annot_file,
                "-I", "TEST=TEST,per-alt"]

        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

        self.assertFilesEqual(expected_file, output_file, filter_regex="##annotation")

    def test_annotate_multi_alts(self):
        input_file = self.inputFiles("vcf-annotate/multi/input.vcf")[0]
        annot_file = self.inputFiles("vcf-annotate/multi/annotation.vcf")[0]
        expected_file = self.inputFiles("vcf-annotate/multi/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = ["vcf-annotate", "-o", output_file,
                "-i", input_file, "-a", annot_file]

        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

        self.assertFilesEqual(expected_file, output_file, filter_regex="##annotation")

if __name__ == "__main__":
    main()

