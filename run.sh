#!/bin/bash

#./build/qemu-system-arm -d int,guest_errors -kernel ~/debian-home/nxp/FreeRTOS_Toggle_Led_Example_S32K358/Debug_FLASH/FreeRTOS_Toggle_Led_Example_S32K358.elf -machine s32k3x8evb -nographic -serial mon:stdio -serial none -serial none -s -S

./build/qemu-system-arm \
            -d int,guest_errors \
            -kernel ~/debian-home/nxp/FreeRTOS_Toggle_Led_Example_S32K358/Debug_FLASH/FreeRTOS_Toggle_Led_Example_S32K358.elf \
            -machine s32k3x8evb \
            -serial mon:stdio \
            -nographic \

