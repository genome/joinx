#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfMergeConsensus(IntegrationTest, unittest.TestCase):

    def test_sample1_all(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample1_all = self.inputFiles("vcf-merge/consensus/merge-S1-all.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S1-S2_S1-all.vcf")[0]
        output_file = self.tempFile("output.vcf")

        equivalent_inputs = [
                [both_samples, sample1_all],
                [sample1_all, both_samples]
                ]

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        for inputs in equivalent_inputs:
            params = base_params + inputs
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_sample1_none(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample1_none = self.inputFiles("vcf-merge/consensus/merge-S1-none.vcf")[0]
        sample1_empty = self.inputFiles("vcf-merge/consensus/merge-S1-empty.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S1-S2_S1-none.vcf")[0]
        output_file = self.tempFile("output.vcf")

        equivalent_inputs = [
                [both_samples, sample1_none],
                [sample1_none, both_samples],
                [both_samples, sample1_empty],
                [sample1_empty, both_samples]
                ]

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        for inputs in equivalent_inputs:
            params = base_params + inputs
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_sample2_all_first(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample2_all = self.inputFiles("vcf-merge/consensus/merge-S2-all.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S2-all_S1-S2.vcf")[0]
        output_file = self.tempFile("output.vcf")

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        params = base_params + [sample2_all, both_samples]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_sample2_all_second(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample2_all = self.inputFiles("vcf-merge/consensus/merge-S2-all.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S1-S2_S2-all.vcf")[0]
        output_file = self.tempFile("output.vcf")

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        params = base_params + [both_samples, sample2_all]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")


    def test_sample2_none_first(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample2_none = self.inputFiles("vcf-merge/consensus/merge-S2-none.vcf")[0]
        sample2_empty = self.inputFiles("vcf-merge/consensus/merge-S2-empty.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S2-none_S1-S2.vcf")[0]
        output_file = self.tempFile("output.vcf")

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]

        equivalent_inputs = [
                [sample2_none, both_samples],
                [sample2_empty, both_samples]
                ]

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        for inputs in equivalent_inputs:
            params = base_params + inputs
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_sample2_none_second(self):
        both_samples = self.inputFiles("vcf-merge/consensus/merge-S1-S2.vcf")[0]
        sample2_none = self.inputFiles("vcf-merge/consensus/merge-S2-none.vcf")[0]
        sample2_empty = self.inputFiles("vcf-merge/consensus/merge-S2-empty.vcf")[0]

        expected_file = self.inputFiles("vcf-merge/consensus/expected-S1-S2_S2-none.vcf")[0]
        output_file = self.tempFile("output.vcf")

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]

        equivalent_inputs = [
                [both_samples, sample2_none],
                [both_samples, sample2_empty]
                ]

        base_params = [ "vcf-merge", "-s", "-R 1.0,FILT,FILT", "-o", output_file ]
        for inputs in equivalent_inputs:
            params = base_params + inputs
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")


if __name__ == "__main__":
    main()

