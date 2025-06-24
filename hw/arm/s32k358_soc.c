#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qom/object.h"
#include "hw/arm/armv7m.h"
#include "hw/arm/boot.h"
#include "hw/boards.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "hw/clock.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "hw/misc/unimp.h"
#include "qemu/units.h"
#include "system/address-spaces.h"
#include "system/system.h"
#include "hw/arm/s32k358_soc.h"
#include "hw/char/s32k358_uart.h"

#define FOREACH(iterator, array) for(typeof(&array[0]) iterator = array; iterator < array + sizeof(array)/sizeof(array[0]); iterator++)

#define  XTAL            (50000000UL)
#define VTOR_OFFT 0x0800

#define UART0_ADDR 0x40328000
#define UART1_ADDR 0x4032C000
#define DMA_ADDR   0x4020C000

#define DMA_IRQ   4

enum eMemPerms {
    RO,
    RW,
};

typedef struct {
    const char *name;
    hwaddr base;
    size_t length;
    size_t offset;
    enum eMemPerms perms;
} MemRegion;

typedef struct {
    const char *name;
    hwaddr base;
    size_t length;
    size_t offset;
    MemoryRegionOps ops;
} IOMemRegion;

typedef struct {
    const char *name;
    hwaddr base;
    size_t length;
} UnimpDev;

static const MemRegion mem_regions[] = {
    {
        .name = "S32K358.itcm0",
        .base = S32K358_ITCM0_BASE,
        .length = S32K358_ITCM_LENGTH,
        .offset = offsetof(S32K358State, itcm),
        .perms = RW,
    },
    {
        .name = "S32K358.pflash0",
        .base = S32K358_PFLASH0_BASE,
        .length = S32K358_PFLASH_BLK_LENGTH,
        .offset = offsetof(S32K358State, pflash[0]),
        .perms = RO,
    },
    {
        .name = "S32K358.pflash1",
        .base = S32K358_PFLASH1_BASE,
        .length = S32K358_PFLASH_BLK_LENGTH,
        .offset = offsetof(S32K358State, pflash[1]),
        .perms = RO,
    },
    {
        .name = "S32K358.pflash2",
        .base = S32K358_PFLASH2_BASE,
        .length = S32K358_PFLASH_BLK_LENGTH,
        .offset = offsetof(S32K358State, pflash[2]),
        .perms = RW,
    },
    {
        .name = "S32K358.pflash3",
        .base = S32K358_PFLASH3_BASE,
        .length = S32K358_PFLASH_BLK_LENGTH,
        .offset = offsetof(S32K358State, pflash[3]),
        .perms = RO,
    },
    {
        .name = "S32K358.dflash2",
        .base = S32K358_DFLASH_BASE,
        .length = S32K358_DFLASH_LENGTH,
        .offset = offsetof(S32K358State, dflash),
        .perms = RO,
    },
    {
        .name = "S32K358.sram0",
        .base = S32K358_SRAM0_BASE,
        .length = S32K358_SRAM_BLK_LENGTH,
        .offset = offsetof(S32K358State, sram[0]),
        .perms = RW,
    },
    {
        .name = "S32K358.sram1",
        .base = S32K358_SRAM1_BASE,
        .length = S32K358_SRAM_BLK_LENGTH,
        .offset = offsetof(S32K358State, sram[1]),
        .perms = RW,
    },
    {
        .name = "S32K358.sram2",
        .base = S32K358_SRAM2_BASE,
        .length = S32K358_SRAM_BLK_LENGTH,
        .offset = offsetof(S32K358State, sram[2]),
        .perms = RW,
    },
    {
        .name = "S32K358.dtcm0",
        .base = S32K358_DTCM0_BASE,
        .length = S32K358_DTCM_LENGTH,
        .offset = offsetof(S32K358State, dtcm),
        .perms = RW,
    },
};

static const UnimpDev unimp_devs[] = {
    {.name = "S32K358.unimp.hse_xbic",                    .base = 0x40008000,     .length = 0x4000 },
    {.name = "S32K358.unimp.erm1",                        .base = 0x4000c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pfc1",                        .base = 0x40068000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pfc1_alt",                    .base = 0x4006c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.swt_3",                       .base = 0x40070000,     .length = 0x4000 },
    {.name = "S32K358.unimp.trgmux",                      .base = 0x40080000,     .length = 0x4000 },
    {.name = "S32K358.unimp.bctu",                        .base = 0x40084000,     .length = 0x4000 },
    {.name = "S32K358.unimp.emios0",                      .base = 0x40088000,     .length = 0x4000 },
    {.name = "S32K358.unimp.emios1",                      .base = 0x4008c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.emios2",                      .base = 0x40090000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lcu0",                        .base = 0x40098000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lcu1",                        .base = 0x4009c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.adc_0",                       .base = 0x400a0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.adc_1",                       .base = 0x400a4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.adc_2",                       .base = 0x400a8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pit0",                        .base = 0x400b0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pit1",                        .base = 0x400b4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_2",                        .base = 0x400b8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_2",                        .base = 0x400bc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_3",                        .base = 0x400c4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_3",                        .base = 0x400c8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_4",                        .base = 0x400cc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_4",                        .base = 0x400d0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.axbs",                        .base = 0x40200000,     .length = 0x4000 },
    {.name = "S32K358.unimp.system_xbic",                 .base = 0x40204000,     .length = 0x4000 },
    {.name = "S32K358.unimp.periph_xbic",                 .base = 0x40208000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma",                        .base = 0x4020c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_0",                  .base = 0x40210000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_1",                  .base = 0x40214000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_2",                  .base = 0x40218000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_3",                  .base = 0x4021c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_4",                  .base = 0x40220000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_5",                  .base = 0x40224000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_6",                  .base = 0x40228000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_7",                  .base = 0x4022c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_8",                  .base = 0x40230000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_9",                  .base = 0x40234000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_10",                 .base = 0x40238000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_11",                 .base = 0x4023c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.debug_apb_page0",             .base = 0x40240000,     .length = 0x4000 },
    {.name = "S32K358.unimp.debug_apb_page1",             .base = 0x40244000,     .length = 0x4000 },
    {.name = "S32K358.unimp.debug_apb_page2",             .base = 0x40248000,     .length = 0x4000 },
    {.name = "S32K358.unimp.debug_apb_page3",             .base = 0x4024c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.debug_apb_paged_area",        .base = 0x40250000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sda-ap",                      .base = 0x40254000,     .length = 0x4000 },
    {.name = "S32K358.unimp.eim0",                        .base = 0x40258000,     .length = 0x4000 },
    {.name = "S32K358.unimp.erm0",                        .base = 0x4025c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mscm",                        .base = 0x40260000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pram_0",                      .base = 0x40264000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pfc",                         .base = 0x40268000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pfc_alt",                     .base = 0x4026c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.swt_0",                       .base = 0x40270000,     .length = 0x4000 },
    {.name = "S32K358.unimp.stm_0",                       .base = 0x40274000,     .length = 0x4000 },
    {.name = "S32K358.unimp.xrdc",                        .base = 0x40278000,     .length = 0x4000 },
    {.name = "S32K358.unimp.intm",                        .base = 0x4027c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.dmamux_0",                    .base = 0x40280000,     .length = 0x4000 },
    {.name = "S32K358.unimp.dmamux_1",                    .base = 0x40284000,     .length = 0x4000 },
    {.name = "S32K358.unimp.rtc",                         .base = 0x40288000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mc_rgm",                      .base = 0x4028c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac0_hse",  .base = 0x40290000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac0_hse",  .base = 0x40294000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac1_m7_0", .base = 0x40298000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac1_m7_0", .base = 0x4029c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac2_m7_1", .base = 0x402a0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac2_m7_1", .base = 0x402a4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac3",      .base = 0x402a8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.dcm",                         .base = 0x402ac000,     .length = 0x4000 },
    {.name = "S32K358.unimp.wkpu",                        .base = 0x402b4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.cmu",                         .base = 0x402bc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.tspc",                        .base = 0x402c4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sirc",                        .base = 0x402c8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sxosc",                       .base = 0x402cc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.firc",                        .base = 0x402d0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fxosc",                       .base = 0x402d4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mc_cgm",                      .base = 0x402d8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mc_me",                       .base = 0x402dc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pll",                         .base = 0x402e0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pll2",                        .base = 0x402e4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pmc",                         .base = 0x402e8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fmu",                         .base = 0x402ec000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fmu_alt",                     .base = 0x402f0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac4_m7_2", .base = 0x402f4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac4_m7_2", .base = 0x402f8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pit2",                        .base = 0x402fc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pit3",                        .base = 0x40300000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_0",                   .base = 0x40304000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_1",                   .base = 0x40308000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_2",                   .base = 0x4030c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_3",                   .base = 0x40310000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_4",                   .base = 0x40314000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_5",                   .base = 0x40318000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_6",                   .base = 0x4031c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_7",                   .base = 0x40320000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexio",                      .base = 0x40324000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_0",                    .base = 0x40328000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_1",                    .base = 0x4032c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_2",                    .base = 0x40330000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_3",                    .base = 0x40334000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_4",                    .base = 0x40338000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_5",                    .base = 0x4033c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_6",                    .base = 0x40340000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_7",                    .base = 0x40344000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac5_m7_3", .base = 0x40348000,     .length = 0x4000 },
    {.name = "S32K358.unimp.siul_virtwrapper_pdac5_m7_3", .base = 0x4034c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpi2c_0",                     .base = 0x40350000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpi2c_1",                     .base = 0x40354000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_0",                     .base = 0x40358000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_1",                     .base = 0x4035c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_2",                     .base = 0x40360000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_3",                     .base = 0x40364000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sai0",                        .base = 0x4036c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpcmp_0",                     .base = 0x40370000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpcmp_1",                     .base = 0x40374000,     .length = 0x4000 },
    {.name = "S32K358.unimp.tmu",                         .base = 0x4037c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.crc",                         .base = 0x40380000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fccu_",                       .base = 0x40384000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_0",                        .base = 0x4038c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_1",                        .base = 0x40390000,     .length = 0x4000 },
    {.name = "S32K358.unimp.jdc",                         .base = 0x40394000,     .length = 0x4000 },
    {.name = "S32K358.unimp.configuration_gpr",           .base = 0x4039c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.stcu",                        .base = 0x403a0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.selftest_gpr",                .base = 0x403b0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.aes_accel",                   .base = 0x403c0000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app0",                    .base = 0x403d0000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app1",                    .base = 0x403e0000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app2",                    .base = 0x403f0000,     .length = 0x10000},
    {.name = "S32K358.unimp.tcm_xbic",                    .base = 0x40400000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_xbic",                   .base = 0x40404000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pram2_tcm_xbic",              .base = 0x40408000,     .length = 0x4000 },
    {.name = "S32K358.unimp.aes_mux_xbic",                .base = 0x4040c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_12",                 .base = 0x40410000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_13",                 .base = 0x40414000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_14",                 .base = 0x40418000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_15",                 .base = 0x4041c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_16",                 .base = 0x40420000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_17",                 .base = 0x40424000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_18",                 .base = 0x40428000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_19",                 .base = 0x4042c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_20",                 .base = 0x40430000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_21",                 .base = 0x40434000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_22",                 .base = 0x40438000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_23",                 .base = 0x4043c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_24",                 .base = 0x40440000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_25",                 .base = 0x40444000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_26",                 .base = 0x40448000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_27",                 .base = 0x4044c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_28",                 .base = 0x40450000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_29",                 .base = 0x40454000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_30",                 .base = 0x40458000,     .length = 0x4000 },
    {.name = "S32K358.unimp.edma_tcd_31",                 .base = 0x4045c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sema42",                      .base = 0x40460000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pram_1",                      .base = 0x40464000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pram_2",                      .base = 0x40468000,     .length = 0x4000 },
    {.name = "S32K358.unimp.swt_1",                       .base = 0x4046c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.swt_2",                       .base = 0x40470000,     .length = 0x4000 },
    {.name = "S32K358.unimp.stm_1",                       .base = 0x40474000,     .length = 0x4000 },
    {.name = "S32K358.unimp.stm_2",                       .base = 0x40478000,     .length = 0x4000 },
    {.name = "S32K358.unimp.stm_3",                       .base = 0x4047c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.emac",                        .base = 0x40480000,     .length = 0x4000 },
    {.name = "S32K358.unimp.gmac0",                       .base = 0x40484000,     .length = 0x4000 },
    {.name = "S32K358.unimp.gmac1",                       .base = 0x40488000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_8",                    .base = 0x4048c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_9",                    .base = 0x40490000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_10",                   .base = 0x40494000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_11",                   .base = 0x40498000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_12",                   .base = 0x4049c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_13",                   .base = 0x404a0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_14",                   .base = 0x404a4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpuart_15",                   .base = 0x404a8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_4",                     .base = 0x404bc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpspi_5",                     .base = 0x404c0000,     .length = 0x4000 },
    {.name = "S32K358.unimp.quadspi",                     .base = 0x404cc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.sai1",                        .base = 0x404dc000,     .length = 0x4000 },
    {.name = "S32K358.unimp.usdhc",                       .base = 0x404e4000,     .length = 0x4000 },
    {.name = "S32K358.unimp.lpcmp_2",                     .base = 0x404e8000,     .length = 0x4000 },
    {.name = "S32K358.unimp.mu_1",                        .base = 0x404ec000,     .length = 0x4000 },
    {.name = "S32K358.unimp.eim0",                        .base = 0x4050c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.eim1",                        .base = 0x40510000,     .length = 0x4000 },
    {.name = "S32K358.unimp.eim2",                        .base = 0x40514000,     .length = 0x4000 },
    {.name = "S32K358.unimp.eim3",                        .base = 0x40518000,     .length = 0x4000 },
    {.name = "S32K358.unimp.aes_app3",                    .base = 0x40520000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app4",                    .base = 0x40530000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app5",                    .base = 0x40540000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app6",                    .base = 0x40550000,     .length = 0x10000},
    {.name = "S32K358.unimp.aes_app7",                    .base = 0x40560000,     .length = 0x10000},
    {.name = "S32K358.unimp.flexcan_8",                   .base = 0x40570000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_9",                   .base = 0x40574000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_10",                  .base = 0x40578000,     .length = 0x4000 },
    {.name = "S32K358.unimp.flexcan_11",                  .base = 0x4057c000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fmu1",                        .base = 0x40580000,     .length = 0x4000 },
    {.name = "S32K358.unimp.fmu1_alt",                    .base = 0x40584000,     .length = 0x4000 },
    {.name = "S32K358.unimp.pram_3",                      .base = 0x40588000,     .length = 0x4000 },
};

static void lpuart_init(S32K358State *s, Error  **errp) {
    DeviceState *armv7m = DEVICE(&s->armv7m);

    for (size_t i = 0; i < 16; i++) {
        DeviceState *dev = DEVICE(&(s->lpuart[i]));
        SysBusDevice *busdev = SYS_BUS_DEVICE(dev);

        qdev_prop_set_chr(dev, "chardev", serial_hd(i));
        qdev_connect_clock_in(dev, "clk", i == 0 || i == 1 || i == 8 ? s->aips_plat_clk : s->aips_slow_clk);

        if (!sysbus_realize(busdev, errp))
            return;

        busdev = SYS_BUS_DEVICE(dev);
        sysbus_mmio_map(busdev, 0, lpuart_bases[i]);
        sysbus_connect_irq(busdev, 0, qdev_get_gpio_in(armv7m, lpuart_irq[i]));
    }
}

/* This is done because FreeRTOS checks this memory region for this value before
 * initialization.
 * */
static uint64_t mc_me_dummy_read(void *opaque, hwaddr addr, unsigned int size) {
    return  addr == 0x00000310? 0x01000000 : 0x00000000;
}

static void mc_me_dummy_write(void *opaque, hwaddr addr, uint64_t val, unsigned int size) {
    return;
}

static const MemoryRegionOps mc_me_dummy_ops = {
    .read = mc_me_dummy_read,
    .write = mc_me_dummy_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s32k358_soc_realize(DeviceState *dev_soc, Error **errp)
{
    /* Cast the generic DeviceState pointer to the SoC-specific state. */
    S32K358State *s = S32K358_SOC(dev_soc);            
    /* Get the global system memory region, which acts as the root for all memory mappings. */
    MemoryRegion *system_memory = get_system_memory(); 
    DeviceState *armv7m;
    //SysBusDevice *busdev;
    Error *err = NULL;

    /*
     * We use s->refclk internally and only define it with qdev_init_clock_in()
     * so it is correctly parented and not leaked on an init/deinit; it is not
     * intended as an externally exposed clock.
     */

    if (clock_has_source(s->refclk)) {
        error_setg(errp, "refclk clock must not be wired up by the board code");
        return;
    }

    if (!clock_has_source(s->sysclk)) {
        error_setg(errp, "sysclk clock must be wired up by the board code");
        return;
    }
    
    /* Initialize clocks */
    clock_set_mul_div(s->refclk, 8, 1);
    clock_set_source(s->refclk, s->sysclk);
    clock_set_hz(s->aips_plat_clk, 80000000);
    clock_set_hz(s->aips_plat_clk, 40000000);

    /* Initialization of the memory mappings */
    FOREACH(region, mem_regions) {
        switch (region->perms) {
            case RO:
                memory_region_init_rom((MemoryRegion*) ((char *)s + region->offset) , OBJECT(dev_soc), region->name, region->length, &err);
                break;
            case RW:
                memory_region_init_ram((MemoryRegion*) ((char *)s + region->offset) , OBJECT(dev_soc), region->name, region->length, &err);
                break;
        }
        if (err != NULL) {
            error_propagate(errp, err);
            return;
        }
        memory_region_add_subregion(system_memory, region->base, (MemoryRegion *)((char *)s + region->offset));
    }

    /* Initialize mc_me dummy region */
    memory_region_init_io(&s->mc_me, OBJECT(dev_soc), &mc_me_dummy_ops, s, "S32K358.mc_me", S32K358_MC_ME_LENGTH);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mc_me);
    memory_region_add_subregion(system_memory, S32K358_MC_ME_BASE, &s->mc_me);

    /* Declare unimplemented devices */
    FOREACH(device, unimp_devs)
        create_unimplemented_device(device->name, device->base, device->length);

    /* Configuration of the Arm Cortex-M device
     * Many of those values can be found on a specific table in the manual
     */
    armv7m = DEVICE(&s->armv7m);
    qdev_prop_set_uint32(   armv7m,    "num-irq",           240                            );
    qdev_prop_set_uint8(    armv7m,    "num-prio-bits",     4                              );
    qdev_prop_set_string(   armv7m,    "cpu-type",          ARM_CPU_TYPE_NAME("cortex-m7") );
    qdev_prop_set_uint32(   armv7m,    "init-svtor",        S32K358_PFLASH_BASE + VTOR_OFFT);
    qdev_prop_set_uint32(   armv7m,    "init-nsvtor",       S32K358_PFLASH_BASE + VTOR_OFFT);
    /* The way the IVT works in this board is complicated. A structure which can
     * be in many places but one of them is in the start of the program flash is
     * read by the SBAF, which then sets the configuration of the board
     * according to the fields in this structure, and also sets the VTOR for
     * each core. We can't do that here without significant effort. However, the
     * address of the IVT can be found in the symbol __COREn_VTOR (CORE0 in this
     * case) in the ELF file.
     */
    qdev_prop_set_uint32(   armv7m,    "mpu-ns-regions",    16                             );
    qdev_prop_set_uint32(   armv7m,    "mpu-s-regions",     16                             );
    qdev_prop_set_bit(      armv7m,    "enable-bitband",    false                          ); //Our CPU does not support bitbanding.
    qdev_connect_clock_in(  armv7m,    "cpuclk",            s->sysclk                      );
    qdev_connect_clock_in(  armv7m,    "refclk",            s->refclk                      );
    
    object_property_set_link(OBJECT(&s->armv7m), "memory",
                             OBJECT(get_system_memory()), &error_abort);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), errp)) {
        return;
    }

    lpuart_init(s, errp);
}

static void s32k358_soc_class_init(ObjectClass *class, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(class);

    dc->realize = s32k358_soc_realize;
}

static void s32k358_soc_init(Object *obj)
{
    S32K358State *s = S32K358_SOC(obj);
    object_initialize_child(obj, "armv7m", &s->armv7m, TYPE_ARMV7M);

    s->refclk = qdev_init_clock_in(DEVICE(s), "refclk", NULL, NULL, 0);
    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL, 0);
    s->aips_plat_clk = qdev_init_clock_in(DEVICE(s), "aips_plat_clk", NULL, NULL, 0);
    s->aips_slow_clk = qdev_init_clock_in(DEVICE(s), "aips_slow_clk", NULL, NULL, 0);

    for (int i = 0; i < 16; i++)
        object_initialize_child(obj, "lpuart[*]", &s->lpuart[i], TYPE_S32K358_LPUART);
}

static const TypeInfo s32k358_soc_info = {
    .name           = TYPE_S32K358_SOC,
    .parent         = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(S32K358State),
    .instance_init  = s32k358_soc_init,
    .class_init     = s32k358_soc_class_init,
};

static void s32k358_soc_types(void)
{
    type_register_static(&s32k358_soc_info);
}

type_init(s32k358_soc_types)
