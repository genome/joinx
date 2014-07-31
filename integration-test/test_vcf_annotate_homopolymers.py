#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfAnnotateHomopolymer(IntegrationTest, unittest.TestCase):

    def test_annotate_homopolymer(self):
        input_file = self.inputFiles("vcf-annotate-homopolymers/input.vcf")[0]
        bed_file = self.inputFiles("vcf-annotate-homopolymers/homop.bed.slop")[0]
        expected_file = self.inputFiles("vcf-annotate-homopolymers/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = ["vcf-annotate-homopolymers", "-o", output_file,
                "-v", input_file, "-b", bed_file, "-m 2",
                "-n", "TEST"]
        print "Executing", " ".join(params)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

        self.assertFilesEqual(expected_file, output_file, filter_regex="##annotation")

if __name__ == "__main__":
    main()

