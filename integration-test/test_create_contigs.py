#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestCreateContigs(IntegrationTest, unittest.TestCase):

    def test_create_contigs(self):
        input_files = self.inputFiles("small.fa", "variants-contig.vcf")
        output_fasta = self.tempFile("output.fa")
        output_remap = self.tempFile("output.fa.remap")
        expected_fasta = self.inputFiles("expected-contigs.fa")[0]
        expected_remap = self.inputFiles("expected-contigs.fa.remap")[0]
        params = ["create-contigs", "--flank=10", "-o", output_fasta,
            "-R", output_remap]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_fasta, output_fasta)
        self.assertFilesEqual(expected_remap, output_remap)

    def test_fasta_not_found(self):
        input_files = self.inputFiles("variants-contig.vcf")
        output_fasta = self.tempFile("output.fa")
        output_remap = self.tempFile("output.fa.remap")
        params = ["create-contigs", "--flank=10", "-o", output_fasta,
            "-R", output_remap]
        params.append("boof.fa")
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(1, rv)
        self.assertTrue("Failed to memory map fasta 'boof.fa'" in err, "value was %s" %err)

    def test_vcf_not_found(self):
        input_files = self.inputFiles("small.fa")
        output_fasta = self.tempFile("output.fa")
        output_remap = self.tempFile("output.fa.remap")

        params = ["create-contigs", "--flank=10", "-o", output_fasta,
            "-R", output_remap ]
        params.extend(input_files)
        params.append("boof.vcf")
        rv, err = self.execute(params)
        self.assertEqual(1, rv)
        self.assertTrue("Failed to open file boof.vcf" in err, "value was %s" %err)



if __name__ == "__main__":
    main()
