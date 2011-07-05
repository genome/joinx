#!/usr/bin/env python

from joinxtest import JoinxTest, main
import unittest

class TestCreateContigs(JoinxTest, unittest.TestCase):

    def test_create_contigs(self):
        input_files = self.inputFiles("small.fa", "variants-contig.bed")
        output_file = self.tempFile("output.bed")
        expected_file = self.inputFiles("expected-contigs.fa")[0]
        params = ["create-contigs", "--flank=10", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.joinx(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
