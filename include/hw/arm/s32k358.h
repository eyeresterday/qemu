#ifndef S32K358_H
#define S32K358_H

#include "qemu/units.h"


//FreeRTOS does not use this but just in case.
//Memory is RO (Probably, I still have to double check)
//The two ITCM regions overlap
#define S32K358_ITCM0_BASE          0x00000000
#define S32K358_ITCM_LENGTH         64*KiB

//In practice, only pflash 0 is used, memory regions are contiguous
#define S32K358_PFLASH0_BASE        0x00400000
#define S32K358_PFLASH1_BASE        0x00600000
#define S32K358_PFLASH2_BASE        0x00800000
#define S32K358_PFLASH3_BASE        0x00A00000
#define S32K358_PFLASH_BLK_LENGTH   2*MiB
#define S32K358_PFLASH_BASE         0x00400000
#define S32K358_PFLASH_LENGTH       8*MiB


#define S32K358_DFLASH_BASE         0x10000000
#define S32K358_DFLASH_LENGTH       128*KiB

//SRAM regions are contiguous.
#define S32K358_SRAM0_BASE          0x20400000
#define S32K358_SRAM1_BASE          0x20440000
#define S32K358_SRAM2_BASE          0x20480000
#define S32K358_SRAM_BLK_LENGTH     256*KiB
#define S32K358_SRAM_BASE           0x20400000
#define S32K358_SRAM_LENGTH         768*KiB


#define S32K358_DTCM0_BASE          0x20000000
#define S32K358_DTCM_LENGTH         128*KiB


#define S32K358_AIPS0_BASE           0x40000000
#define S32K358_AIPS1_BASE           0x42000000
#define S32K358_AIPS2_BASE           0x44000000
#define S32K358_AIPS_BLK_LENGTH      2*MiB


#define S32K358_MC_ME_BASE 0x402dc000
#define S32K358_MC_ME_LENGTH 1340


#define S32K358_LPUART0_BASE 0x40328000

#define S32K358_DMA_CSR 0x4020C000

#define S32K358_PAGE_SIZE 256 //bits

#endif
