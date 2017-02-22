#!/bin/bash

# Name: linkup.sh
# Author: Ladar Levison
#
# Description: Used for creating shortcuts in a user's bin directory and the magma development scripts.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

process() {
	SRC=`find $MAGMA_DIST/dev/scripts/ -name $1`
	mkdir --parents "$HOME/bin"
	rm -f "$HOME/bin/$2"
	ln -s "$SRC" "$HOME/bin/$2"
	chmod +x "$HOME/bin/$2"
}

process "build.lib.sh" "build.lib"
process "build.docs.sh" "build.docs"
process "build.check.sh" "build.check"
process "build.magma.sh" "build.magma"

process "magma.vg.sh" "magma.vg"
process "magma.helgrind.sh" "magma.helgrind"
process "magma.mem.sh" "magma.mem"
process "magma.num.sh" "magma.num"
process "magma.run.sh" "magma.run"
process "magma.kill.sh" "magma.kill"
process "magma.loop.sh" "magma.loop"
process "magma.stat.sh" "magma.stat"
process "magma.pprof.sh" "magma.pprof"
process "magma.watcher.sh" "magma.watcher"
process "magma.killhard.sh" "magma.killhard"
process "magma.callgrind.sh" "magma.callgrind"

process "check.vg.sh" "check.vg"
process "check.run.sh" "check.run"
process "check.pprof.sh" "check.pprof"
process "check.massif.sh" "check.massif"
process "check.helgrind.sh" "check.helgrind"
process "check.callgrind.sh" "check.callgrind"
process "check.cachegrind.sh" "check.cachegrind"

process "tmap.sh" "tmap"
process "tpop.sh" "tpop"
process "tnet.sh" "tnet"
process "tweb.sh" "tweb"
process "tmolt.sh" "tmolt"
process "tlsmap.sh" "tlsmap"
process "tlspop.sh" "tlspop"
process "tlsnet.sh" "tlsnet"
process "sslpop.sh" "sslpop"
process "sslmap.sh" "sslmap"
process "sslnet.sh" "sslnet"
process "sslweb.sh" "sslweb"

process "mycon.sh" "mycon"
process "mua.reset.sh" "mua.reset"
process "schema.dump.sh" "schema.dump"
process "schema.reset.sh" "schema.reset"
process "schema.init.sh" "schema.init"

process "t.authplain.sh" "t.authplain"
process "t.authlogin.sh" "t.authlogin"
process "t.authstacie.sh" "t.authstacie"
process "t.outbound.sh" "t.outbound"
process "t.inbound.sh" "t.inbound"
process "t.dkimverify.sh" "t.dkimverify"
process "t.camel.sh" "t.camel"
process "t.ciphers.sh" "t.ciphers"
process "t.pop.sh" "t.pop"
process "t.imap.sh" "t.imap"
process "t.imap.id.sh" "t.imap.id"
process "t.imap.body.sh" "t.imap.body"
process "t.imap.purge.sh" "t.imap.purge"
process "t.http.options.sh" "t.http.options"
process "t.inbound.attachment.1.sh" "t.inbound.attachment.1"
process "t.inbound.attachment.2.sh" "t.inbound.attachment.2"
process "t.colors.sh" "t.colors"

process "freshen.clamav.sh" "freshen.clamav"
process "runner.sh" "runner"
process "testde.sh" "testde"

process "stacie.py" "stacie"
process "legacy.pl" "legacy"

process "linkup.sh" "linkup"


