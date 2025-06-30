#ifndef S32K358_LPUART_H
#define S32K358_LPUART_H

#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "chardev/char-fe.h"
#include "qapi/error.h"
#include "qemu/timer.h"
#include "hw/registerfields.h"
#include "qom/object.h"
#include "hw/qdev-clock.h"

#define TYPE_S32K358_LPUART "s32k358-lpuart"
OBJECT_DECLARE_SIMPLE_TYPE(S32K358LPUARTState, S32K358_LPUART)

REG32(LPUART_VERID,             0x00)
    FIELD(LPUART_VERID, FEATURE,      0, 16) 
    FIELD(LPUART_VERID, MINOR,      16, 8) 
    FIELD(LPUART_VERID, MAJOR,      24, 8) // FIELD(reg, field, shift, length)
REG32(LPUART_PARAM,             0x04)
    FIELD(LPUART_PARAM, RXFIFO,     0, 8)
    FIELD(LPUART_PARAM, TXFIFO,     8, 8)
REG32(LPUART_GLOBAL,            0x08)
    FIELD(LPUART_GLOBAL, RST,       1, 1)
REG32(LPUART_PINCFG,            0x0C)
    FIELD(LPUART_PINCFG, TRGSEL,    0, 2)
REG32(LPUART_BAUD,              0x10);
    FIELD(LPUART_BAUD, SBR,         0, 12)
    FIELD(LPUART_BAUD, SBNS,        13, 1)
    FIELD(LPUART_BAUD, RXEDGIE,     14, 1)
    FIELD(LPUART_BAUD, LBKDIE,      15, 1)
    FIELD(LPUART_BAUD, RESYNCDIS,   16, 1)
    FIELD(LPUART_BAUD, BOTHEDGE,    17, 1)
    FIELD(LPUART_BAUD, MATCFG,      18, 2)
    FIELD(LPUART_BAUD, RDMAE,       21, 1)
    FIELD(LPUART_BAUD, TDMAE,       23, 1)
    FIELD(LPUART_BAUD, OSR,         24, 5)
    FIELD(LPUART_BAUD, M10,         29, 1)
    FIELD(LPUART_BAUD, MAEN2,       30, 1)
    FIELD(LPUART_BAUD, MAEN1,       31, 1)
REG32(LPUART_STAT,              0x14)
    FIELD(LPUART_STAT, LBKFE,       0, 1)
    FIELD(LPUART_STAT, AME,         1, 1)
    FIELD(LPUART_STAT, MSF,         8, 1)
    FIELD(LPUART_STAT, TSF,         9, 1)
    FIELD(LPUART_STAT, MA2F,        14, 1)
    FIELD(LPUART_STAT, MA1F,        15, 1)
    FIELD(LPUART_STAT, PF,          16, 1)
    FIELD(LPUART_STAT, FE,          17, 1)
    FIELD(LPUART_STAT, NF,          18, 1)
    FIELD(LPUART_STAT, OR,          19, 1)
    FIELD(LPUART_STAT, IDLE,        20, 1)
    FIELD(LPUART_STAT, RDRF,        21, 1)
    FIELD(LPUART_STAT, TC,          22, 1)
    FIELD(LPUART_STAT, TDRE,        23, 1)
    FIELD(LPUART_STAT, RAF,         24, 1)
    FIELD(LPUART_STAT, LBKDE,       25, 1)
    FIELD(LPUART_STAT, BRK13,       26, 1)
    FIELD(LPUART_STAT, RWUID,       27, 1)
    FIELD(LPUART_STAT, RXINV,       28, 1)
    FIELD(LPUART_STAT, MSBF,        29, 1)
    FIELD(LPUART_STAT, RXEDGIF,     30, 1)
    FIELD(LPUART_STAT, LBKDIF,      31, 1)
REG32(LPUART_CTRL,              0x18)
    FIELD(LPUART_CTRL, PT,       0, 1)
    FIELD(LPUART_CTRL, PE,       1, 1)
    FIELD(LPUART_CTRL, ILT,      2, 1)
    FIELD(LPUART_CTRL, WAKE,     3, 1)
    FIELD(LPUART_CTRL, M,        4, 1)
    FIELD(LPUART_CTRL, RSRC,     5, 1)
    FIELD(LPUART_CTRL, LOOPS,    7, 1)
    FIELD(LPUART_CTRL, IDLECFG,  8, 3)
    FIELD(LPUART_CTRL, M7,       11, 1)
    FIELD(LPUART_CTRL, MA2IE,    14, 1)
    FIELD(LPUART_CTRL, MA1IE,    15, 1)
    FIELD(LPUART_CTRL, SBK,      16, 1)
    FIELD(LPUART_CTRL, RWU,      17, 1)
    FIELD(LPUART_CTRL, RE,       18, 1)
    FIELD(LPUART_CTRL, TE,       19, 1)
    FIELD(LPUART_CTRL, ILIE,     20, 1)
    FIELD(LPUART_CTRL, RIE,      21, 1)
    FIELD(LPUART_CTRL, TCIE,     22, 1)
    FIELD(LPUART_CTRL, TIE,      23, 1)
    FIELD(LPUART_CTRL, PEIE,     24, 1)
    FIELD(LPUART_CTRL, FEIE,     25, 1)
    FIELD(LPUART_CTRL, NEIE,     26, 1)
    FIELD(LPUART_CTRL, ORIE,     27, 1)
    FIELD(LPUART_CTRL, TXINV,    28, 1)
    FIELD(LPUART_CTRL, TXDIR,    29, 1)
    FIELD(LPUART_CTRL, R9T8,     30, 1)
    FIELD(LPUART_CTRL, R8T9,     31, 1)
REG32(LPUART_DATA,              0x1C)
    FIELD(LPUART_DATA, R0T0,       0, 1)
    FIELD(LPUART_DATA, R1T1,       1, 1)
    FIELD(LPUART_DATA, R2T2,       2, 1)
    FIELD(LPUART_DATA, R3T3,       3, 1)
    FIELD(LPUART_DATA, R4T4,       4, 1)
    FIELD(LPUART_DATA, R5T5,       5, 1)
    FIELD(LPUART_DATA, R6T6,       6, 1)
    FIELD(LPUART_DATA, R7T7,       7, 1)
    FIELD(LPUART_DATA, R8T8,       8, 1)
    FIELD(LPUART_DATA, R9T9,       9, 1)
    FIELD(LPUART_DATA, LINBRK,     10, 1)
    FIELD(LPUART_DATA, IDLINE,     11, 1)
    FIELD(LPUART_DATA, RXEMPT,     12, 1)
    FIELD(LPUART_DATA, FRETSC,     13, 1)
    FIELD(LPUART_DATA, PARITYE,    14, 1)
    FIELD(LPUART_DATA, NOISY,      15, 1)
REG32(LPUART_MATCH,             0x20)
    FIELD(LPUART_MATCH, MA1,       0, 10)
    FIELD(LPUART_MATCH, MA2,       16, 10)
REG32(LPUART_MODIR,             0x24)
    FIELD(LPUART_MODIR, TXCTSE,    0, 1)
    FIELD(LPUART_MODIR, TXRTSE,    1, 1)
    FIELD(LPUART_MODIR, TXRTSPOL,  2, 1)
    FIELD(LPUART_MODIR, TXCTSC,    3, 1)
    FIELD(LPUART_MODIR, RXRTSE,    4, 1)
    FIELD(LPUART_MODIR, TXCTSSRC,  5, 1)
    FIELD(LPUART_MODIR, RTSWATER,  8, 4)
    FIELD(LPUART_MODIR, TNP,       16, 2)
    FIELD(LPUART_MODIR, IREN,      18, 1)
REG32(LPUART_FIFO,              0x28)
    FIELD(LPUART_FIFO, RXFIFOSIZE, 0, 3)
    FIELD(LPUART_FIFO, RXFE,       3, 1)
    FIELD(LPUART_FIFO, TXFIFOSIZE, 4, 3)
    FIELD(LPUART_FIFO, TXFE,       7, 1)
    FIELD(LPUART_FIFO, RXUFE,      8, 1)
    FIELD(LPUART_FIFO, TXOFE,      9, 1)
    FIELD(LPUART_FIFO, RXIDEN,     10, 3)
    FIELD(LPUART_FIFO, RXFLUSH,    14, 3)
    FIELD(LPUART_FIFO, TXFLUSH,    15, 3)
    FIELD(LPUART_FIFO, RXUF,       16, 3)
    FIELD(LPUART_FIFO, TXOF,       17, 3)
    FIELD(LPUART_FIFO, RXEMPT,     22, 3)
    FIELD(LPUART_FIFO, TXEMPT,     23, 3)
REG32(LPUART_WATER,             0x2C)
    FIELD(LPUART_WATER, TXWATER,   0, 3)
    FIELD(LPUART_WATER, TXCOUNT,   8, 5)
    FIELD(LPUART_WATER, RXWATER,   16, 3)
    FIELD(LPUART_WATER, RXCOUNT,   24, 5)
REG32(LPUART_DATARO,            0x30)
    FIELD(LPUART_DATARO,DATA,      0, 16)
REG32(LPUART_MCR,                  0x40)
    FIELD(LPUART_MCR, CTS,         0, 1)
    FIELD(LPUART_MCR, DSR,         1, 1)
    FIELD(LPUART_MCR, RIN,         2, 1)
    FIELD(LPUART_MCR, DCD,         3, 1)
    FIELD(LPUART_MCR, DTR,         8, 1)
    FIELD(LPUART_MCR, RTS,         9, 1)
REG32(LPUART_MSR,               0x44)
    FIELD(LPUART_MSR, DCTS,        0, 1)
    FIELD(LPUART_MSR, DDSR,        1, 1)
    FIELD(LPUART_MSR, DRI,         2, 1)
    FIELD(LPUART_MSR, DDCD,        3, 1)
    FIELD(LPUART_MSR, CTS,         4, 1)
    FIELD(LPUART_MSR, DSR,         5, 1)
    FIELD(LPUART_MSR, RIN,         5, 1)
    FIELD(LPUART_MSR, DCD,         7, 1)
REG32(LPUART_REIR,              0x48)
    FIELD(LPUART_REIR, IDTIME,     0, 14)
REG32(LPUART_TEIR,              0x4C)
    FIELD(LPUART_TEIR, IDTIME,     0, 14)
REG32(LPUART_HDCR,              0x50)
    FIELD(LPUART_HDCR, TXSTALL,    0, 1)
    FIELD(LPUART_HDCR, RXSEL,      1, 1)
    FIELD(LPUART_HDCR, RXWRMSK,    2, 1)
    FIELD(LPUART_HDCR, RXMSK,      3, 1)
    FIELD(LPUART_HDCR, RTSEXT,     8, 8)
REG32(LPUART_TOCR,              0x58)
    FIELD(LPUART_TOCR, TOEN,     0, 4)
    FIELD(LPUART_TOCR, TOIE,     8, 4)
REG32(LPUART_TOSR,              0x5C)
    FIELD(LPUART_TOSR, TOZ,      0, 4)
    FIELD(LPUART_TOSR, TOF,      8, 4)
REG32(LPUART_TIMEOUT0,           0x60)
    FIELD(LPUART_TIMEOUT0, TIMEOUT, 0, 14)
    FIELD(LPUART_TIMEOUT0, CFG,     30, 2)
REG32(LPUART_TIMEOUT1,           0x60)
    FIELD(LPUART_TIMEOUT1, TIMEOUT, 0, 14)
    FIELD(LPUART_TIMEOUT1, CFG,     30, 2)
REG32(LPUART_TIMEOUT2,           0x60)
    FIELD(LPUART_TIMEOUT2, TIMEOUT, 0, 14)
    FIELD(LPUART_TIMEOUT2, CFG,     30, 2)
REG32(LPUART_TIMEOUT3,           0x60)
    FIELD(LPUART_TIMEOUT3, TIMEOUT, 0, 14)
    FIELD(LPUART_TIMEOUT3, CFG,     30, 2)
/* Uh, there were also supposed to be 128 control and data burst registers here
 * but I'm not doing that
 */

static const hwaddr lpuart_bases[16] = {
    0x40328000,
    0x4032C000,
    0x40330000,
    0x40334000,
    0x40338000,
    0x4033C000,
    0x40340000,
    0x40344000,
    0x4048C000,
    0x40490000,
    0x40494000,
    0x40498000,
    0x4049C000,
    0x404A0000,
    0x404A4000,
    0x404A8000,
};

//static const unsigned lpuart_irq[] = {141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156}; 
static const unsigned lpuart_irq_base = 141; 


struct S32K358LPUARTState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    Clock *clk;
    qemu_irq irq;

    CharBackend chr;

    uint32_t regs[0x70];
    uint8_t rxfifo[256];
    uint8_t rxfifohead;
    uint8_t rxfifotail;
    uint8_t rxfifoen;
};

#endif
