#!/bin/bash
# TODO: Try using sslscan to make things more robust.
# http://sourceforge.net/projects/sslscan/
LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../
MAGMA_DIST=`pwd`

DELAY=100
SERVER="lavabit.com:993"
#SERVER="localhost.lavabit.com:9500"
export LD_LIBRARY_PATH="$MAGMA_DIST/lib/sources/openssl/:$MAGMA_DIST/lib/sources/openssl/apps/"
export OPENSSL="$MAGMA_DIST/lib/sources/openssl/apps/openssl"
#export OPENSSL_CONF="$MAGMA_DIST/lib/sources/openssl/apps/openssl.cnf"
export OPENSSL_CONF="/etc/pki/tls/openssl.cnf"
export CIPHERS=`$OPENSSL ciphers 'ALL' | sed -e 's/:/ /g'`


echo Testing $SERVER...
echo Using $OPENSSL...
echo 
echo Obtaining cipher list from `$OPENSSL version`.
echo

for cipher in ${CIPHERS[@]}
do
echo ""
echo ""
printf "%-50s" "Testing $cipher..."
echo
result=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -ssl2 2>&1`
#echo $result
if [[ "$result" =~ ":error:" ]] ; then
  error=`echo -n $result | cut -d':' -f6`  
  echo -n SSLv2 = NO \($error\)
  #printf "%-20.20s" " SSLv2 = NO"
elif [[ "$result" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " SSLv2 = YES"
else
  printf "%-20.20s" " SSLv2 = UNKNOWN RESPONSE"
  #echo $result
fi
echo 

result=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -ssl3 -no_ssl2 2>&1`
#echo $result
if [[ "$result" =~ ":error:" ]] ; then
  error=`echo -n $result | cut -d':' -f6`  
  echo -n SSLv3 = NO \($error\)
  #printf "%-20.20s" " SSLv3 = NO"
elif [[ "$result" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " SSLv3 = YES"
else
  printf "%-20.20s" " SSLv3 = UNKNOWN RESPONSE"
  #echo $result
fi
echo 

result=`printf "A00 LOGOUT\r\n" | $OPENSSL s_client -crlf -cipher "$cipher" -connect $SERVER -tls1 2>&1`
#echo $result
if [[ "$result" =~ ":error:" ]] ; then
  error=`echo -n $result | cut -d':' -f6`
  echo -n TLSv1 = NO \($error\)
  #printf "%-20.20s" " TLSv1 = NO"
elif [[ "$result" =~ "Cipher is " ]] ; then
  printf "%-20.20s" " TLSv1 = YES"
else
  printf "%-20.20s" " TLSv1 = UNKNOWN RESPONSE"
  #echo $result
fi

echo 
usleep $DELAY
done

