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

subtest 'sort' => sub {
    my $input = join(" ", glob("$data_dir/unsorted*.bed"));
    my $expected = "$data_dir/expected-sort.bed";

    my $fh = new File::Temp;
    my $tmpfile = $fh->filename;

    my $rv = $jx->exec("sort $input -o $tmpfile");
    is($rv, 0, "executed command");
    my $diff = `diff $tmpfile $expected`;
    is($diff, '', "results are as expected (unstable sort)");

    # test stable sort too
    $rv = $jx->exec("sort -s $input -o $tmpfile");
    is($rv, 0, "executed command");
    $diff = `diff $tmpfile $expected`;
    is($diff, '', "results are as expected (stable sort)");

    $fh->close();
};

subtest 'file not found' => sub {
    my ($rv, $out, $err) = $jx->exec("sort no file for you");
    is($rv, 1, "expected error: file not found");
    like($err, qr/Failed to open input file 'no'/, 'got file not found error message');

};

done_testing();
