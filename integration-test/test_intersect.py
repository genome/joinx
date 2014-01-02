#!/usr/bin/env python

import os

from integrationtest import IntegrationTest, main
import unittest
import sys

class TestIntersect(IntegrationTest, unittest.TestCase):

    def test_intersect(self):
        data = {
            "--exact-allele --output-both": "expected-exact-allele-both.bed",
            "--exact-allele": "expected-exact-allele.bed",
            "--exact-pos --output-both": "expected-exact-pos-both.bed",
            "--exact-pos": "expected-exact-pos.bed",
            "--output-both": "expected-noargs-both.bed",
            "": "expected-noargs.bed",
        }

        for args, expected in data.items():
            output_file = self.tempFile("output.bed")

            params = [ "intersect", args, "-o", output_file ]
            params.extend(self.inputFiles("a.bed", "b.bed"))

            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertEqual('', err)
            expected_file = self.inputFiles(expected)[0]
            self.assertFilesEqual(expected_file, output_file)

    def test_adjacent_insertions(self):
        output_file = self.tempFile("output.bed")
        params = [
            "intersect", "--full",
            "--adjacent-insertions",
            "-o", output_file
        ]
        params.extend(self.inputFiles(
            "adjacent-insertions-a.bed",
            "adjacent-insertions-b.bed"
        ))
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = self.inputFiles("expected-adjacent-insertions.bed")[0]
        self.assertFilesEqual(expected_file, output_file)

    def test_no_adjacent_insertions(self):
        output_file = self.tempFile("output.bed")
        params = [
            "intersect", "--full",
            "-o", output_file
        ]
        params.extend(self.inputFiles(
            "adjacent-insertions-a.bed",
            "adjacent-insertions-b.bed"
        ))
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = self.inputFiles("expected-no-adjacent-insertions.bed")[0]
        self.assertFilesEqual(expected_file, output_file)

    def test_exact_pos_regions(self):
        output_file = self.tempFile("output.bed")
        params = [
            "intersect",
            "--exact-pos",
            "-o", output_file
        ]
        a, b = self.inputFiles("a.bed", "a-regions.bed")
        params.extend([a, b])
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = a
        self.assertFilesEqual(expected_file, output_file)

    def test_partial_match(self):
        output_file = self.tempFile("output.bed")
        params = [
            "intersect",
            "--exact-allele --iub-match --output-both",
            "-o", output_file
        ]
        params.extend(self.inputFiles("iub-a.bed", "iub-b.bed"))
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = self.inputFiles("expected-iub-both.bed")[0]
        self.assertFilesEqual(expected_file, output_file)

    def test_position_only(self):
        output_file = self.tempFile("output.bed")
        params = [ "intersect", "-o", output_file ]
        params.extend(self.inputFiles("a.bed", "posonly.bed"))
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = self.inputFiles("a.bed")[0]
        self.assertFilesEqual(expected_file, output_file)

    def test_file_not_found(self):
        rv, err = self.execute(["intersect", "qwert", "djfsoidjfdj"])
        self.assertEqual(2, rv)
        self.assertEqual("Failed to open file qwert\n", err)

    def test_invalid_arguments(self):
        rv, err = self.execute(["the bear went over the mountain"])
        self.assertEqual(1, rv)
        self.assertTrue(err.startswith("Invalid subcommand 'the'"))

    def test_unsorted_data(self):
        params = ["intersect"]
        params.extend(self.inputFiles("a.bed", "sort/unsorted0.bed"))
        rv, err = self.execute(params)
        self.assertEqual(3, rv)
        self.assertTrue(err.startswith("Unsorted data found in stream"))

    def test_output_format_extra_fields(self):
        output_file = self.tempFile("output.bed")
        params = [
            "intersect",
            "--format 'I A3-4'",
            "-o", output_file
        ]
        params.extend(self.inputFiles("a.bed", "b.bed"))
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)
        expected_file = self.inputFiles("expected-format-I,A3-4.bed")[0]
        self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()
