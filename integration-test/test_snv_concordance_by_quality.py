#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestSnvConcordanceByQuality(IntegrationTest, unittest.TestCase):

    def test_snv_concordance_by_quality(self):
        input_files = self.inputFiles("snva.bed", "snvb.bed")
        expected_file = self.inputFiles("expected-concordance-by-qual.txt")[0]
        output_file = self.tempFile("output.bed")

        params = [ "snv-concordance-by-quality", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()

