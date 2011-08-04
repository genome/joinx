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
        self.assertFilesEqual(expected_file, output_file)

    def test_fasta_not_found(self):
        input_files = self.inputFiles("variants-contig.bed")
        output_file = self.tempFile("output.bed")
        params = ["create-contigs", "--flank=10", "-o", output_file ]
        params.append("boof.fa")
        params.extend(input_files)
        rv, err = self.joinx(params)
        self.assertEqual(1, rv)
        self.assertTrue("Failed to load fasta file: boof.fa" in err, "value was %s" %err)

    def test_bed_not_found(self):
        input_files = self.inputFiles("small.fa")
        output_file = self.tempFile("output.bed")
        params = ["create-contigs", "--flank=10", "-o", output_file ]
        params.extend(input_files)
        params.append("boof.bed")
        rv, err = self.joinx(params)
        self.assertEqual(1, rv)
        self.assertTrue("Failed to open file boof.bed" in err, "value was %s" %err)



if __name__ == "__main__":
    main()
