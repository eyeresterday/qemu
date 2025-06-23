
#ifndef S32K358_SOC_H
#define S32K358_SOC_H

#include "hw/sysbus.h"
#include "hw/arm/armv7m.h"
//#include "hw/arm/s32k3_uart.h"
//#include "hw/arm/s32k3_timer.h"
#include "hw/arm/s32k358.h"
#include "hw/clock.h"
#include "qom/object.h"
#include "hw/char/s32k358_uart.h"

#include <stddef.h>

#define TYPE_S32K358_SOC "s32k358_soc"
OBJECT_DECLARE_SIMPLE_TYPE(S32K358State, S32K358_SOC)

/* UART and DMA device types (defined elsewhere, e.g., in uart.c and dma.c) */
#define TYPE_S32K358_UART "s32k358-uart"
#define TYPE_S32K358_DMA  "s32k358-dma"

#define S32K358_NUM_TIMERS 8

/* SoC state structure */
struct S32K358State {
    /*< private >*/
    /* QEMU object base type */    
    SysBusDevice parent_obj;

    /*< public >*/
    /* ARM Cortex-M core */
    ARMv7MState armv7m;

    /* Memory Regions */
    MemoryRegion itcm;

    MemoryRegion sram[3];
    MemoryRegion pflash[4];
    MemoryRegion dflash;
 
    MemoryRegion dtcm;
    MemoryRegion aips0;
    MemoryRegion aips1;
    MemoryRegion aips2;

    MemoryRegion mc_me;

    MemoryRegion flash_alias;

    /* Peripherals */
    S32K358LPUARTState lpuart[16];
    DeviceState dma;

    /* Clocks */
    Clock *refclk;
    Clock *sysclk;
    Clock *aips_plat_clk;
    Clock *aips_slow_clk;
};

#endif /* S32K358_SOC_H */
