#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../../

MAGMA_DIST=`pwd`

cd $MAGMA_DIST/res/corpus/
/usr/bin/time -f "%E" tar cf $HOME/Desktop/messages.tar *
cd $HOME/Desktop/
du -m messages.tar



/usr/bin/time -f "%E" pigz -9 -k messages.tar
du -m messages.tar.gz
rm -f messages.tar.gz

/usr/bin/time -f "%E" pbzip2 -9 -k messages.tar
du -m messages.tar.bz2
rm -f messages.tar.bz2

/usr/bin/time -f "%E" pxz -9 -e -k messages.tar
du -m messages.tar.xz
rm -f messages.tar.xz

/usr/bin/time -f "%E" pxz -k messages.tar
du -m messages.tar.xz
rm -f messages.tar.xz

/usr/bin/time -f "%E" lzop -9 messages.tar
du -m messages.tar.lzo
rm -f messages.tar.lzo

/usr/bin/time -f "%E" lbzip2 -9 -k messages.tar
du -m messages.tar.lzo
rm -f messages.tar.lzo

/usr/bin/time -f "%E" zip -9 messages.tar.zip messages.tar
du -m messages.tar.zip
rm -f messages.tar.zip

/usr/bin/time -f "%E" rar a -m5 messages.tar.rar messages.tar
du -m messages.tar.rar
rm -f messages.tar.rar
