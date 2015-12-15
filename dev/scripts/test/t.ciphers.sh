#/bin/bash

# Name: t.ciphers.sh
# Author: Ladar Levison
#
# Description: Used for testing the supported ciphers using the OpenSSL binary.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../
MAGMA_DIST=`pwd`

if [ -z "$1" ]; then
	SERVER="localhost:9500"
	echo Testing the DEFAULT IMAP SERVER: $SERVER...
else
	SERVER="$1"
	echo Testing the provided IMAP server: $SERVER...
fi

DELAY=100

# Currently using the bundled OpenSSL binary

export LD_LIBRARY_PATH="$MAGMA_DIST/lib/sources/openssl/:$MAGMA_DIST/lib/sources/openssl/apps/"
export OPENSSL="$MAGMA_DIST/lib/sources/openssl/apps/openssl"
export OPENSSL_CONF="$MAGMA_DIST/lib/sources/openssl/apps/openssl.cnf"

# Uncomment to use the system provided OpenSSL binary

#unset LD_LIBRARY_PATH
#export OPENSSL=`which openssl`
#export OPENSSL_CONF="/etc/pki/tls/openssl.cnf"


echo Obtaining cipher list from `$OPENSSL version`.
echo Using $OPENSSL...
echo

export CIPHERS=`$OPENSSL ciphers 'ALL' | sed -e 's/:/ /g'`

for cipher in ${CIPHERS[@]}
do
echo ""
echo ""
printf "%-50s" "Testing $cipher..."
echo

RESULT=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -ssl2 2>&1`
if [[ "$RESULT" =~ ":error:" ]] ; then
  ERROR=`echo -n $RESULT | cut -d':' -f6`  
  printf "%-20.50s" " SSLv2   = NO ($ERROR)"
elif [[ "$RESULT" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " SSLv2   = YES"
else
  printf "%-20.50s" " SSLv2   = UNKNOWN RESPONSE ($RESULT)"
fi
echo 

RESULT=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -ssl3 -no_ssl2 2>&1`
if [[ "$RESULT" =~ ":error:" ]] ; then
  ERROR=`echo -n $RESULT | cut -d':' -f6`  
  printf "%-20.50s" " SSLv3   = NO ($ERROR)"
elif [[ "$RESULT" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " SSLv3   = YES"
else
  printf "%-20.50s" " SSLv3   = UNKNOWN RESPONSE ($RESULT)"
fi
echo 

RESULT=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -tls1 2>&1`
if [[ "$RESULT" =~ ":error:" ]] ; then
  ERROR=`echo -n $RESULT | cut -d':' -f6`
  printf "%-20.50s" " TLSv1   = NO ($ERROR)"
elif [[ "$RESULT" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " TLSv1   = YES"
else
  printf "%-20.50s" " TLSv1   = UNKNOWN RESPONSE ($RESULT)"
fi
echo

RESULT=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -tls1_1 2>&1`
if [[ "$RESULT" =~ ":error:" ]] ; then
  ERROR=`echo -n $RESULT | cut -d':' -f6`
  printf "%-20.50s" " TLSv1.1 = NO ($ERROR)"
elif [[ "$RESULT" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " TLSv1.1 = YES"
else
  printf "%-20.50s" " TLSv1.1 = UNKNOWN RESPONSE ($RESULT)"
fi
echo
 
RESULT=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -tls1_2 2>&1`
if [[ "$RESULT" =~ ":error:" ]] ; then
  ERROR=`echo -n $RESULT | cut -d':' -f6`
  printf "%-20.50s" " TLSv1.2 = NO ($ERROR)"
elif [[ "$RESULT" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " TLSv1.2 = YES"
else
  printf "%-20.50s" " TLSv1.2 = UNKNOWN RESPONSE ($RESULT)"
fi

usleep $DELAY
done

