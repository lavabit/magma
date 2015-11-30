#/bin/bash

# meant to convert the auto-generated eclipse make files to more general make files with absolute paths
# incomplete and inactive TODO

echo "Update the makefiles"
export REPLACE="${\/home\/ladar//\//\\/}"

cd $HOME/Lavabit/magma/.debug/
find -name "*.mk" -exec sed -i -e "s/$REPLACE\//\\\$HOME\//g" {} \;

cd $HOME/Lavabit/magma/.check/
find -name "*.mk" -exec sed -i -e "s/$REPLACE\//\\\$HOME\//g" {} \;

cd $HOME/Lavabit/magma.check/.check/
find -name "*.mk" -exec sed -i -e "s/$REPLACE\//\\\$HOME\//g" {} \;
