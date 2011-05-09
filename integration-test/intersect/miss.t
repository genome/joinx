#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw/dirname/;
use File::Temp;
use Test::More tests => 3;

my $joinx_path = shift @ARGV;

my $dir = dirname($0);
# TODO: get rid of hard coded version # here
my $exe = "$joinx_path intersect";
my $data_dir = "$dir/data"; 
my $input = "$data_dir/a.bed $data_dir/b.bed";

my ($fh_a, $fh_b) = (new File::Temp, new File::Temp);
my $tmpfile_a = $fh_a->filename;
my $tmpfile_b = $fh_b->filename;

my $cmd = "$exe $input --miss-a $tmpfile_a --miss-b $tmpfile_b > /dev/null";
my $rv = system($cmd);
is($rv&127, 0, "executed $cmd");
my $diff = `diff $tmpfile_a $data_dir/expected-miss-a.bed`;
is($diff, '', "results in miss-a are as expected");
$diff = `diff $tmpfile_b $data_dir/expected-miss-b.bed`;
is($diff, '', "results in miss-b are as expected");
$fh_a->close();
$fh_b->close();

done_testing();
