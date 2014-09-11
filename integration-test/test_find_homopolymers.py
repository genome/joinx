#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfAnnotateHomopolymer(IntegrationTest, unittest.TestCase):

    def test_find_homopolymers(self):
        input_file = self.inputFiles("find-homopolymers/test.fa")[0]
        expected_file = self.inputFiles("find-homopolymers/expected.bed")[0]
        output_file = self.tempFile("output.bed")

        params = ["find-homopolymers", "-o", output_file,
                "-m 2", "-f", input_file]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)

        self.assertFilesEqual(expected_file, output_file, filter_regex="##annotation")

if __name__ == "__main__":
    main()

