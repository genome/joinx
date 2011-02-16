#!/usr/bin/env perl

use warnings;
use strict;

use POSIX;
use File::Basename qw/dirname/;
use File::Temp;
use Test::More tests => 1;

my $dir = dirname($0);
# TODO: get rid of hard coded version # here
my $exe = "$dir/../../bin/joinx sort";
my $cmd = "$exe no file for u";
my $rv = system($cmd);
is(WEXITSTATUS($rv), 1, "command exited with error as expected");
