#!/usr/bin/make
#
# The Magma Makefile
#
#########################################################################

TOPDIR					= $(realpath ..)

PROGRAM					= magmad
MAGMA_VERSION			= "6.0.2"
MAGMA_COMMIT			= $(shell git log --format="%H" -n 1 | cut -c33-40)
MAGMA_TIMESTAMP			= $(shell date +'%Y%m%d.%H%M')

# Source Files
SRCDIR					= $(TOPDIR)/src
SRCDIRS					= $(shell find * -type d -print)
SRCFILES				= $(foreach dir,$(SRCDIRS), $(wildcard $(dir)/*.c))

# Bundled Dependency Include Paths
INCDIR					= $(TOPDIR)/lib/sources
INCDIRS					= spf2/src/include clamav/libclamav mysql/include openssl/include lzo/include xml2/include \
		zlib bzip2 tokyocabinet memcached dkim/libopendkim dspam/src jansson/src gd png jpeg freetype/include

# Compiler Parameters
CC						= gcc

CFLAGS					= -std=gnu99 -O0 -fPIC -fmessage-length=0 -ggdb3 -rdynamic -c -Wall -Werror -MMD 
CFLAGS_PEDANTIC			= -Wextra -Wpacked -Wunreachable-code -Wformat=2

CINCLUDES				= -I$(SRCDIR) 
CINCLUDES				+= $(addprefix -I,$(INCLUDE_DIR_ABSPATHS))

CDEFINES				= -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -DHAVE_NS_TYPE -DFORTIFY_SOURCE=2
CDEFINES				+= -DMAGMA_PEDANTIC 

# File Specific Parameters 
CDEFINES.engine/status/build.c 	= \
		-DMAGMA_VERSION="\"$(MAGMA_VERSION)\"" \
		-DMAGMA_COMMIT="\"$(MAGMA_COMMIT)\"" \
		-DMAGMA_TIMESTAMP="\"$(MAGMA_TIMESTAMP)\""

# Archiver Parameters
AR						= ar
ARFLAGS					= rcs

# Linker Parameters
LD						:= gcc
LDFLAGS					:= -rdynamic

# Hidden Directory for Dependency Files
DEPDIR					= .deps
DEPFILES				= $(patsubst %.c,$(DEPDIR)/%.d,$(SRCFILES))

# Hidden Directory for Object Files
OBJDIR					= .objs
OBJFILES				= $(patsubst %.c,$(OBJDIR)/%.o,$(SRCFILES))

# Resolve the External Include Directory Paths
INCLUDE_DIR_VPATH		= $(INCDIR) /usr/include /usr/local/include
INCLUDE_DIR_SEARCH 		= $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(INCLUDE_DIR_VPATH)))))
INCLUDE_DIR_ABSPATHS 	+= $(foreach target,$(INCDIRS), $(call INCLUDE_DIR_SEARCH,$(target)))

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
TARGETGOAL		= $(MAKECMDGOALS)
else
TARGETGOAL		= $(.DEFAULT_GOAL)
endif

# Shortcuts
.PHONY: all clean $(basename $(PROGRAM))
all: config warning $(PROGRAM)
	

#Need this for after we add the exe suffix
#$(basename $(ARCHIVE)): $(ARCHIVE)

warning:
	@echo 
	@echo 'For verbose output' 
	@echo '  ' $(BLUE)make $(GREEN)VERBOSE=yes $(BLUE)all$(NORMAL)
	@echo 

config:
	@echo 
	@echo 'TARGET' $(TARGETGOAL)
	@echo 'VERBOSE' $(VERBOSE)
	@echo 
	@echo 'VERSION ' $(MAGMA_VERSION)
	@echo 'COMMIT '$(MAGMA_COMMIT)
	@echo 'DATE ' $(MAGMA_TIMESTAMP)
	@echo 
	
# Delete the compiled program along with the generated object and dependency files
clean:
	@$(RM) $(PROGRAM) $(OBJFILES) $(DEPFILES)
	@for d in $(sort $(dir $(OBJFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@for d in $(sort $(dir $(DEPFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

# Construct the static archive file
$(PROGRAM): $(OBJFILES)
	@echo 'Constructing' $(RED)$@$(NORMAL)
	$(RUN)$(AR) $(ARFLAGS) $@ $(OBJFILES)
	$(RUN)$(RANLIB) $@
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

# Object files
$(OBJDIR)/%.o: %.c
	@echo 'Building' $(YELLOW)$<$(NORMAL)
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(RUN)$(CC) $(CFLAGS) $(CFLAGS.$<) $(CDEFINES) $(CDEFINES.$<) $(CINCLUDES) $(CINCLUDES.$<) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

# If we've already generated dependency files, use them to see if a rebuild is required
-include $(DEPFILES)

