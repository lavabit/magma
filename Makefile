#!/usr/bin/make
#
# The Magma Makefile
#
#########################################################################

TOPDIR					= $(realpath .)
MFLAGS					=
MAKEFLAGS				= --output-sync=target --jobs=6

# Identity of this package.
PACKAGE_NAME			= Magma Daemon
PACKAGE_TARNAME			= magma
PACKAGE_VERSION			= 6.2
PACKAGE_STRING			= $(PACKAGE_NAME) $(PACKAGE_VERSION)
PACKAGE_BUGREPORT		= support@lavabit.com
PACKAGE_URL				= https://lavabit.com

ifeq ($(OS),Windows_NT)
    HOSTTYPE 			:= "Windows"
    LIBPREFIX			:= 
    DYNLIBEXT			:= ".dll"
    STATLIBEXT			:= ".lib"
    EXEEXT 				:= ".exe"
else
    HOSTTYPE			:= $(shell uname -s)
    LIBPREFIX			:= "lib"
    DYNLIBEXT			:= ".so"
    STATLIBEXT			:= ".a"
    EXEEXT				:= 
endif

MAGMA_PROGRAM			= $(addsuffix $(EXEEXT), magmad)
CHECK_PROGRAM			= $(addsuffix $(EXEEXT), magmad.check)

MAGMA_REPO				= $(shell which git &> /dev/null && git log &> /dev/null && echo 1) 
ifneq ($(strip $(MAGMA_REPO)),1)
	MAGMA_VERSION			:= $(PACKAGE_VERSION)
	MAGMA_COMMIT			:= "NONE"
else
	MAGMA_VERSION			:= $(PACKAGE_VERSION).$(shell git log --format='%H' | wc -l)
	MAGMA_COMMIT			:= $(shell git log --format="%H" -n 1 | cut -c33-40)
endif
MAGMA_TIMESTAMP			= $(shell date +'%Y%m%d.%H%M')

# Source Files
BUILD_SRCFILES			= src/engine/status/build.c

MAGMA_SRCDIRS			= $(shell find src -type d -print)
MAGMA_SRCFILES			= $(filter-out src/engine/status/build.c, $(foreach dir,$(MAGMA_SRCDIRS), $(wildcard $(dir)/*.c)))

CHECK_SRCDIRS			= $(shell find check -type d -print)
CHECK_SRCFILES			= $(foreach dir,$(CHECK_SRCDIRS), $(wildcard $(dir)/*.c))

# Bundled Dependency Include Paths
INCDIR					= $(TOPDIR)/lib/sources
INCDIRS					= spf2/src/include clamav/libclamav mysql/include openssl/include lzo/include xml2/include \
		zlib bzip2 tokyocabinet memcached dkim/libopendkim dspam/src jansson/src gd png jpeg freetype/include utf8proc

# Compiler Parameters
CC						= gcc

CFLAGS					= -std=gnu99 -O0 -fPIC -fmessage-length=0 -ggdb3 -rdynamic -c $(CFLAGS_WARNINGS) -MMD 
CFLAGS_WARNINGS			= -Wall -Werror -Winline -Wformat-security -Warray-bounds
CFLAGS_PEDANTIC			= -Wextra -Wpacked -Wunreachable-code -Wformat=2

CINCLUDES				= $(addprefix -I,$(INCLUDE_DIR_ABSPATHS))
MAGMA_CINCLUDES			= -Isrc $(CINCLUDES)
CHECK_CINCLUDES			= -Icheck $(MAGMA_CINCLUDES)

CDEFINES				= -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -DHAVE_NS_TYPE -DFORTIFY_SOURCE=2 -DMAGMA_PEDANTIC 
CDEFINES.build.c 		= \
		-DMAGMA_VERSION=\"$(MAGMA_VERSION)\" \
		-DMAGMA_COMMIT=\"$(MAGMA_COMMIT)\" \
		-DMAGMA_TIMESTAMP=\"$(MAGMA_TIMESTAMP)\"

# Linker Parameters
LD						= gcc
LDFLAGS					= -rdynamic

MAGMA_DYNAMIC			= -lrt -ldl -lpthread
CHECK_DYNAMIC			= $(MAGMA_DYNAMIC) -lcheck

MAGMA_STATIC			= 
CHECK_STATIC			= 

# Archiver Parameters
AR						= ar
ARFLAGS					= rcs

# Hidden Directory for Dependency Files
DEPDIR					= .deps
BUILD_DEPFILES			= $(patsubst %.c,$(OBJDIR)/%.d,$(BUILD_SRCFILES))
MAGMA_DEPFILES			= $(patsubst %.c,$(DEPDIR)/%.d,$(MAGMA_SRCFILES))
CHECK_DEPFILES			= $(patsubst %.c,$(DEPDIR)/%.d,$(CHECK_SRCFILES))

# Hidden Directory for Object Files
OBJDIR					= .objs
BUILD_OBJFILES			= $(patsubst %.c,$(OBJDIR)/%.o,$(BUILD_SRCFILES))
MAGMA_OBJFILES			= $(patsubst %.c,$(OBJDIR)/%.o,$(MAGMA_SRCFILES))
CHECK_OBJFILES			= $(patsubst %.c,$(OBJDIR)/%.o,$(CHECK_SRCFILES))

# Modified Object Files
#MODIFIED_SRCFILES		= $(shell git ls-files --modified --others src)
#MODIFIED_SRCFILES		= $(shell find src -mmin -1)
#MODIFIED_OBJFILES		= $(patsubst %.c,$(OBJDIR)/%.o,$(MODIFIED_SRCFILES))

# Resolve the External Include Directory Paths
INCLUDE_DIR_VPATH		= $(INCDIR) /usr/include /usr/local/include
INCLUDE_DIR_SEARCH 		= $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(INCLUDE_DIR_VPATH)))))
INCLUDE_DIR_ABSPATHS	+= $(foreach target,$(INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))

# Other External programs
MV						= mv --force
RM						= rm --force
RMDIR					= rmdir --parents --ignore-fail-on-non-empty
MKDIR					= mkdir --parents
RANLIB					= ranlib

# Text Coloring
RED						= $$(tput setaf 1)
BLUE					= $$(tput setaf 4)
GREEN					= $$(tput setaf 2)
WHITE					= $$(tput setaf 7)
YELLOW					= $$(tput setaf 3)

# Text Weighting
BOLD					= $$(tput bold)
NORMAL					= $$(tput sgr0)

ifeq ($(VERBOSE),yes)
RUN						=
else
RUN						= @
VERBOSE					= no
endif

# So we can tell the user what happened
ifdef MAKECMDGOALS
TARGETGOAL				+= $(MAKECMDGOALS)
else
TARGETGOAL				= $(.DEFAULT_GOAL)
endif

# Special Make Directives
#.NOTPARALLEL: warning conifg

.PHONY: warning config finished all check incremental
all: config warning $(MAGMA_PROGRAM) $(CHECK_PROGRAM) finished

check: config warning $(CHECK_PROGRAM)

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
	
# Alias the target names on Windows to the equivalent without the exe extension.
ifeq ($(HOSTTYPE),Windows)

.PHONY: $(basename $(MAGMA_PROGRAM(OBJDIR)/src/%.o: src/%.cM))
$(basename $(MAGMA_PROGRAM)): $(MAGMA_PROGRAM)

.PHONY: $(basename $(CHECK_PROGRAM))
$(basename $(CHECK_PROGRAM)): $(CHECK_PROGRAM)

endif

# Delete the compiled program along with the generated object and dependency files
clean:
	@$(RM) $(MAGMA_PROGRAM) $(CHECK_PROGRAM) $(MAGMA_OBJFILES) $(CHECK_OBJFILES) $(MAGMA_DEPFILES) $(CHECK_DEPFILES)
	@for d in $(sort $(dir $(MAGMA_OBJFILES)) $(dir $(CHECK_OBJFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@for d in $(sort $(dir $(MAGMA_DEPFILES)) $(dir $(CHECK_DEPFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

incremental: $(MODIFIED_OBJFILES)

# The Build Information
$(BUILD_OBJFILES): $(BUILD_SRCFILES) $(MAGMA_SRCFILES) $(CHECK_SRCFILES)

# Construct the magma daemon executable file
$(MAGMA_PROGRAM): $(MAGMA_OBJFILES) $(BUILD_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
else
	@echo 
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(MAGMA_OBJFILES) $(BUILD_OBJFILES) -Wl,--start-group $(MAGMA_STATIC) -Wl,--end-group $(MAGMA_DYNAMIC)

# Construct the magma unit test executable
$(CHECK_PROGRAM): $(CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) $(BUILD_OBJFILES)
ifeq ($(VERBOSE),no)
	@echo 'Constructing' $(RED)$@$(NORMAL)
endif
	$(RUN)$(LD) $(LDFLAGS) --output='$@' $(CHECK_OBJFILES) $(filter-out .objs/src/magma.o, $(MAGMA_OBJFILES)) $(BUILD_OBJFILES) -Wl,--start-group $(CHECK_STATIC) -Wl,--end-group $(CHECK_DYNAMIC)

# The Magma Daemon Object files
$(OBJDIR)/src/%.o: src/%.c
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) $(CFLAGS) $(CFLAGS.$(<F)) $(CDEFINES) $(CDEFINES.$(<F)) $(MAGMA_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

# The Magma Unit Test Object files
$(OBJDIR)/check/%.o: check/%.c
ifeq ($(VERBOSE),no)
	@echo 'Building' $(YELLOW)$<$(NORMAL)
endif
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) $(CFLAGS) $(CFLAGS.$(<F)) $(CDEFINES) $(CDEFINES.$(<F)) $(CHECK_CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

# If we've already generated dependency files, use them to see if a rebuild is required
-include $(MAGMA_DEPFILES) $(CHECK_DEPFILES) $(BUILD_DEPFILES)

# vim:set softtabstop=4 shiftwidth=4 tabstop=4:
