#!/usr/bin/env python

from integrationtest import IntegrationTest, main
import unittest

class TestCmdline(IntegrationTest, unittest.TestCase):

    def test_help(self):
        params = [ "-h" ]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

    def test_version(self):
        params = [ "-v" ]
        rv, err = self.execute(params)
        self.assertEqual(0, rv)
        self.assertEqual('', err)

    def test_command_help(self):
        cmds = ['bed-merge', 'check-ref', 'create-contigs', 'intersect',
                'ref-stats', 'snv-concordance', 'snv-concordance-by-quality',
                'sort', 'vcf-annotate', 'vcf-filter', 'vcf-merge',
                'vcf-normalize-indels', 'vcf-report', 'vcf-site-filter',
                'vcf2raw', 'wig2bed']

        for cmd in cmds:
            params = [cmd, '-h']
            print params
            rv, err = self.execute(params)
            self.assertEqual(0, rv)
            self.assertEqual('', err)

if __name__ == "__main__":
    main()

