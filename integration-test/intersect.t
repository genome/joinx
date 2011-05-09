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

subtest 'intersect' => sub {
    my %data = (
        '--exact-allele --output-both' => 'expected-exact-allele-both.bed',
        '--exact-allele' => 'expected-exact-allele.bed',
        '--exact-pos --output-both' => 'expected-exact-pos-both.bed',
        '--exact-pos' => 'expected-exact-pos.bed',
        '--output-both' => 'expected-noargs-both.bed',
        '' => 'expected-noargs.bed',
    );

    my $fh = new File::Temp;
    my $tmpfile = $fh->filename;

    my $input = "$data_dir/a.bed $data_dir/b.bed";
    for my $args (keys %data) {
        my $rv = $jx->exec("intersect $input $args -o $tmpfile");
        is($rv, 0, "executed with $args");
        my $diff = `diff $tmpfile $data_dir/$data{$args}`;
        is($diff, '', "results are as expected for args '$args'");
    }
    $fh->close();
};

subtest 'partial match' => sub {
    my $fh = new File::Temp;
    my $tmpfile = $fh->filename;

    my $input = "$data_dir/iub-a.bed $data_dir/iub-b.bed";
    my $rv = $jx->exec("intersect $input --exact-allele --iub-match --output-both -o $tmpfile");
    is($rv, 0, "executed command");
    my $diff = `diff $tmpfile $data_dir/expected-iub-both.bed`;
    is($diff, '', "results are as expected");
    $fh->close();
};

subtest 'file not found' => sub {
    my ($rv, $out, $err) = $jx->exec("intersect nonexisting files");
    is($rv, 1, "expected error: file not found");
    like($err, qr/Failed to open input file 'nonexisting'/, 'got file not found error message');
};

subtest 'invalid arguments' => sub {
    my ($rv, $out, $err) = $jx->exec("intersect --qwert $data_dir/a.bed $data_dir/b.bed");
    is($rv, 1, "expected error: invalid argument");
    like($err, qr/unknown option qwert/, 'got invalid argument error message');
};

subtest 'unsorted data' => sub {
    my ($rv, $out, $err) = $jx->exec("intersect $data_dir/a.bed $data_dir/unsorted0.bed");
    is($rv, 1, "expected error: unsorted data");
    like($err, qr/Unsorted data.*unsorted0.bed/, 'got unsorted data error message');
    ($rv, $out, $err) = $jx->exec("intersect $data_dir/unsorted0.bed $data_dir/a.bed");
    is($rv, 1, "expected error: unsorted data");
    like($err, qr/Unsorted data.*unsorted0.bed/, 'got unsorted data error message');
};

subtest 'position only' => sub {
    my $fh = new File::Temp;
    my $tmpfile = $fh->filename;
    my ($rv, $out, $err) = $jx->exec("intersect $data_dir/a.bed $data_dir/posonly.bed -o $tmpfile");
    is($rv, 0, "executed command");
    my $diff = `diff $tmpfile $data_dir/a.bed`;
    is($diff, '', "results are as expected for position only intersection");
    $fh->close();
};

done_testing();
