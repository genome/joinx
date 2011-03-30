#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw/dirname/;
use File::Temp;
use Test::More tests => 2;

my $dir = dirname($0);
# TODO: get rid of hard coded version # here
my $exe = "$dir/../../bin/joinx intersect";
my $data_dir = "$dir/data"; 
my $input = "$data_dir/iub-a.bed $data_dir/iub-b.bed";
my $fh = new File::Temp;
my $tmpfile = $fh->filename;

my $cmd = "$exe $input --exact-allele --iub-match --output-both > $tmpfile";
my $rv = system($cmd);
is($rv&127, 0, "executed $cmd");
my $diff = `diff $tmpfile $data_dir/expected-iub-both.bed`;
is($diff, '', "results are as expected");
$fh->close();

done_testing();
