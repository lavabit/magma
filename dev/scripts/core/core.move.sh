#!/bin/bash

MESSAGE=$(git show -s --format='%s' | cat)

echo "testing 1"
mkdir ./../libcore/src  ./../libcore/check
cp -R  ./src/core/ ./../libcore/src
cp -R ./check/magma/core/ ./../libcore/check
cd ./../libcore
git add .
git commit -a -m "$MESSAGE" -e
git push

cd ./../libcore
make || echo "WARNING: core is in a broken state and wont build. Please fix."
cd ./../magma