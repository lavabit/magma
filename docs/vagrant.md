# Vagrant provisioning instructions

First install vagrant and virtualbox.
[This](https://github.com/dotless-de/vagrant-vbguest) plugin may also be useful
for keeping virtualbox guest additions in sync.  Other providers may work but
are as yet untested.

Note that magmad.so will be built the first time you invoke make, which takes
around 30 minutes.

```shell
vagrant up --provider=virtualbox # takes a few minutes
vagrant ssh
cd /vagrant # This folder is shared with the host system
autoreconf -i
mkdir build
cd build
../configure
make
make check
```

For a build with debug symbols:

```shell
mkdir debug
cd debug
../configure CPPFLAGS=-DDEBUG CFLAGS="-g -O0"
make
make check
```
