################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include users/subdir.mk
-include providers/subdir.mk
-include objects/subdir.mk
-include network/subdir.mk
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
all: magmad.check

dependents:
	-cd /home/ladar/git/magma.distribution/src/.check && $(MAKE) clean all

# Tool invocations
magmad.check: $(OBJS) $(USER_OBJS) /home/ladar/git/magma.distribution/src/.check/libmagma.a
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/ladar/git/magma.distribution/src/.check" -rdynamic -o"magmad.check" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) magmad.check
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/ladar/git/magma.distribution/src/.check/libmagma.a:

-include ../makefile.targets
