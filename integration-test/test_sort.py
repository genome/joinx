#!/usr/bin/env python

from joinxtest import JoinxTest, main
import unittest

class TestSort(JoinxTest, unittest.TestCase):

    def test_sort(self):
        input_files = self.inputFiles("unsorted*.bed")
        expected_file = self.inputFiles("expected-sort.bed")[0]
        output_file = self.tempFile("output.bed")

        # test normal and stable sort
        for arg in ["", "-s"]:
            params = [ "sort", "-o", output_file, arg ]
            params.extend(input_files)
            rv, err = self.joinx(params)
            self.assertEqual(0, rv)
            self.assertEqual('', err)
            self.assertFilesEqual(expected_file, output_file)

if __name__ == "__main__":
    main()

