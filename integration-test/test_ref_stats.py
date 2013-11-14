#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestRefStats(IntegrationTest, unittest.TestCase):

    def test_basic(self):
        fasta_file = self.inputFiles("refstats.fa")[0]
        bed_file = self.inputFiles("refstats.bed")[0]
        expected_file = self.inputFiles("expected-refstats-basic.txt")[0]
        output_file = self.tempFile("output.bed")

        params = ["ref-stats", "-b", bed_file, "-f", fasta_file,
                "-o", output_file]
        rv, err = self.execute(params)
        if err:
            print "STDERR:", err

        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

    def test_with_bases(self):
        fasta_file = self.inputFiles("refstats.fa")[0]
        bed_file = self.inputFiles("refstats.bed")[0]
        expected_file = self.inputFiles("expected-refstats-with-bases.txt")[0]
        output_file = self.tempFile("output.bed")

        params = ["ref-stats", "-b", bed_file, "-f", fasta_file,
                "-o", output_file, "-r"]
        rv, err = self.execute(params)
        if err:
            print "STDERR:", err

        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

    def test_duplicate_tokens(self):
        fasta_file = self.inputFiles("refstats.fa")[0]
        bed_file = self.inputFiles("refstats.bed")[0]

        params = ["ref-stats", "-b", bed_file, "-f", fasta_file,
                "-t", "a", "-t", "a/t"]

        rv, err = self.execute(params)
        self.assertNotEqual(0, rv)
        self.assertTrue("Duplicate element" in err)

    def test_invalid_tokens(self):
        fasta_file = self.inputFiles("refstats.fa")[0]
        bed_file = self.inputFiles("refstats.bed")[0]

        params = ["ref-stats", "-b", bed_file, "-f", fasta_file,
                "-t", "a.c"]

        rv, err = self.execute(params)
        self.assertNotEqual(0, rv)
        self.assertTrue("Invalid character" in err)


if __name__ == "__main__":
    main()
