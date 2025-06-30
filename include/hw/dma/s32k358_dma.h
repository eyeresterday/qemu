#ifndef S32K358_DMA_H
#define S32K358_DMA_H

#include "hw/registerfields.h"
#include "qom/object.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"

#define TYPE_S32K358_DMA "s32k358_dma"
OBJECT_DECLARE_SIMPLE_TYPE(S32K358DMAState, S32K358_DMA)

/* DMA Registers */
//REG32(DMA_CSR,              0x00)
REG32(DMA_CSR,             0x00)
    FIELD(DMA_CSR,  EDBG,       1,  1)
    FIELD(DMA_CSR,  ERCA,       2,  1)
    FIELD(DMA_CSR,  HAE,        4,  1) /* Halt */
    FIELD(DMA_CSR,  HALT,       5,  1)
    FIELD(DMA_CSR,  GCLC,       6,  1)
    FIELD(DMA_CSR,  GMRC,       7,  1)
    FIELD(DMA_CSR,  ECX,        8,  1)
    FIELD(DMA_CSR,  CX,         9,  1) /* Cancel Transfer */
    FIELD(DMA_CSR,  ACTIVE_ID,  24, 5) /* Cancel transfer with error */
    FIELD(DMA_CSR,  ACTIVE,     31, 1)
REG32(DMA_ES,               0x04)
REG32(DMA_INT,              0x08)
REG32(DMA_HSR,              0x0C)
REG32(DMA_GRPRI,            0x0C)
    FIELD(DMA_GRPRI,EDBG,       0,  5)
/* TCD Registers */
REG32(CH0_CSR,              0x00)
    FIELD(CH0_CSR,  ERQ,        0,  1)
    FIELD(CH0_CSR,  EARQ,       1,  1)
    FIELD(CH0_CSR,  EEI,        2,  1)
    FIELD(CH0_CSR,  EBW,        3,  1)
    FIELD(CH0_CSR,  DONE,       30, 1)
    FIELD(CH0_CSR,  ACTIVE,     31, 1)
REG32(CH0_ES,               0x04)
REG32(CH0_INT,              0x08)
REG32(CH0_SBR,              0x0C)
    FIELD(CH0_SBR,  MID,        0,  4)
    FIELD(CH0_SBR,  PAL,       15,  1)
    FIELD(CH0_SBR,  EMI,       16,  1)
    FIELD(CH0_SBR,  ATTR,      17,  3)
REG32(CH0_PRI,              0x10)
REG32(TCD0_SADDR,           0x20)
REG16(TCD0_SOFF,            0x24)
REG16(TCD0_ATTR,            0x26)
    FIELD(TCD0_ATTR,DSIZE,     0,  3)
    FIELD(TCD0_ATTR,DMOD,      3,  5)
    FIELD(TCD0_ATTR,SSIZE,     8,  3)
    FIELD(TCD0_ATTR,SMOD,      11, 5)
REG32(TCD0_MLOFF,            0x28)
    FIELD(TCD0_MLOFF,NBYTES,  0,  30)
    FIELD(TCD0_MLOFF,MLOFF,  10,  10)
    FIELD(TCD0_MLOFF,DMLOE,  1,   30)
    FIELD(TCD0_MLOFF,SMLOE,  1,   31)
REG32(TCD0_SLAST,            0x2C)
REG32(TCD0_DADDR,           0x30)
REG16(TCD0_DOFF,            0x34)
REG16(TCD0_CITER,           0x34)
    FIELD(TCD0_CITER,CITER,   0,  15)
    FIELD(TCD0_CITER,ELINK,   15,  1)
REG32(TCD0_DLAST_SGA,       0x38)
REG16(TCD0_CSR,             0x3C)
    FIELD(TCD0_CSR, START,   0,   1)
    FIELD(TCD0_CSR, INTMAJOR,1,   1)
    FIELD(TCD0_CSR, INTHALF, 2,   1)
    FIELD(TCD0_CSR, DREQ,    3,   1)
    FIELD(TCD0_CSR, ESG,     4,   1)
    FIELD(TCD0_CSR, ESDA,    7,   1)
    FIELD(TCD0_CSR, BWC,     14,  2)
REG16(TCD0_BITER,           0x3C)
    FIELD(CH0_BITER,BITER,   0,  15)
    FIELD(CH0_BITER,ELINK,   15,  1)

static const unsigned int dma_irq_base = 20;
static const hwaddr dma_base = 0x4020C000;
static const hwaddr tdc_base = 0x40210000;

struct S32K358DMAState {
    /* Parent class */
    SysBusDevice parent_obj;

    /* QEMU classes */
    MemoryRegion mmio;
    qemu_irq irq;
    MemoryRegion *downstream;
    AddressSpace downstream_as;

    /* Registers */
    
    uint32_t edma_regs[4];
    ///* Channel arbitration groups will be unimplemented */
    uint32_t chn[32][8];
    uint32_t tcd[32][8];
};

#endif
