#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestSnvConcordance(IntegrationTest, unittest.TestCase):

    def test_snv_concordance_ab(self):
        input_files = self.inputFiles("snva.bed", "snvb.bed")
        expected_file = self.inputFiles("expected-concordance-ab.txt")[0]
        output_file = self.tempFile("output.bed")

        params = [ "snv-concordance", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

    def test_snv_concordance_ba(self):
        input_files = self.inputFiles("snvb.bed", "snva.bed")
        expected_file = self.inputFiles("expected-concordance-ba.txt")[0]
        output_file = self.tempFile("output.bed")

        params = [ "snv-concordance", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()

