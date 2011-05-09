#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw/dirname/;
use File::Temp;
use Test::More tests => 4;

my $joinx_path = shift @ARGV;

my $dir = dirname($0);
# TODO: get rid of hard coded version # here
my $exe = "$joinx_path sort";
my $data_dir = "$dir/data"; 
my $input = join(" ", glob("$data_dir/input*.bed"));
my $expected = "$data_dir/expected.bed";

my $fh = new File::Temp;
my $tmpfile = $fh->filename;

my $cmd = "$exe $input -o $tmpfile";
my $rv = system($cmd);
is($rv&127, 0, "executed $cmd");
my $diff = `diff $tmpfile $expected`;
is($diff, '', "results are as expected (unstable sort)");

# test stable sort too
$cmd = "$exe -s $input -o $tmpfile";
$rv = system($cmd);
is($rv&127, 0, "executed $cmd");
$diff = `diff $tmpfile $expected`;
is($diff, '', "results are as expected (stable sort)");

$fh->close();

done_testing();
