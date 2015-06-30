#!/bin/sh

CHECK_DIR="`dirname \"$0\"`"
BUILD_DIR="`pwd`"
echo "$BUILD_DIR/tests $CHECK_DIR/../res/config/magma.sandbox.config"
"$BUILD_DIR/tests" "$CHECK_DIR/../res/config/magma.sandbox.config"
