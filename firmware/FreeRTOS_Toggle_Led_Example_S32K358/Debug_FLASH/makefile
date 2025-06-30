################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include generate/src/subdir.mk
-include board/subdir.mk
-include RTD/src/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include FreeRTOS/Source/portable/MemMang/subdir.mk
-include FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/subdir.mk
-include FreeRTOS/Source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_SIZE += \
FreeRTOS_Toggle_Led_Example_S32K358.siz \


# All Target
all: FreeRTOS_Toggle_Led_Example_S32K358.elf secondary-outputs

# Tool invocations
FreeRTOS_Toggle_Led_Example_S32K358.elf: $(OBJS) C:/Users/WangblowsMicroshaft/workspaceS32DS.3.5/FreeRTOS_Toggle_Led_Example_S32K358/Project_Settings/Linker_Files/linker_flash_s32k358.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "FreeRTOS_Toggle_Led_Example_S32K358.elf" "@FreeRTOS_Toggle_Led_Example_S32K358.args"  $(USER_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

FreeRTOS_Toggle_Led_Example_S32K358.siz: FreeRTOS_Toggle_Led_Example_S32K358.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley FreeRTOS_Toggle_Led_Example_S32K358.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
