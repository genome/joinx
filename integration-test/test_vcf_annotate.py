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

        expected_lines = "\n".join([x for x in open(expected_file).readlines()
                if not x.startswith("##annotation")])

        output_lines = "\n".join([x for x in open(output_file).readlines()
                if not x.startswith("##annotation")])

        self.assertMultiLineEqual(expected_lines, output_lines)

if __name__ == "__main__":
    main()

