FROM centos:6

RUN yum -y install wget;\
 wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm;\
 wget http://rpms.famillecollet.com/enterprise/remi-release-6.rpm;\
 rpm -Uvh remi-release-6*.rpm epel-release-6*.rpm

RUN yum -y install gcc make autoconf automake binutils bison flex gcc-c++ gettext libtool make patch \
pkgconfig mysql-server memcached gettext-devel patch perl perl-Time-HiRes check check-devel \
ncurses-devel libbsd-devel zlib-devel valgrind valgrind-devel libbsd

ADD web magma/web
ADD tools magma/tools
ADD sandbox magma/sandbox
ADD res magma/res
ADD Makefile magma
ADD lib magma/lib
ADD dev magma/dev
ADD LICENSE magma
ADD COPYRIGHT magma
ADD src/providers/symbols.h magma/src/providers/
ENV TERM dumb

RUN cd magma && dev/scripts/builders/build.lib.sh all

RUN cd magma && rm -rf lib/archives lib/check lib/logs lib/patches lib/sources lib/objects

ADD src magma/src
ADD check magma/check

RUN cd magma && make -j4 magmad

ENTRYPOINT [ "/bin/bash" ]