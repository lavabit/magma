#!/bin/bash

mkdir ./../libcore/src  ./../libcore/check
cp -Rv  ./src/core/* ./../libcore/src
cp -Rv ./check/magma/core/* ./../libcore/check
cd ./../libcore
git add .
git commit
git push
