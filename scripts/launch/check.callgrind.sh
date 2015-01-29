#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]
#

#### Still not tracking forks/threads properly!!

valgrind --tool=callgrind --trace-children=yes --demangle=yes --num-callers=50 --error-limit=no \
--show-below-main=no --max-stackframe=20000000 --dsymutil=yes --fullpath-after= --read-var-info=yes \
--dump-line=yes --compress-strings=yes --compress-pos=yes --collect-systime=yes --collect-bus=yes \
--zero-before=main --dump-instr=yes --trace-jump=yes --collect-atstart=yes --instr-atstart=yes  \
--callgrind-out-file=$HOME/callgrind.out.%p \
--suppressions=$MAGMA_DIST/res/config/magma.supp --suppressions=/usr/lib64/valgrind/default.supp \
$MAGMA_DIST/check/.check/magmad.check $MAGMA_DIST/res/config/magma.sandbox.config &

export VPID=$!
export VLOG=$HOME/Desktop/callgrind.out.$VPID

wait $VPID

if [ -e "$VLOG" ] && [ -f "$VLOG" ]; then 
	qcachegrind $VLOG &
	#kcachegrind $VLOG &
fi
