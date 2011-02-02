#!/usr/bin/env perl

use warnings;
use strict;

use File::Basename qw/dirname/;
use File::Temp;
use Test::More tests => 12;

my $dir = dirname($0);
# TODO: get rid of hard coded version # here
my $exe = "$dir/../bin/joinx1.0";
my $data_dir = "$dir/data"; 
my $input = "$data_dir/a.bed $data_dir/b.bed";
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

for my $args (keys %data) {
    my $cmd = "$exe $input $args > $tmpfile";
    my $rv = system($cmd);
    is($rv&127, 0, "executed $cmd");
    my $diff = `diff $tmpfile $data_dir/$data{$args}`;
    is($diff, '', "results are as expected for '$args'");
}
$fh->close();

done_testing();
