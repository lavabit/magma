#!/bin/bash

mkdir ./../libcore/src  ./../libcore/check
cp -R  ./src/core/ ./../libcore/src
cp -R ./check/magma/core/ ./../libcore/check
cd ./../libcore
git add .
git commit
git push
