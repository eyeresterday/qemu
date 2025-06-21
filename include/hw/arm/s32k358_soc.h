
#ifndef S32K358_SOC_H
#define S32K358_SOC_H

#include "hw/sysbus.h"
#include "hw/arm/armv7m.h"
//#include "hw/arm/s32k3_uart.h"
//#include "hw/arm/s32k3_timer.h"
#include "hw/arm/s32k358.h"
#include "hw/clock.h"
#include "qom/object.h"
#include <stddef.h>

#define TYPE_S32K358_SOC "s32k358_soc"       // Defines the unique type name for this SoC
OBJECT_DECLARE_SIMPLE_TYPE(S32K358State, S32K358_SOC)  // Declares the type and associated methods for the SoC in QEMU's object model.

/* UART and DMA device types (defined elsewhere, e.g., in uart.c and dma.c) */
#define TYPE_S32K358_UART "s32k358-uart"   //Defines the type name for the UART peripheral.
#define TYPE_S32K358_DMA  "s32k358-dma"    //Defines the type name for the DMA peripheral.

#define S32K358_NUM_TIMERS 8 //?

////---represents the state of our board during the emulation. Every connected device needs to be part of the state ---////
/* SoC state structure */
struct S32K358State {
    /*< private >*/
    /* QEMU object base type */    
    SysBusDevice parent_obj;

    /*< public >*/
    /* ARM Cortex-M core */
    ARMv7MState armv7m;

    /* Memory Regions */
    MemoryRegion itcm; //RO, probably
    // Overlaps itcm2

    MemoryRegion sram[3]; //RW
    //	Program Flash
    MemoryRegion pflash[4]; //RX
    //I don't think it's possible to model a RX behavior but we can model a RO behavior
    //	Data Flash
    MemoryRegion dflash; //RW
 
    MemoryRegion dtcm; //RW
    // Overlabps dtcm2

    MemoryRegion aips0; //RO?
    MemoryRegion aips1; //RO?
    MemoryRegion aips2; //RO?

    MemoryRegion mc_me;

    MemoryRegion flash_alias;

    //Declare the device state for UART and DMA.
    /* Peripherals */
    DeviceState uart0;       // UART0 device
    DeviceState uart1;       // UART1 device
    DeviceState dma;         // DMA controller device

    /* Clocks */
    Clock *refclk;
    Clock *sysclk;
    Clock *aips_plat_clk;
    Clock *aips_slow_clk;
};

#endif /* S32K358_SOC_H */
