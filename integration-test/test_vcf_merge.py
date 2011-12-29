#!/usr/bin/env python

from joinxtest import JoinxTest, main
import unittest

class TestVcfMerge(JoinxTest, unittest.TestCase):

    def test_vcf_merge(self):
        input_files = self.inputFiles("merge-[0-9].vcf")
        expected_file = self.inputFiles("merged.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.joinx(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()

