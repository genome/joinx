#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestVcfMerge(IntegrationTest, unittest.TestCase):

    def test_vcf_merge_sort_order(self):
        input_files = sorted(self.inputFiles("vcf-merge/sorting/merge-[0-9].vcf"))
        expected_file = self.inputFiles("vcf-merge/sorting/expected.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge", "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")


    def test_vcf_merge(self):
        merge_strategy_file = self.tempFile("strategy.ms")
        open(merge_strategy_file, "w").write(
            "info.CALLER=uniq-concat\n" +
            "info.NOTHOME=uniq-concat\n"
        )

        input_files = sorted(self.inputFiles("vcf-merge/merge-[0-9].vcf"))
        expected_file = self.inputFiles("vcf-merge/merged.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge", "-M", merge_strategy_file, "-o", output_file ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        # no assertIn in python 2.6 :(
        self.assertTrue('Warning' in err)
        self.assertTrue('NOTHOME' in err)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_vcf_merge_samples_consensus_60(self):
        merge_strategy_file = self.tempFile("strategy.ms")
        open(merge_strategy_file, "w").write("info.CALLER=uniq-concat\n")

        input_files = sorted(self.inputFiles("vcf-merge/merge-samples-[12].vcf"))
        expected_file = self.inputFiles("vcf-merge/expected-merge-samples-c60.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge",
            "-s",
            "-R '0.6,CNS,Consensus filter'",
            "-M", merge_strategy_file,
            "-o", output_file
        ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_vcf_merge_samples_consensus_50(self):
        merge_strategy_file = self.tempFile("strategy.ms")
        open(merge_strategy_file, "w").write("info.CALLER=uniq-concat\n")

        input_files = sorted(self.inputFiles("vcf-merge/merge-samples-[12].vcf"))
        expected_file = self.inputFiles("vcf-merge/expected-merge-samples-c50.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge",
            "-s",
            "-R '0.5,CNS,Consensus filter'",
            "-M", merge_strategy_file,
            "-o", output_file
        ]
        params.extend(input_files)
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_vcf_merge_samples_consensus_50_dup_inputs(self):
        merge_strategy_file = self.tempFile("strategy.ms")
        open(merge_strategy_file, "w").write("info.CALLER=uniq-concat\n")

        input_files = sorted(self.inputFiles("vcf-merge/merge-samples-[12].vcf"))
        self.assertEqual(2, len(input_files))
        expected_file = self.inputFiles("vcf-merge/expected-merge-samples-c50-D.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge",
            "-s",
            "-D", "%s=-A" %input_files[0],
            "-D", "%s=-B" %input_files[1],
            "-R '0.5,CNS,Consensus filter'",
            "-M", merge_strategy_file,
            "-o", output_file
        ]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")

    def test_vcf_merge_per_alt_list(self):
        merge_strategy_file = self.tempFile("strategy.ms")
        open(merge_strategy_file, "w").write("info.CALLER=per-alt-delimited-list")

        input_files = sorted(self.inputFiles("vcf-merge/merge-per-alt-list-[12].vcf"))
        self.assertEqual(2, len(input_files))
        expected_file = self.inputFiles("vcf-merge/expected-merge-per-alt-list.vcf")[0]
        output_file = self.tempFile("output.vcf")

        params = [ "vcf-merge",
            "-M", merge_strategy_file,
            "-o", output_file
        ] + input_files
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertFilesEqual(expected_file, output_file, filter_regex="##fileDate=")





if __name__ == "__main__":
    main()

