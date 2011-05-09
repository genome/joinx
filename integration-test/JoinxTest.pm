package JoinxTest;

use strict;
use warnings;

use POSIX;
use IPC::Open3;
use File::Temp;
use File::Slurp qw/read_file/;

sub new {
    my ($class, $joinx_path) = @_;
    my $self = {};
    $self->{joinx_path} = $joinx_path;
    bless $self, $class;
    return $self;
};

sub exec {
    my ($self, @args) = @_;
    my $fh_out = new File::Temp;
    my $fh_err = new File::Temp;
    my $fh_in;
    my $cmd = join(" ", $self->{joinx_path}, @args);
    my $out_path = $fh_out->filename;
    my $err_path = $fh_err->filename;
    my $rv = system("$cmd > $out_path 2> $err_path");
    $rv = WEXITSTATUS($?);
	$? = 0;
    return $rv unless wantarray;
    my $txt_out = read_file($fh_out->filename);
    my $txt_err = read_file($fh_err->filename);
    return ($rv, $txt_out, $txt_err);
}

1;
