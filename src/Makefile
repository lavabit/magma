#!/usr/bin/make
#
# The Magma Makefile
#
#########################################################################

TOPDIR					= $(realpath ..)

ARCHIVE					= magmad
MAGMA_VERSION			= "6.0.2"
MAGMA_COMMIT			= `git log --format="%H" -n 1 | cut -c33-40`
MAGMA_TIMESTAMP			= `date +'%Y%m%d.%H%M'`



SRCDIR					= $(TOPDIR)/src
BUNDLEDINCDIR			= $(TOPDIR)/lib/sources

INCDIRS					= spf2/src/include clamav/libclamav mysql/include openssl/include lzo/include xml2/include \
		zlib bzip2 tokyocabinet memcached dkim/libopendkim dspam/src jansson/src gd png jpeg freetype/include

SRCDIRS 				= core core/classify core/host core/buckets core/log core/strings core/thread core/memory core/hash \
		core/compare core/indexes core/encodings core/parsers core/parsers/numbers core/parsers/formats core/parsers/special \
		engine engine/controller engine/status engine/config engine/config/servers engine/config/relay engine/config/global \
		engine/config/cache engine/context network objects objects/mail objects/contacts objects/sessions objects/neue \
		objects/folders objects/users objects/warehouse objects/config objects/messages providers providers/consumers \
		providers/checkers providers/images providers/cryptography providers/database providers/storage providers/compress \
		providers/parsers servers servers/http servers/smtp servers/molten servers/dmtp servers/imap servers/pop web \
		web/contact web/json_api web/portal web/statistics web/teacher web/register
SRCFILES				= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

# Compiler Parameters
CC						= gcc

CFLAGS					= -std=gnu99 -O0 -fPIC -fmessage-length=0 -ggdb3 -rdynamic -c -Wall -Werror -MMD 
CFLAGS_PEDANTIC			= -Wextra -Wpacked -Wunreachable-code -Wformat=2

CINCLUDES				= -I$(SRCDIR) 
CINCLUDES				+= $(addprefix -I,$(INCLUDE_DIR_ABSPATHS))

CDEFINES				= -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -DHAVE_NS_TYPE -DFORTIFY_SOURCE=2
CDEFINES				+= -DMAGMA_PEDANTIC -DMAGMA_VERSION="\"$(MAGMA_VERSION)\"" -DMAGMA_COMMIT="\"$(MAGMA_COMMIT)\"" \
		-DMAGMA_TIMESTAMP="\"$(MAGMA_TIMESTAMP)\""

# Stack protection is currently untested. The options have been included here so they can be enabled in the future.
# Stack smash protection adds guard variables to function calls, which are checked when the function returns. If the guard
# variable is smashed then magmad will abruptly exit with an error message.
CFLAGS_STACKSMASH		= -fstack-protector-all -Wstack-protector

# Archiver Parameters
AR						= ar
ARFLAGS					= rcs

# Hidden Directory for Dependency Files
DEPDIR					= .deps
DEPFILES				= $(patsubst %.c,$(DEPDIR)/%.d,$(SRCFILES))

# Hidden Directory for Object Files
OBJDIR					= .objs
OBJFILES				= $(patsubst %.c,$(OBJDIR)/%.o,$(SRCFILES))

# Resolve the External Include Directory Paths
INCLUDE_DIR_VPATH		= $(BUNDLEDINCDIR) /usr/include /usr/local/include
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
endif

# So we can tell the user what happened
ifdef MAKECMDGOALS
TARGETGOAL		= $(MAKECMDGOALS)
else
TARGETGOAL		= $(.DEFAULT_GOAL)
endif

# Shortcuts
.PHONY: all clean libs $(basename $(ARCHIVE))
all: $(ARCHIVE)
libs: $(ARCHIVE)
#Need this for after we add the exe suffix
#$(basename $(ARCHIVE)): $(ARCHIVE)

# Delete the archive along with the generated object and dependency files
clean:
	@$(RM) $(ARCHIVE) $(OBJFILES) $(DEPFILES)
	@for d in $(sort $(dir $(OBJFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@for d in $(sort $(dir $(DEPFILES))); do if test -d "$$d"; then $(RMDIR) "$$d"; fi; done
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

# Construct the static archive file
$(ARCHIVE): $(OBJFILES)
	@echo 'Constructing' $(RED)$@$(NORMAL)
	@$(AR) $(ARFLAGS) $@ $(OBJFILES)
	@$(RANLIB) $@
	@echo 'Finished' $(BOLD)$(GREEN)$(TARGETGOAL)$(NORMAL)

# Object files
$(OBJDIR)/%.o: %.c
	@echo 'Building' $(YELLOW)$<$(NORMAL)
	@test -d $(DEPDIR)/$(dir $<) || $(MKDIR) $(DEPDIR)/$(dir $<)
	@test -d $(OBJDIR)/$(dir $<) || $(MKDIR) $(OBJDIR)/$(dir $<)
	$(CC) $(CFLAGS) $(CDEFINES) $(CINCLUDES) -MF"$(<:%.c=$(DEPDIR)/%.d)" -MT"$@" -o"$@" "$<"

# If we've already generated dependency files, use them to see if a rebuild is required
-include $(DEPFILES)

