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


$key1 = sha512_base64($userid . $salt . $inpass);
$key2 = sha512_hex($userid . $salt . $inpass);
$token1 = sha512_base64($inpass, sha512($inpass, sha512($userid . $salt . $inpass)));
$token2 = sha512_hex($inpass, sha512($inpass, sha512($userid . $salt . $inpass)));

$key1 =~ s/\//\_/g;
$key1 =~ s/\+/\-/g; 	
$token1 =~ s/\//\_/g;
$token1 =~ s/\+/\-/g; 	

print "key[b64] = " . $key1 . "\n";
print "key[hex] = " . $key2 . "\n";
print "token[b64] = " . $token1 . "\n";
print "token[hex] = " . $token2 . "\n";

