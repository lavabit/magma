#!/usr/bin/make
#
# The Magma Makefile
#
#########################################################################

TOPDIR							= $(realpath .)
MFLAGS							=
MAKEFLAGS						= --output-sync=target --jobs=6

# Identity of this package.
PACKAGE_NAME					= Magma Daemon
PACKAGE_TARNAME					= magma
PACKAGE_VERSION					= 6.2
PACKAGE_STRING					= $(PACKAGE_NAME) $(PACKAGE_VERSION)
PACKAGE_BUGREPORT				= support@lavabit.com
PACKAGE_URL						= https://lavabit.com

MAGMA_PROGRAM					= $(addsuffix $(EXEEXT), magmad)
MAGMA_CHECK_PROGRAM				= $(addsuffix $(EXEEXT), magmad.check)

DIME_PROGRAM					= $(addsuffix $(EXEEXT), dime)
SIGNET_PROGRAM					= $(addsuffix $(EXEEXT), signet)
GENREC_PROGRAM					= $(addsuffix $(EXEEXT), genrec)
DIME_CHECK_PROGRAM				= $(addsuffix $(EXEEXT), dime.check)

# Source Files
BUILD_SRCFILES					= src/engine/status/build.c

MAGMA_STATIC					= #$(TOPDIR)/lib/local/lib/libz.a $(TOPDIR)/lib/local/lib/libcrypto.a $(TOPDIR)/lib/local/lib/libssl.a
MAGMA_DYNAMIC					= -lrt -ldl -lpthread -lresolv
MAGMA_SRCDIRS					= $(shell find src -type d -print)
MAGMA_SRCFILES					= $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(MAGMA_SRCDIRS), $(wildcard $(dir)/*.c)))

MAGMA_CHECK_STATIC				= $(MAGMA_STATIC) $(TOPDIR)/lib/local/lib/libcheck.a
MAGMA_CHECK_DYNAMIC				= $(MAGMA_DYNAMIC) -lm
MAGMA_CHECK_SRCDIRS				= $(shell find check/magma -type d -print)
MAGMA_CHECK_SRCFILES			= $(foreach dir, $(MAGMA_CHECK_SRCDIRS), $(wildcard $(dir)/*.c))

DIME_SRCDIRS					= $(shell  find src/providers/dime tools/dime -type d -print)
DIME_SRCFILES					= $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(DIME_SRCDIRS), $(wildcard $(dir)/*.c)))
DIME_STATIC						= lib/local/lib/libz$(STATLIBEXT) lib/local/lib/libssl$(STATLIBEXT) lib/local/lib/libcrypto$(STATLIBEXT)

SIGNET_SRCDIRS					= $(shell find src/providers/dime tools/signet -type d -print)
SIGNET_SRCFILES					= $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(SIGNET_SRCDIRS), $(wildcard $(dir)/*.c)))
SIGNET_STATIC					= lib/local/lib/libz$(STATLIBEXT) lib/local/lib/libssl$(STATLIBEXT) lib/local/lib/libcrypto$(STATLIBEXT)

GENREC_SRCDIRS					= $(shell find src/providers/dime tools/genrec -type d -print)
GENREC_SRCFILES					= $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(GENREC_SRCDIRS), $(wildcard $(dir)/*.c)))
GENREC_STATIC					= lib/local/lib/libz$(STATLIBEXT) lib/local/lib/libssl$(STATLIBEXT) lib/local/lib/libcrypto$(STATLIBEXT)

DIME_CHECK_STATIC				= $(MAGMA_STATIC) $(TOPDIR)/lib/sources/googtest/lib/.libs/libgtest.a
DIME_CHECK_DYNAMIC				= $(MAGMA_DYNAMIC) -lstdc++
DIME_CHECK_CPPDIRS				= $(shell find check/dime -type d -print)
DIME_CHECK_SRCDIRS				= $(shell find src/providers/dime -type d -print)
DIME_CHECK_CPPFILES				= $(foreach dir, $(DIME_CHECK_CPPDIRS), $(wildcard $(dir)/*.cpp))
DIME_CHECK_SRCFILES				= $(filter-out $(FILTERED_SRCFILES), $(foreach dir, $(DIME_CHECK_SRCDIRS), $(wildcard $(dir)/*.c)))
 
FILTERED_SRCFILES				= src/providers/dime/ed25519/test.c src/providers/dime/ed25519/test-internals.c \
 src/providers/dime/ed25519/fuzz/curve25519-ref10.c src/providers/dime/ed25519/fuzz/ed25519-donna-sse2.c \
 src/providers/dime/ed25519/fuzz/fuzz-curve25519.c src/providers/dime/ed25519/fuzz/ed25519-donna.c \
 src/providers/dime/ed25519/fuzz/ed25519-ref10.c src/providers/dime/ed25519/fuzz/fuzz-ed25519.c
 
PACKAGE_DEPENDENCIES			=  $(TOPDIR)/magmad.so $(MAGMA_STATIC) $(filter-out $(MAGMA_STATIC), $(MAGMA_CHECK_STATIC))

# Bundled Dependency Include Paths
INCDIR							= $(TOPDIR)/lib/sources
MAGMA_INCDIRS					= spf2/src/include clamav/libclamav mysql/include openssl/include lzo/include xml2/include \
		zlib bzip2 tokyocabinet memcached dkim/libopendkim dspam/src jansson/src gd png jpeg freetype/include \
		utf8proc

MAGMA_CHECK_INCDIRS				= 

MAGMA_CINCLUDES					= -Isrc -Isrc/providers $(addprefix -I,$(MAGMA_INCLUDE_ABSPATHS))
DIME_CHECK_CINCLUDES			= $(MAGMA_CINCLUDES)
MAGMA_CHECK_CINCLUDES			= -Icheck/magma -Ilib/local/include/ $(MAGMA_CINCLUDES) $(addprefix -I,$(MAGMA_CHECK_INCLUDE_ABSPATHS))
DIME_CHECK_CPPINCLUDES			= -Icheck/dime -Ilib/sources/googtest/include/ -Ilib/sources/googtest/ -Ilib/sources/googtap/src/ $(MAGMA_CINCLUDES)

CDEFINES						= -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -DHAVE_NS_TYPE -DFORTIFY_SOURCE=2 -DMAGMA_PEDANTIC 
CDEFINES						+= -DDIME_BUILD=\"$(MAGMA_VERSION)\" -DDIME_STAMP=\"$(MAGMA_TIMESTAMP)\"

CPPDEFINES						= $(CDEFINES) -DGTEST_TAP_PRINT_TO_STDOUT -DGTEST_HAS_PTHREAD=1

CDEFINES.build.c 				= -DMAGMA_VERSION=\"$(MAGMA_VERSION)\" -DMAGMA_COMMIT=\"$(MAGMA_COMMIT)\" -DMAGMA_TIMESTAMP=\"$(MAGMA_TIMESTAMP)\"

# Hidden Directory for Dependency Files
DEPDIR							= .deps
DIME_DEPFILES					= $(patsubst %.c,$(DEPDIR)/%.d,$(DIME_SRCFILES))
MAGMA_DEPFILES					= $(patsubst %.c,$(DEPDIR)/%.d,$(MAGMA_SRCFILES))
SIGNET_DEPFILES					= $(patsubst %.c,$(DEPDIR)/%.d,$(SIGNET_SRCFILES))
GENREC_DEPFILES					= $(patsubst %.c,$(DEPDIR)/%.d,$(GENREC_SRCFILES))
MAGMA_CHECK_DEPFILES			= $(patsubst %.c,$(DEPDIR)/%.d,$(MAGMA_CHECK_SRCFILES))
DIME_CHECK_DEPFILES				= $(patsubst %.c,$(DEPDIR)/%.d,$(DIME_CHECK_SRCFILES))
DIME_CHECK_DEPFILES				+= $(patsubst %.cc,$(DEPDIR)/%.d,$(DIME_CHECK_CCFILES))
DIME_CHECK_DEPFILES				+= $(patsubst %.cpp,$(DEPDIR)/%.d,$(DIME_CHECK_CPPFILES))

# Hidden Directory for Object Files
OBJDIR							= .objs
DIME_OBJFILES					= $(patsubst %.c,$(OBJDIR)/%.o,$(DIME_SRCFILES))
MAGMA_OBJFILES					= $(patsubst %.c,$(OBJDIR)/%.o,$(MAGMA_SRCFILES))
SIGNET_OBJFILES					= $(patsubst %.c,$(OBJDIR)/%.o,$(SIGNET_SRCFILES))
GENREC_OBJFILES					= $(patsubst %.c,$(OBJDIR)/%.o,$(GENREC_SRCFILES))
MAGMA_CHECK_OBJFILES			= $(patsubst %.c,$(OBJDIR)/%.o,$(MAGMA_CHECK_SRCFILES))
DIME_CHECK_OBJFILES				= $(patsubst %.c,$(OBJDIR)/%.o,$(DIME_CHECK_SRCFILES))
DIME_CHECK_OBJFILES				+= $(patsubst %.cc,$(OBJDIR)/%.o,$(DIME_CHECK_CCFILES))
DIME_CHECK_OBJFILES				+= $(patsubst %.cpp,$(OBJDIR)/%.o,$(DIME_CHECK_CPPFILES))

# Resolve the External Include Directory Paths
INCLUDE_DIR_VPATH				= $(INCDIR) /usr/include /usr/local/include
INCLUDE_DIR_SEARCH 				= $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(INCLUDE_DIR_VPATH)))))

# Generate the Absolute Directory Paths for Includes
MAGMA_INCLUDE_ABSPATHS			+= $(foreach target,$(MAGMA_INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))
MAGMA_CHECK_INCLUDE_ABSPATHS	+= $(foreach target,$(MAGMA_CHECK_INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))

# Compiler Parameters
CC								= gcc
CFLAGS							= -std=gnu99 -O0 -fPIC -fmessage-length=0 -ggdb3 -rdynamic -c $(CFLAGS_WARNINGS) -MMD 
CFLAGS_WARNINGS					= -Wall -Werror -Winline -Wformat-security -Warray-bounds
CFLAGS_PEDANTIC					= -Wextra -Wpacked -Wunreachable-code -Wformat=2

CPP								= g++
CPPFLAGS						= -std=c++0x $(CPPFLAGS_WARNINGS) -Wno-unused-parameter -pthread -g3 
CPPFLAGS_WARNINGS				= -Wfatal-errors -Werror -Wall -Wextra  -Wformat=2 -Wwrite-strings -Wno-format-nonliteral 

# Linker Parameters
LD								= gcc
LDFLAGS							= -rdynamic

# Archiver Parameters
AR								= ar
ARFLAGS							= rcs

# Other External programs
MV								= mv --force
RM								= rm --force
RMDIR							= rmdir --parents --ignore-fail-on-non-empty
MKDIR							= mkdir --parents
RANLIB							= ranlib

# Text Coloring
RED								= $$(tput setaf 1)
BLUE							= $$(tput setaf 4)
GREEN							= $$(tput setaf 2)
WHITE							= $$(tput setaf 7)
YELLOW							= $$(tput setaf 3)

# Text Weighting
BOLD							= $$(tput bold)
NORMAL							= $$(tput sgr0)

# Calculate the version, commit and timestamp strings.
MAGMA_REPO						= $(shell which git &> /dev/null && git log &> /dev/null && echo 1) 
ifneq ($(strip $(MAGMA_REPO)),1)
	MAGMA_VERSION				:= $(PACKAGE_VERSION)
	MAGMA_COMMIT				:= "NONE"
else
	MAGMA_VERSION				:= $(PACKAGE_VERSION).$(shell git log --format='%H' | wc -l)
	MAGMA_COMMIT				:= $(shell git log --format="%H" -n 1 | cut -c33-40)
endif
setup: $(PACKAGE_DEPENDENCIES)
MAGMA_TIMESTAMP					= $(shell date +'%Y%m%d.%H%M')

ifeq ($(VERBOSE),yes)
RUN								=
else
RUN								= @
VERBOSE							= no
endif

# So we can tell the user what happened
ifdef MAKECMDGOALS
TARGETGOAL						+= $(MAKECMDGOALS)
else
TARGETGOAL						= $(.DEFAULT_GOAL)
endif

ifeq ($(OS),Windows_NT)
    HOSTTYPE 					:= "Windows"
    LIBPREFIX					:= 
    DYNLIBEXT					:= ".dll"
    STATLIBEXT					:= ".lib"
    EXEEXT 						:= ".exe"
    
    # Alias the target names on Windows to the equivalent without the exe extension.
    $(basename $(MAGMA_PROGRAM)) := $(MAGMA_PROGRAM)
	$(basename $(MAGMA_CHECK_PROGRAM)) := $(MAGMA_CHECK_PROGRAM)
else
    HOSTTYPE					:= $(shell uname -s)
    LIBPREFIX					:= "lib"
    DYNLIBEXT					:= ".so"
    STATLIBEXT					:= ".a"
    EXEEXT						:= 
endif

all: config warning setup $(MAGMA_PROGRAM) $(DIME_PROGRAM) $(SIGNET_PROGRAM) $(GENREC_PROGRAM) $(MAGMA_CHECK_PROGRAM) $(DIME_CHECK_PROGRAM) 

setup: $(PACKAGE_DEPENDENCIES)

check: config warning $(MAGMA_CHECK_PROGRAM) $(DIME_CHECK_PROGRAM) finished
	$(RUN)$(TOPDIR)/$(MAGMA_CHECK_PROGRAM) sandbox/etc/magma.sandbox.config
	$(RUN)$(TOPDIR)/$(DIME_CHECK_PROGRAM) sandbox/etc/magma.sandbox.config

setup: $(PACKAGE_DEPENDENCIES)

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
	@echo 'COMMIT '$(MAGMA_COMMIT)
	@echo 'DATE ' $(MAGMA_TIMESTAMP)
	@echo 'HOST ' $(HOSTTYPE)

finished:
ifeq ($(VERBOSE),no)
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)
endif

# Delete the compiled program along with the generated object and dependency files
clean:
	@$(RM) $(MAGMA_PROGRAM) $(DIME_PROGRAM) $(SIGNET_PROGRAM) $(GENREC_PROGRAM) $(MAGMA_CHECK_PROGRAM) $(DIME_CHECK_PROGRAM)
	@$(RM) $(MAGMA_OBJFILES) $(DIME_OBJFILES) $(SIGNET_OBJFILES) $(GENREC_OBJFILES) $(MAGMA_CHECK_OBJFILES) $(DIME_CHECK_OBJFILES)
	@$(RM) $(MAGMA_DEPFILES) $(DIME_DEPFILES) $(SIGNET_DEPFILES) $(GENREC_DEPFILES) $(MAGMA_CHECK_DEPFILES) $(DIME_CHECK_DEPFILES)
	@for d in $(sort $(dir $(MAGMA_OBJFILES)) $(dir $(MAGMA_CHECK_OBJFILES)) $(dir $(DIME_OBJFILES)) $(dir $(SIGNET_OBJFILES)) $(dir $(GENREC_OBJFILES))); \
		do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@for d in $(sort $(dir $(MAGMA_DEPFILES)) $(dir $(MAGMA_CHECK_DEPFILES)) $(dir $(DIME_DEPFILES)) $(dir $(SIGNET_DEPFILES)) $(dir $(GENREC_DEPFILES))); \
		do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

# Construct the magma daemon executable file
$(MAGMA_PROGRAM): $(MAGMA_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo CPPFLAGS
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(MAGMA_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) -Wl,--end-group

# Construct the dime command line utility
$(DIME_PROGRAM): $(DIME_OBJFILES) 
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo 
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(DIME_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) $(DIME_STATIC) -Wl,--end-group

# Construct the signet command line utility
$(SIGNET_PROGRAM): $(SIGNET_OBJFILES) 
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo 
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(SIGNET_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) $(SIGNET_STATIC) -Wl,--end-group

# Construct the dime command line utility
$(GENREC_PROGRAM): $(GENREC_OBJFILES) 
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo 
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(GENREC_OBJFILES) -Wl,--start-group $(MAGMA_DYNAMIC) $(MAGMA_STATIC) $(GENREC_STATIC) -Wl,--end-group

# Construct the magma unit test executable
$(MAGMA_CHECK_PROGRAM): $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES))
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(MAGMA_CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) -Wl,--start-group,--whole-archive $(MAGMA_CHECK_STATIC) -Wl,--no-whole-archive,--end-group  $(MAGMA_CHECK_DYNAMIC) 

# Construct the dime unit test executable
$(DIME_CHECK_PROGRAM): $(DIME_CHECK_OBJFILES) 
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(DIME_CHECK_OBJFILES) -Wl,--start-group,--whole-archive $(DIME_CHECK_STATIC) -Wl,--no-whole-archive,--end-group $(DIME_CHECK_DYNAMIC) 

# The Dime Unit Test Object Files
#.objs/lib/sources/googtest/src/gtest-all.o: lib/sources/googtest/src/gtest-all.cc
#ifeq ($(VERBOSE),no)
#	@echo 'Building' $(YELLOW)$<$(NORMAL)
#endif
#	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
#	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
#	$(RUN)$(CPP) $(CPPFLAGS) $(CPPFLAGS.$(<F)) $(CPPDEFINES) $(CDEFINES.$(<F)) $(DIME_CHECK_CPPINCLUDES) -MF"$(<:%.cc=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

$(OBJDIR)/check/dime/%.o: check/dime/%.cpp 
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CPP) $(CPPFLAGS) $(CPPFLAGS.$(<F)) $(CPPDEFINES) $(CDEFINES.$(<F)) $(DIME_CHECK_CPPINCLUDES) -MF"$(<:%.cpp=$(DEPDIR)/%.d)" -MD -MP -MT"$@" -o"$@" -c "$<"

# The Magma Unit Test Object files
$(OBJDIR)/check/magma/%.o: check/magma/%.c
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) $(CFLAGS) $(CFLAGS.$(<F)) $(CDEFINES) $(CDEFINES.$(<F)) $(MAGMA_CHECK_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

# The Magma Daemon Object files
$(OBJDIR)/%.o: %.c $(PACKAGE_DEPENDENCIES)
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) $(CFLAGS) $(CFLAGS.$(<F)) $(CDEFINES) $(CDEFINES.$(<F)) $(MAGMA_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"
	
$(PACKAGE_DEPENDENCIES): 
ifeq ($(VERBOSE),no)
	@echo 'Running the ' $(YELLOW)'setup'$(NORMAL) ' scripts.'
endif
	$(RUN)dev/scripts/linkup.sh
ifeq ($(VERBOSE),no)
	@echo 'Building the ' $(YELLOW)'bundled'$(NORMAL) ' dependencies.'
endif
	$(RUN)dev/scripts/builders/build.lib.sh all

# If we've already generated dependency files, use them to see if a rebuild is required
-include $(MAGMA_DEPFILES) $(DIME_DEPFILES) $(SIGNET_DEPFILES) $(GENREC_DEPFILES) $(MAGMA_CHECK_DEPFILES) $(DIME_CHECK_DEPFILES)

# Special Make Directives
.SUFFIXES: .c .cc .cpp .o 
# .NOTPARALLEL: warning conifg
.PHONY: all warning config finished check setup

# vim:set softtabstop=4 shiftwidth=4 tabstop=4:
