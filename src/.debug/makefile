################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include web/teacher/subdir.mk
-include web/statistics/subdir.mk
-include web/register/subdir.mk
-include web/portal/subdir.mk
-include web/contact/subdir.mk
-include servers/smtp/subdir.mk
-include servers/pop/subdir.mk
-include servers/molten/subdir.mk
-include servers/imap/subdir.mk
-include servers/http/subdir.mk
-include providers/storage/subdir.mk
-include providers/parsers/subdir.mk
-include providers/images/subdir.mk
-include providers/database/subdir.mk
-include providers/cryptography/subdir.mk
-include providers/consumers/subdir.mk
-include providers/compress/subdir.mk
-include providers/checkers/subdir.mk
-include providers/subdir.mk
-include objects/warehouse/subdir.mk
-include objects/users/subdir.mk
-include objects/sessions/subdir.mk
-include objects/neue/subdir.mk
-include objects/messages/subdir.mk
-include objects/mail/subdir.mk
-include objects/folders/subdir.mk
-include objects/contacts/subdir.mk
-include objects/config/subdir.mk
-include objects/subdir.mk
-include network/subdir.mk
-include engine/status/subdir.mk
-include engine/controller/subdir.mk
-include engine/context/subdir.mk
-include engine/config/servers/subdir.mk
-include engine/config/relay/subdir.mk
-include engine/config/global/subdir.mk
-include engine/config/cache/subdir.mk
-include core/thread/subdir.mk
-include core/strings/subdir.mk
-include core/parsers/special/subdir.mk
-include core/parsers/numbers/subdir.mk
-include core/parsers/formats/subdir.mk
-include core/parsers/subdir.mk
-include core/memory/subdir.mk
-include core/log/subdir.mk
-include core/indexes/subdir.mk
-include core/host/subdir.mk
-include core/hash/subdir.mk
-include core/encodings/subdir.mk
-include core/compare/subdir.mk
-include core/classify/subdir.mk
-include core/buckets/subdir.mk
-include core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: magmad

# Tool invocations
magmad: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"../../magma/.check/" -rdynamic -o"magmad" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) magmad
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
