#/usr/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

echo $SELECTION 
#| /usr/local/bin/uncrustify -l c -c /home/ladar/Lavabit/magma.universe/environment/uncrustify/uncrustify.cfg

