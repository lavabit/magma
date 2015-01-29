#!/bin/bash

echo ""
sync; echo 3 | tee /proc/sys/vm/drop_caches &> /dev/null; memflush --servers=localhost:11211
su ladar -l -c "/usr/bin/time --verbose bash -c 'Lavabit/magma.so/scripts/build all &> /dev/null'"
echo ""
echo ""
sync; echo 3 | tee /proc/sys/vm/drop_caches &> /dev/null; memflush --servers=localhost:11211
su ladar -l -c "/usr/bin/time --verbose bash -c 'build all &> /dev/null'"
echo ""
updatedb
