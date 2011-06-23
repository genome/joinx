#!/usr/bin/env perl

use warnings;
use strict;

use FindBin;
use lib "$FindBin::Bin";
use JoinxTest;
use POSIX;
use File::Temp;
use Test::More;

my $joinx_path = shift @ARGV;
my $jx = new JoinxTest($joinx_path);
my $data_dir = "$FindBin::Bin/data"; 

subtest 'create contigs' => sub {
    my $fh = new File::Temp;
    my $tmpfile = $fh->filename;

    my $input = "$data_dir/small.fa $data_dir/variants-contig.bed";
    my $rv = $jx->exec("create-contigs $input --flank=10 -o $tmpfile");
    is($rv, 0, "executed command");
    my $diff = `diff $tmpfile $data_dir/expected-contigs.fa`;
    is($diff, '', "results are as expected");
    $fh->close();
};
