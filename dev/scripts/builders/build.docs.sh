#!/bin/bash -e

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

# Remove any existing Latex files to ensure a clean build process.
find dev/docs/latex/ -type f -exec rm --force {} \;

# Extract the current Magma version from the Makefile.
MAGMA_VERSION=`grep --extended-regexp "^PACKAGE_VERSION" $MAGMA_DIST/Makefile | awk --field-separator='=' '{print \$2}' | tr --delete [:blank:]`

# If the script is called from within the Magma repo, determine the patch number.
MAGMA_PATCH=`which git &> /dev/null && git log &> /dev/null && printf "." && git log --format='.%H' | wc -l`

# Update the project version information inside the Doxyfile.
sed --in-place --expression="s/^PROJECT_NUMBER = [0-9]*\.[0-9]*\.\?[0-9]*$/PROJECT_NUMBER = $MAGMA_VERSION$MAGMA_PATCH/g" $MAGMA_DIST/dev/scripts/builders/build.doxyfile

# Extract the Doxygen comments and generate the documentation as a collection of HTML and Latex files. 
doxygen $MAGMA_DIST/dev/scripts/builders/build.doxyfile

# Refman template tweaks.
sed --in-place --expression="/clearemptydoublepage/d" $MAGMA_DIST/dev/docs/latex/refman.tex
sed --in-place --expression="/Generated by Doxygen/d" $MAGMA_DIST/dev/docs/latex/refman.tex
sed --in-place --expression="3i\\\\\usepackage[top=0.75in, bottom=0.75in, left=0.35in, right=0.35in]{geometry}" $MAGMA_DIST/dev/docs/latex/refman.tex

# Document styling tweaks.
sed --in-place --expression="/Generated on .* for Magma by Doxygen/d" $MAGMA_DIST/dev/docs/latex/doxygen.sty
sed --in-place --expression="/rfoot/d" $MAGMA_DIST/dev/docs/latex/doxygen.sty
sed --in-place --expression="/lfoot/d" $MAGMA_DIST/dev/docs/latex/doxygen.sty
sed --in-place --expression="/cfoot/d" $MAGMA_DIST/dev/docs/latex/doxygen.sty

# Convert the Latex files into a single PDF file.
cd $MAGMA_DIST/dev/docs/latex/ && make refman.pdf

# Over the magma-api.pdf file with the resulting PDF.
mv --force $MAGMA_DIST/dev/docs/latex/refman.pdf $MAGMA_DIST/dev/docs/magma-api.pdf
