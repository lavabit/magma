#!/usr/bin/make
#
# The Magma Makefile
#
#########################################################################

TOPDIR                        = $(realpath .)
MFLAGS                        =
MAKEFLAGS                     = --output-sync=target

# Identity of this package.
PACKAGE_NAME                  = Magma Daemon
PACKAGE_TARNAME               = magma
PACKAGE_VERSION               = 7.0
PACKAGE_STRING                = $(PACKAGE_NAME) $(PACKAGE_VERSION)
PACKAGE_BUGREPORT             = support@lavabit.com
PACKAGE_URL                   = https://lavabit.com

MAGMA_PROGRAM                 = $(addsuffix $(EXEEXT), magmad)
MAGMA_CHECK_PROGRAM           = $(addsuffix $(EXEEXT), magmad.check)
MAGMA_SHARED_LIBRARY          = $(addsuffix $(DYNLIBEXT), magmad)
MAGMA_STATIC_LIBRARY          = $(addsuffix $(STATLIBEXT), magmad)

MAGMA_PROGRAM_GPROF           = $(addsuffix $(EXEEXT), magmad.gprof)
MAGMA_CHECK_PROGRAM_GPROF     = $(addsuffix $(EXEEXT), magmad.check.gprof)

MAGMA_PROGRAM_PPROF           = $(addsuffix $(EXEEXT), magmad.pprof)
MAGMA_CHECK_PROGRAM_PPROF     = $(addsuffix $(EXEEXT), magmad.check.pprof)

MAGMA_PROGRAM_STRIPPED        = $(addsuffix .stripped, $(MAGMA_PROGRAM))
MAGMA_CHECK_PROGRAM_STRIPPED  = $(addsuffix .stripped, $(MAGMA_CHECK_PROGRAM))
MAGMA_SHARED_LIBRARY_STRIPPED = $(addsuffix .stripped, $(MAGMA_SHARED_LIBRARY))
MAGMA_STATIC_LIBRARY_STRIPPED = $(addsuffix .stripped, $(MAGMA_STATIC_LIBRARY))

DIME_PROGRAM                  = $(addsuffix $(EXEEXT), dime)
SIGNET_PROGRAM                = $(addsuffix $(EXEEXT), signet)
GENREC_PROGRAM                = $(addsuffix $(EXEEXT), genrec)
DIME_CHECK_PROGRAM            = $(addsuffix $(EXEEXT), dime.check)

# Source Files
BUILD_SRCFILES                = src/engine/status/build.c

MAGMA_STATIC                  =
MAGMA_DYNAMIC                 = -lrt -ldl -lpthread -lresolv
MAGMA_SRCDIRS                 = $(shell find src -type d -print)
MAGMA_SRCFILES                = $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(MAGMA_SRCDIRS), $(wildcard $(dir)/*.c)))

MAGMA_CHECK_STATIC            = $(MAGMA_STATIC) \
                                $(TOPDIR)/lib/local/lib/libcheck$(STATLIBEXT)
MAGMA_CHECK_DYNAMIC           = $(MAGMA_DYNAMIC) -lm
MAGMA_CHECK_SRCDIRS           = $(shell find check/magma -type d -print)
MAGMA_CHECK_SRCFILES          = $(foreach dir, $(MAGMA_CHECK_SRCDIRS), $(wildcard $(dir)/*.c))

DIME_SRCDIRS                  = $(shell find src/ tools/dime -type d -print)
DIME_SRCFILES                 = $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(DIME_SRCDIRS), $(wildcard $(dir)/*.c)))
DIME_STATIC                   = $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libssl$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libutf8proc$(STATLIBEXT)

SIGNET_SRCDIRS                = $(shell find src/ tools/signet -type d -print)
SIGNET_SRCFILES               = $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(SIGNET_SRCDIRS), $(wildcard $(dir)/*.c)))
SIGNET_STATIC                 = $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libssl$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libutf8proc$(STATLIBEXT)

GENREC_SRCDIRS                = $(shell find src/ tools/genrec -type d -print)
GENREC_SRCFILES               = $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(GENREC_SRCDIRS), $(wildcard $(dir)/*.c)))
GENREC_STATIC                 = $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libssl$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libutf8proc$(STATLIBEXT)

DIME_CHECK_DYNAMIC            = $(MAGMA_DYNAMIC) -lstdc++
DIME_CHECK_CPPDIRS            = $(shell find check/dime -type d -print)
DIME_CHECK_SRCDIRS            = $(shell find src/ check/dime -type d -print)
DIME_CHECK_CPPFILES           = $(foreach dir, $(DIME_CHECK_CPPDIRS), $(wildcard $(dir)/*.cpp))
DIME_CHECK_SRCFILES           = $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(DIME_CHECK_SRCDIRS), $(wildcard $(dir)/*.c)))
DIME_CHECK_STATIC             = $(MAGMA_STATIC) \
                                $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libssl$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT) \
                                $(TOPDIR)/lib/local/lib/libutf8proc$(STATLIBEXT) \
                                $(TOPDIR)/lib/sources/googtest/lib/.libs/libgtest$(STATLIBEXT)

FILTERED_SRCFILES             = src/providers/dime/ed25519/test.c \
                                src/providers/dime/ed25519/test-internals.c \
                                src/providers/dime/ed25519/fuzz/curve25519-ref10.c \
                                src/providers/dime/ed25519/fuzz/ed25519-donna-sse2.c \
                                src/providers/dime/ed25519/fuzz/fuzz-curve25519.c \
                                src/providers/dime/ed25519/fuzz/ed25519-donna.c \
                                src/providers/dime/ed25519/fuzz/ed25519-ref10.c \
                                src/providers/dime/ed25519/fuzz/fuzz-ed25519.c

#PACKAGE_DEPENDENCIES         = $(MAGMA_SHARED_LIBRARY) $(MAGMA_STATIC) $(filter-out $(MAGMA_STATIC), $(MAGMA_CHECK_STATIC))
#PACKAGE_DEPENDENCIES          = $(MAGMA_STATIC) $(filter-out $(MAGMA_STATIC), $(MAGMA_CHECK_STATIC))


# Bundled Dependency Include Paths
INCDIR                        = $(TOPDIR)/lib/local/include
MAGMA_INCDIRS                 = spf2/ mysql/ openssl/ lzo/ libxml2/ libmemcached/ opendkim/ dspam/ freetype2/
MAGMA_CHECK_INCDIRS           =

MAGMA_CINCLUDES               = -Isrc -Isrc/providers -I$(INCDIR) $(addprefix -I,$(MAGMA_INCLUDE_ABSPATHS))
DIME_CHECK_CINCLUDES          = $(MAGMA_CINCLUDES)
MAGMA_CHECK_CINCLUDES         = -Icheck/magma \
                                -Ilib/local/include/ \
                                $(MAGMA_CINCLUDES) \
                                $(addprefix -I,$(MAGMA_CHECK_INCLUDE_ABSPATHS))
DIME_CHECK_CPPINCLUDES        = -Icheck/dime \
                                -Ilib/sources/googtest/include/ \
                                -Ilib/sources/googtest/ \
                                -Ilib/sources/googtap/src/ \
                                $(MAGMA_CINCLUDES)

CDEFINES                      = -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE \
                                -DHAVE_NS_TYPE -DFORTIFY_SOURCE=2 -DMAGMA_PEDANTIC
CDEFINES.build.c              = -DMAGMA_VERSION=\"$(MAGMA_VERSION)\" \
                                -DMAGMA_COMMIT=\"$(MAGMA_COMMIT)\" \
                                -DMAGMA_TIMESTAMP=\"$(MAGMA_TIMESTAMP)\"

CPPDEFINES                    = $(CDEFINES) -DGTEST_TAP_PRINT_TO_STDOUT -DGTEST_HAS_PTHREAD=1

# Hidden Directory for Dependency Files
DEPDIR                        = .deps
DIME_DEPFILES                 = $(patsubst %.c,$(DEPDIR)/%.d,$(DIME_SRCFILES))
MAGMA_DEPFILES                = $(patsubst %.c,$(DEPDIR)/%.d,$(MAGMA_SRCFILES))
SIGNET_DEPFILES               = $(patsubst %.c,$(DEPDIR)/%.d,$(SIGNET_SRCFILES))
GENREC_DEPFILES               = $(patsubst %.c,$(DEPDIR)/%.d,$(GENREC_SRCFILES))
MAGMA_CHECK_DEPFILES          = $(patsubst %.c,$(DEPDIR)/%.d,$(MAGMA_CHECK_SRCFILES))
DIME_CHECK_DEPFILES           = $(patsubst %.c,$(DEPDIR)/%.d,$(DIME_CHECK_SRCFILES))
DIME_CHECK_DEPFILES          += $(patsubst %.cc,$(DEPDIR)/%.d,$(DIME_CHECK_CCFILES))
DIME_CHECK_DEPFILES          += $(patsubst %.cpp,$(DEPDIR)/%.d,$(DIME_CHECK_CPPFILES))

# Hidden Directory for Object Files
OBJDIR                        = .objs
MAGMA_OBJFILES                = $(patsubst %.c,$(OBJDIR)/%.o,$(MAGMA_SRCFILES))
MAGMA_CHECK_OBJFILES          = $(patsubst %.c,$(OBJDIR)/%.o,$(MAGMA_CHECK_SRCFILES))

MAGMA_PROF_OBJFILES           = $(patsubst %.c,$(OBJDIR)/%.pg.o,$(MAGMA_SRCFILES))
MAGMA_CHECK_PROF_OBJFILES     = $(patsubst %.c,$(OBJDIR)/%.pg.o,$(MAGMA_CHECK_SRCFILES))

DIME_OBJFILES                 = $(filter-out .objs/src//magma.o, $(patsubst %.c,$(OBJDIR)/%.o,$(DIME_SRCFILES)))
SIGNET_OBJFILES               = $(filter-out .objs/src//magma.o, $(patsubst %.c,$(OBJDIR)/%.o,$(SIGNET_SRCFILES)))
GENREC_OBJFILES               = $(filter-out .objs/src//magma.o, $(patsubst %.c,$(OBJDIR)/%.o,$(GENREC_SRCFILES)))
DIME_CHECK_OBJFILES           = $(filter-out .objs/src//magma.o, $(patsubst %.c,$(OBJDIR)/%.o,$(DIME_CHECK_SRCFILES)))
DIME_CHECK_OBJFILES          += $(patsubst %.cc,$(OBJDIR)/%.o,$(DIME_CHECK_CCFILES))
DIME_CHECK_OBJFILES          += $(patsubst %.cpp,$(OBJDIR)/%.o,$(DIME_CHECK_CPPFILES))

# Resolve the External Include Directory Paths
INCLUDE_DIR_VPATH             = $(INCDIR) /usr/include /usr/local/include
INCLUDE_DIR_SEARCH            = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(INCLUDE_DIR_VPATH)))))

# Generate the Absolute Directory Paths for Includes
MAGMA_INCLUDE_ABSPATHS       += $(foreach target,$(MAGMA_INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))
MAGMA_CHECK_INCLUDE_ABSPATHS += $(foreach target,$(MAGMA_CHECK_INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))

# C Compiler
ifeq ($(patsubst undefined,default,$(origin CC)),default)
CC  = gcc
endif

# C++ Preprocessor
ifeq ($(patsubst undefined,default,$(origin CPP)),default)
CPP  = gcc
endif

# C++ Compiler
ifeq ($(patsubst undefined,default,$(origin CXX)),default)
CXX  = gcc
endif

# Linker
ifeq ($(patsubst undefined,default,$(origin LD)),default)
LD  = gcc
endif

# Archiver
ifeq ($(patsubst undefined,default,$(origin AR)),default)
AR  = ar
endif

# Symbol Stripper
ifeq ($(patsubst undefined,default,$(origin STRIP)),default)
STRIP  = strip
endif

# File Installer
ifeq ($(patsubst undefined,default,$(origin INSTALL)),default)
INSTALL  = install
endif

# Archive Symbol Updater
ifeq ($(patsubst undefined,default,$(origin RANLIB)),default)
RANLIB  = ranlib
endif

# Verbosity Control
ifeq ($(patsubst undefined,default,$(origin VERBOSE)),default)
VERBOSE  = no
endif

# Quick Dependency Builds
ifeq ($(patsubst undefined,default,$(origin QUICK)),default)
QUICK  = yes
endif

# C Compiler Options
CFLAGS                       ?=
CFLAGS_WARNINGS               = -Wall -Werror -Winline -Wformat-security -Warray-bounds
CFLAGS_PEDANTIC               = -Wextra -Wpacked -Wunreachable-code -Wformat=2
CFLAGS_COMBINED               = -std=gnu99 -O0 -fPIC -fmessage-length=0 -ggdb3 -c $(CFLAGS_WARNINGS) -MMD $(CFLAGS)

# C++ Compiler Options
CPPFLAGS                     ?= 
CPPFLAGS_WARNINGS             = -Werror -Wall -Wextra -Wformat=2 -Wwrite-strings -Wno-format-nonliteral
CPPFLAGS_COMBINED             = -std=c++0x $(CPPFLAGS_WARNINGS) -Wno-unused-parameter -pthread -g3 $(CPPFLAGS)

# Linker Options
LDFLAGS                      ?=
LDFLAGS_COMBINED              = -rdynamic $(LDFLAGS)

# Archiver Options
ARFLAGS                       = rcs

# Strip Options
STRIPFLAGS                    = --strip-debug

# GProf Options
GPROF                         = -pg -finstrument-functions -fprofile-arcs -ftest-coverage

# PProf Options
PPROF                         = -lprofiler

# Miscellaneous External programs
MV                            = mv --force
RM                            = rm --force
RMDIR                         = rmdir --parents --ignore-fail-on-non-empty
MKDIR                         = mkdir --parents

# Control the Text Color/Weight if the TERM supports it. If no TERM is available, then
# default to using vt100 as the terminal type.
ifdef TERM
  RED                           = $$(tput setaf 1 || true)
  BLUE                          = $$(tput setaf 4 || true)
  GREEN                         = $$(tput setaf 2 || true)
  WHITE                         = $$(tput setaf 7 || true)
  YELLOW                        = $$(tput setaf 3 || true)
  BOLD                          = $$(tput bold || true)
  NORMAL                        = $$(tput sgr0 || true)
else
  RED                           = $$(if [ -t 0 ]; then tput -Tvt100 setaf 1 ; else true ; fi)
  BLUE                          = $$(if [ -t 0 ]; then tput -Tvt100 setaf 4 ; else true ; fi)
  GREEN                         = $$(if [ -t 0 ]; then tput -Tvt100 setaf 2 ; else true ; fi)
  WHITE                         = $$(if [ -t 0 ]; then tput -Tvt100 setaf 7 ; else true ; fi)
  YELLOW                        = $$(if [ -t 0 ]; then tput -Tvt100 setaf 3 ; else true ; fi)
  BOLD                          = $$(if [ -t 0 ]; then tput -Tvt100 bold ; else true ; fi)
  NORMAL                        = $$(if [ -t 0 ]; then tput -Tvt100 sgr0 ; else true ; fi)
endif

# Calculate the version, commit and timestamp strings.
MAGMA_REPO                    = $(strip $(shell which git 2>&1 > /dev/null && git log 2>&1 > /dev/null && echo '1'))
ifneq ($(strip $(MAGMA_REPO)),1)
MAGMA_VERSION                := $(PACKAGE_VERSION)
MAGMA_COMMIT                 := "NONE"
else
# Use the number of commits since the v7.0.0 tag as the patch level.
MAGMA_VERSION                := $(PACKAGE_VERSION).$(shell git log `git log -n 1 v7.0.0 --pretty='%H'`..`git log --pretty='%H'` --format='%H' | wc -l)
MAGMA_COMMIT                 := $(shell git log --format="%H" -n 1 | cut -c1-7)
endif

MAGMA_TIMESTAMP               = $(shell date +'%Y%m%d.%H%M')

ifeq ($(VERBOSE),yes)
RUN                           =
else
RUN                           = @
endif

# So we can tell the user what happened
ifdef MAKECMDGOALS
TARGETGOAL                   += $(MAKECMDGOALS)
else
TARGETGOAL                   = $(.DEFAULT_GOAL)
endif

ifeq ($(OS),Windows_NT)
  HOSTTYPE                   := "Windows"
  LIBPREFIX                  :=
  DYNLIBEXT                  := ".dll"
  STATLIBEXT                 := ".lib"
  EXEEXT                     := ".exe"

  # Alias the target names on Windows to the equivalent without the exe extension.
  $(basename $(DIME_PROGRAM)) := $(DIME_PROGRAM)
  $(basename $(MAGMA_PROGRAM)) := $(MAGMA_PROGRAM)
  $(basename $(SIGNET_PROGRAM)) := $(SIGNET_PROGRAM)
  $(basename $(GENREC_PROGRAM)) := $(GENREC_PROGRAM)
  $(basename $(DIME_CHECK_PROGRAM)) := $(DIME_CHECK_PROGRAM)
  $(basename $(MAGMA_CHECK_PROGRAM)) := $(MAGMA_CHECK_PROGRAM)
else
  HOSTTYPE                 := $(shell uname -s)
  LIBPREFIX                := lib
  DYNLIBEXT                := .so
  STATLIBEXT               := .a
  EXEEXT                   :=
endif

all: config warning \
	$(MAGMA_PROGRAM) \
	$(DIME_PROGRAM) \
	$(SIGNET_PROGRAM) \
	$(GENREC_PROGRAM) \
	$(DIME_CHECK_PROGRAM) \
	$(MAGMA_CHECK_PROGRAM) \
	finished

strip: config warning \
	$(MAGMA_PROGRAM_STRIPPED) \
	$(MAGMA_CHECK_PROGRAM_STRIPPED) \
	$(MAGMA_SHARED_LIBRARY_STRIPPED) \
	finished

stripped: strip

warning:
ifeq ($(VERBOSE),no)
	@echo
	@echo 'For a more verbose output'
	@echo '  make '$(GREEN)'VERBOSE=yes' $(NORMAL)$(TARGETGOAL)
	@echo
endif

config:
	@echo
	@echo 'TARGET' $(TARGETGOAL)
	@echo 'VERBOSE' $(VERBOSE)
	@echo
	@echo 'VERSION ' $(MAGMA_VERSION)
	@echo 'COMMIT ' $(MAGMA_COMMIT)
	@echo 'DATE ' $(MAGMA_TIMESTAMP)
	@echo 'HOST ' $(HOSTTYPE)

setup:
ifeq ($(VERBOSE),no)
	@echo 'Running the '$(YELLOW)'setup'$(NORMAL)' scripts.'
	@dev/scripts/linkup.sh &> /dev/null
else
	@dev/scripts/linkup.sh
endif

keys: $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Generating new '$(YELLOW)'key'$(NORMAL)' files.'
	@dev/scripts/builders/build.lib.sh generate &> /dev/null
else
	@dev/scripts/builders/build.lib.sh generate
endif

docs:
ifeq ($(VERBOSE),no)
	@echo 'Building the '$(YELLOW)'documentation'$(NORMAL)' files.'
	$(RUN)dev/scripts/builders/build.docs.sh &> /dev/null
else
	@dev/scripts/builders/build.docs.sh
endif

check: config  \
	warning \
	$(MAGMA_CHECK_PROGRAM) \
	$(DIME_CHECK_PROGRAM) \
	finished
	$(RUN)$(TOPDIR)/$(MAGMA_CHECK_PROGRAM) sandbox/etc/magma.sandbox.config
	$(RUN)$(TOPDIR)/$(DIME_CHECK_PROGRAM)

gprof: $(MAGMA_PROGRAM_GPROF) $(MAGMA_CHECK_PROGRAM_GPROF)

pprof: $(MAGMA_PROGRAM_PPROF) $(MAGMA_CHECK_PROGRAM_PPROF)

# If verbose mode is disabled, we only output this finished message.
finished:
ifeq ($(VERBOSE),no)
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)
endif

#incremental: $(MAGMA_INCREMENTAL_BUILD)

# Delete the compiled program along with the generated object and dependency files
clean:
	@$(RM) $(MAGMA_PROGRAM) $(DIME_PROGRAM) $(SIGNET_PROGRAM) $(GENREC_PROGRAM) $(MAGMA_CHECK_PROGRAM) $(DIME_CHECK_PROGRAM)
	@$(RM) $(MAGMA_PROGRAM_PPROF) $(MAGMA_CHECK_PROGRAM_PPROF) $(MAGMA_PROGRAM_GPROF) $(MAGMA_CHECK_PROGRAM_GPROF)
	@$(RM) $(MAGMA_PROGRAM_STRIPPED) $(MAGMA_CHECK_PROGRAM_STRIPPED) $(MAGMA_SHARED_LIBRARY_STRIPPED) c
	@$(RM) $(MAGMA_OBJFILES) $(DIME_OBJFILES) $(SIGNET_OBJFILES) $(GENREC_OBJFILES) $(MAGMA_CHECK_OBJFILES) $(DIME_CHECK_OBJFILES) $(MAGMA_PROF_OBJFILES) $(MAGMA_CHECK_PROF_OBJFILES)
	@$(RM) $(MAGMA_DEPFILES) $(DIME_DEPFILES) $(SIGNET_DEPFILES) $(GENREC_DEPFILES) $(MAGMA_CHECK_DEPFILES) $(DIME_CHECK_DEPFILES)
	@for d in $(sort $(dir $(MAGMA_OBJFILES)) $(dir $(MAGMA_CHECK_OBJFILES)) $(dir $(DIME_OBJFILES)) $(dir $(SIGNET_OBJFILES)) $(dir $(GENREC_OBJFILES))); \
	  do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@for d in $(sort $(dir $(MAGMA_DEPFILES)) $(dir $(MAGMA_CHECK_DEPFILES)) $(dir $(DIME_DEPFILES)) $(dir $(SIGNET_DEPFILES)) $(dir $(GENREC_DEPFILES))); \
	  do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@$(RM) --force gmon.out callgrind.out callgraph.dot callgraph.dot.ps massif.out.* cachegrind.out.* \
	  magmad.pprof.out magmad.check.pprof.out magmad.gprof.* magmad.check.gprof.*
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

distclean:
	@$(RM) $(MAGMA_PROGRAM) $(DIME_PROGRAM) $(SIGNET_PROGRAM) $(GENREC_PROGRAM) $(MAGMA_CHECK_PROGRAM) $(DIME_CHECK_PROGRAM)
	@$(RM) $(MAGMA_PROGRAM_STRIPPED) $(MAGMA_CHECK_PROGRAM_STRIPPED) $(MAGMA_SHARED_LIBRARY_STRIPPED) $(MAGMA_STATIC_LIBRARY_STRIPPED)
	@$(RM) $(MAGMA_PROGRAM_PPROF) $(MAGMA_CHECK_PROGRAM_PPROF) $(MAGMA_PROGRAM_GPROF) $(MAGMA_CHECK_PROGRAM_GPROF)
	@$(RM) $(MAGMA_OBJFILES) $(DIME_OBJFILES) $(SIGNET_OBJFILES) $(GENREC_OBJFILES) $(MAGMA_CHECK_OBJFILES) $(DIME_CHECK_OBJFILES) $(MAGMA_PROF_OBJFILES) $(MAGMA_CHECK_PROF_OBJFILES)
	@$(RM) $(MAGMA_DEPFILES) $(DIME_DEPFILES) $(SIGNET_DEPFILES) $(GENREC_DEPFILES) $(MAGMA_CHECK_DEPFILES) $(DIME_CHECK_DEPFILES)
	@$(RM) --force --recursive lib/local lib/logs lib/objects lib/sources dev/docs/html dev/docs/latex $(DEPDIR) $(OBJDIR)
	@$(RM) --force gmon.out callgrind.out callgraph.dot callgraph.dot.ps massif.out.* cachegrind.out.* \
	  magmad.pprof.out magmad.check.pprof.out magmad.gprof.* magmad.check.gprof.*
	@$(RM) $(MAGMA_SHARED_LIBRARY) $(MAGMA_STATIC_LIBRARY)
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

%.stripped $(addsuffix $(DYNLIBEXT), %.stripped): %
ifeq ($(VERBOSE),no)
	@echo 'Creating' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(STRIP) $(STRIPFLAGS) --output-format=$(shell objdump -p "$(subst .stripped,,$@)" | grep "file format" | head -1 | \
	awk -F'file format' '{print $$2}' | tr --delete [:space:]) -o "$@" "$(subst .stripped,,$@)"

install: $(MAGMA_PROGRAM) $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Installing' $(GREEN)$(MAGMA_PROGRAM)$(NORMAL)
endif
	$(RUN)$(INSTALL) --mode=0755 --owner=root --group=root --context=system_u:object_r:bin_t:s0 --no-target-directory \
	  $(MAGMA_PROGRAM) /usr/libexec/$(MAGMA_PROGRAM)
	$(RUN)$(INSTALL) --mode=0755 --owner=root --group=root --context=system_u:object_r:bin_t:s0 --no-target-directory \
	  $(MAGMA_SHARED_LIBRARY) /usr/libexec/$(MAGMA_SHARED_LIBRARY)

# Construct the magma daemon executable.
$(MAGMA_PROGRAM): $(MAGMA_SHARED_LIBRARY) $(MAGMA_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(MAGMA_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) -Wl,--end-group

# Construct the magma unit test executable.
$(MAGMA_CHECK_PROGRAM): $(MAGMA_SHARED_LIBRARY) $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) $(MAGMA_CHECK_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) -Wl,--start-group,--whole-archive $(MAGMA_CHECK_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_CHECK_DYNAMIC)

# Construct the magma daemon executable with pprof support.
$(MAGMA_PROGRAM_PPROF): $(MAGMA_SHARED_LIBRARY) $(MAGMA_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(MAGMA_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(PPROF) $(MAGMA_STATIC) -Wl,--end-group

# Construct the magma unit test executable with pprof support.
$(MAGMA_CHECK_PROGRAM_PPROF): $(MAGMA_SHARED_LIBRARY) $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES))
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) -Wl,--start-group,--whole-archive $(MAGMA_CHECK_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_CHECK_DYNAMIC) $(PPROF)

# Construct the magma daemon executable with gprof support.
$(MAGMA_PROGRAM_GPROF): $(MAGMA_SHARED_LIBRARY) $(MAGMA_PROF_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) $(GPROF) -o '$@' $(MAGMA_PROF_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) -Wl,--end-group

# Construct the magma unit test executablew with gprof support.
$(MAGMA_CHECK_PROGRAM_GPROF): $(MAGMA_SHARED_LIBRARY) $(MAGMA_CHECK_PROF_OBJFILES) $(filter-out .objs/src/magma.pg.o, $(MAGMA_PROF_OBJFILES))
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) $(GPROF) -o '$@' $(MAGMA_CHECK_PROF_OBJFILES) $(filter-out .objs/src/magma.pg.o, $(MAGMA_PROF_OBJFILES)) -Wl,--start-group,--whole-archive $(MAGMA_CHECK_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_CHECK_DYNAMIC)

# Construct the dime command line utility.
$(DIME_PROGRAM): $(MAGMA_STATIC) $(DIME_OBJFILES) $(DIME_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(DIME_OBJFILES) -Wl,--start-group,--whole-archive $(MAGMA_STATIC) $(DIME_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_DYNAMIC)

# Construct the signet command line utility.
$(SIGNET_PROGRAM): $(MAGMA_STATIC) $(SIGNET_OBJFILES) $(SIGNET_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(SIGNET_OBJFILES) -Wl,--start-group,--whole-archive $(MAGMA_STATIC) $(SIGNET_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_DYNAMIC)

# Construct the dime command line utility.
$(GENREC_PROGRAM): $(MAGMA_STATIC) $(GENREC_OBJFILES) $(GENREC_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(GENREC_OBJFILES) -Wl,--start-group,--whole-archive $(MAGMA_STATIC) $(GENREC_STATIC) -Wl,--no-whole-archive,--end-group $(MAGMA_DYNAMIC)

# Construct the dime unit test executable.
$(DIME_CHECK_PROGRAM): $(DIME_CHECK_OBJFILES) $(DIME_CHECK_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS_COMBINED) -o '$@' $(DIME_CHECK_OBJFILES) -Wl,--start-group,--whole-archive $(DIME_CHECK_STATIC) -Wl,--no-whole-archive,--end-group $(DIME_CHECK_DYNAMIC)

$(OBJDIR)/check/dime/%.o: check/dime/%.cpp $(MAGMA_SHARED_LIBRARY) $(DIME_CHECK_STATIC)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CPP) -o '$@' $(CPPFLAGS_COMBINED) $(CPPDEFINES) $(CPPFLAGS.$(<F)) $(CPPDEFINES.$(<F)) $(DIME_CHECK_CPPINCLUDES) -MF"$(<:%.cpp=$(DEPDIR)/%.d)" -MD -MP -MT"$@" -c "$<"

# The Magma Unit Test Object Files
$(OBJDIR)/check/magma/%.o: check/magma/%.c $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) -o '$@' $(CDEFINES) $(CFLAGS_COMBINED) $(CFLAGS.$(<F)) $(CDEFINES.$(<F)) $(MAGMA_CHECK_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" "$<"

# The Magma Daemon Object Files
$(OBJDIR)/%.o: %.c $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) -o '$@' $(CDEFINES) $(CFLAGS_COMBINED) $(CFLAGS.$(<F)) $(CDEFINES.$(<F)) $(MAGMA_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" "$<"

# The Magma Unit Test Object Files (GProf Version)
$(OBJDIR)/check/magma/%.pg.o: check/magma/%.c $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) -o '$@' $(GPROF) $(CDEFINES) $(CFLAGS_COMBINED) $(CFLAGS.$(<F)) $(CDEFINES.$(<F)) $(MAGMA_CHECK_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" "$<"

# The Magma Daemon Object Files (GProf Version)
$(OBJDIR)/%.pg.o: %.c $(MAGMA_SHARED_LIBRARY)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) -o '$@' $(GPROF) $(CDEFINES) $(CFLAGS_COMBINED) $(CFLAGS.$(<F)) $(CDEFINES.$(<F)) $(MAGMA_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" "$<"	
	
## The recipes needed to build the various statically linked dependencies. They do not actually depend on the Magma shared library,
## but include the dependency here to keep make from trying to build both at the same time.
#$(TOPDIR)/lib/local/lib/libz$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY)
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)libz$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh zlib &> /dev/null
#else
#	@echo 'Building' $(YELLOW)libz$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh zlib
#endif
#
##$(TOPDIR)/lib/local/lib/libssl$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY) | $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT) $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT) 
#
#$(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY) | $(TOPDIR)/lib/local/lib/libz$(STATLIBEXT)
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)libssl$(STATLIBEXT) libcrypto$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh openssl &> /dev/null
#else
#	@echo 'Building' $(YELLOW)libssl$(STATLIBEXT) libcrypto$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh openssl
#endif
#
#$(TOPDIR)/lib/local/lib/libutf8proc$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY) | $(TOPDIR)/lib/local/lib/libcrypto$(STATLIBEXT)
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)libutf8proc$(STATLIBEXT)$(NORMAL)
#	$(shell [ "`which curl &> /dev/null; echo $$?`" != 0 ] && QUICK=yes dev/scripts/builders/build.lib.sh curl &> /dev/null)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh utf8proc &> /dev/null
#else
#	@echo 'Building' $(YELLOW)libutf8proc$(STATLIBEXT)$(NORMAL)
#	$(shell [ "`which curl &> /dev/null; echo $$?`" != 0 ] && QUICK=yes dev/scripts/builders/build.lib.sh curl)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh utf8proc
#endif
#
#$(TOPDIR)/lib/sources/googtest/lib/.libs/libgtest$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY)
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)googtest$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh googtest &> /dev/null
#else
#	@echo 'Building' $(YELLOW)googtest$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh googtest
#endif
#
#$(TOPDIR)/lib/local/lib/libcheck$(STATLIBEXT): $(MAGMA_SHARED_LIBRARY)
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)libcheck$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh checker &> /dev/null
#else
#	@echo 'Building' $(YELLOW)libcheck$(STATLIBEXT)$(NORMAL)
#	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh checker
#endif

$(DIME_STATIC): $(MAGMA_SHARED_LIBRARY)

$(SIGNET_STATIC): $(MAGMA_SHARED_LIBRARY)

$(GENREC_STATIC): $(MAGMA_SHARED_LIBRARY)

$(DIME_CHECK_STATIC): $(MAGMA_SHARED_LIBRARY)

$(MAGMA_CHECK_STATIC): $(MAGMA_SHARED_LIBRARY)

# The recipe for creating a dynamically loaded shared library with all external dependencies required by Magma.
$(MAGMA_SHARED_LIBRARY): dev/scripts/builders/build.lib.params.sh
	@echo
	@echo 'Building the '$(YELLOW)'bundled'$(NORMAL)' dependencies.'
	@QUICK=$(QUICK) dev/scripts/builders/build.lib.sh all

# If we've already generated dependency files, use them to see if a rebuild is required
-include $(MAGMA_DEPFILES) $(DIME_DEPFILES) $(SIGNET_DEPFILES) $(GENREC_DEPFILES) $(MAGMA_CHECK_DEPFILES) $(DIME_CHECK_DEPFILES)

# Special Make Directives
.SUFFIXES: .c .cc .cpp .o
.PHONY: all strip stripped warning config finished check setup docs clean distclean install pprof gprof

# vim:set softtabstop=2 shiftwidth=2 tabstop=2
