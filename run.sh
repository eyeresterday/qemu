#!/bin/bash

./build/qemu-system-arm \
            -kernel firmware/FreeRTOS_Toggle_Led_Example_S32K358/Debug_FLASH/FreeRTOS_Toggle_Led_Example_S32K358.elf \
            -machine s32k3x8evb \
            -nographic

