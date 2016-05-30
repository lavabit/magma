#!/usr/bin/perl

use DBI;
use Digest::SHA qw(sha512_hex sha512 sha512_base64);

#$salt = "random";
$salt = "+!l6M6rz0c0!+";

# Read in our userid and password from the command line.
$userid = @ARGV[0];
$inpass = @ARGV[1];

if (length($userid) == 0  or length($inpass) == 0) {
	print "Usage: out_password.pl userid password\n";
	exit;
	}


$key = sha512_base64($userid . $salt . $inpass);
$token = sha512_base64($inpass, sha512($inpass, sha512($userid . $salt . $inpass)));

$key =~ s/\//\_/g;
$key =~ s/\+/\-/g; 	
$token =~ s/\//\_/g;
$token =~ s/\+/\-/g; 	

print "key = " . $key . "\n";
print "token = " . $token . "\n";


