#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "hw/qdev-properties-system.h"
#include "qemu/log.h"
#include "hw/char/serial.h"
#include "chardev/char-serial.h"
#include "chardev/char-fe.h"
#include "hw/char/s32k358_uart.h"

#define UART(obj) OBJECT_CHECK(S32K358LPUARTState, (obj), TYPE_S32K358_LPUART)

static inline void uart_reset(uint32_t uart_registers[], hwaddr uart_addr) {
   uint32_t reset_registers[] = {
        uart_addr <= lpuart_bases[1] ? 0x04040007 : 0x04040003,
        uart_addr <= lpuart_bases[1] ? 0x00000404: 0x00000202,
        0x00000000,
        0x00000000,
        0x0F000004,
        0x00C00000,
        0x00000000,
        0x00001000,
        0x00000000,
        0x00000000,
        uart_addr <= lpuart_bases[1] ? 0x00C00033: 0x00C00011,
        0x00000000,
        0x00001000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x0000000F,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
   };
   memcpy(uart_registers, reset_registers, sizeof(reset_registers));
}

static void s32k358_lpuart_update_irq(S32K358LPUARTState *s) {
    uint32_t status_mask = s->regs[R_LPUART_CTRL] & s->regs[R_LPUART_STAT] ;
    qemu_set_irq(s->irq, status_mask &
            (R_LPUART_CTRL_TIE_MASK | R_LPUART_CTRL_TCIE_MASK | R_LPUART_CTRL_RIE_MASK));
}

static void s32k358_lpuart_update_baud(S32K358LPUARTState *s) {
    QEMUSerialSetParams params;
    uint32_t oversampling_rate = 1 + FIELD_EX32(s->regs[R_LPUART_BAUD], LPUART_BAUD, OSR);
    uint32_t divisor = FIELD_EX32(s->regs[R_LPUART_BAUD], LPUART_BAUD, SBR);
    uint32_t baud_clock = clock_get_hz(s->clk);
    params.speed = baud_clock * divisor * oversampling_rate;
    params.parity = 1;
    params.stop_bits = 1;

    qemu_chr_fe_ioctl(&s->chr, CHR_IOCTL_SERIAL_SET_PARAMS, &params);
}

static void s32k358_lpuart_reset(DeviceState *dev) {
    S32K358LPUARTState *s = S32K358_LPUART(dev);

    uart_reset(s->regs, 0); /* TODO: Determine number of LPUART */
    s32k358_lpuart_update_irq(s);
    s32k358_lpuart_update_baud(s);
    qemu_chr_fe_accept_input(&s->chr);
}

static int s32k358_lpuart_can_receive(void *opaque) {
    S32K358LPUARTState *s = (S32K358LPUARTState *) opaque;
    return ~(s->regs[R_LPUART_STAT] & R_LPUART_STAT_RDRF_MASK);
}

static void s32k358_lpuart_receive(void *opaque, const uint8_t *buf, int size) {
    S32K358LPUARTState *s = (S32K358LPUARTState *) opaque;
    uint32_t data = *buf;
    if ((s->regs[R_LPUART_CTRL] & R_LPUART_CTRL_RE_MASK) == 0)
        return;

    s->regs[R_LPUART_DATA] = data;
    s->regs[R_LPUART_CTRL] |= R_LPUART_STAT_RDRF_MASK;
    s32k358_lpuart_update_irq(s);
}


static uint64_t s32k358_lpuart_read(void *opaque, hwaddr addr, unsigned size) {
    /* For now the FIFO will not be implemented */
    /* Implementing the transmitter FIFO is pointless since all characters are
     * autromatically transmitted 
     */
    /* TODO: Handle unaligned reads */
    S32K358LPUARTState *s = (S32K358LPUARTState *)opaque;
	addr &= 0xFF;
    switch (addr) {
        case A_LPUART_DATA:
			s->regs[R_LPUART_STAT] &= ~R_LPUART_STAT_RDRF_MASK;
            qemu_chr_fe_accept_input(&s->chr);
            s32k358_lpuart_update_irq(s);
            /* Fallthrough */
        case A_LPUART_STAT:
        case A_LPUART_CTRL:
        case A_LPUART_FIFO:
		case A_LPUART_BAUD:
            return s->regs[addr];
        case A_LPUART_DATARO:
            return s->regs[R_LPUART_DATA];
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "UART: Read from unsupported register 0x%02lx\n", addr);
            return 0;
    }
}

static void s32k358_lpuart_write(void *opaque, hwaddr addr, uint64_t value, unsigned size) {
    S32K358LPUARTState *s = (S32K358LPUARTState *)opaque;
    addr &= 0xFF;
    uint8_t wrval;
    /* For now writing to readonly bits will do nothing */
    switch (addr) {
        case A_LPUART_PINCFG:
            s->regs[R_LPUART_PINCFG] = value & R_LPUART_PINCFG_TRGSEL_MASK;
            break;
        case A_LPUART_GLOBAL:
            s->regs[R_LPUART_GLOBAL] = value;
            if (value & R_LPUART_GLOBAL_RST_MASK)
                s32k358_lpuart_reset((DeviceState *)s);
            break;

        case A_LPUART_DATA:
			s->regs[R_LPUART_STAT] |= R_LPUART_STAT_TDRE_MASK;
            /* Technically you can write 16 and 32 bits to this register, and
             * the data will be added to the fifo, but since we don't have a
             * fifo yet we will ignore anything other than the first 8 bytes
             */
            if (s->regs[R_LPUART_CTRL] & R_LPUART_CTRL_M_MASK)
			    qemu_log_mask(LOG_GUEST_ERROR, "UART: 9 and 10 bits write is unimplemented.\n");
            if (s->regs[R_LPUART_CTRL] & R_LPUART_CTRL_M7_MASK)
                value &= 0x7F;
            wrval = value;
            qemu_chr_fe_write(&s->chr, &wrval, 1);
            break;

        case A_LPUART_BAUD:
            s->regs[R_LPUART_BAUD] = value;
            /* NOTES: LIN break interrupts are not yet implemented
             * RDMAE is not yet implemented
             * TODO: Match Address mode
             */
            s32k358_lpuart_update_baud(s);
            break;

        case A_LPUART_STAT:
            if (value & R_LPUART_STAT_LBKDIF_MASK) {
                s->regs[R_LPUART_STAT] &= ~R_LPUART_STAT_LBKDIF_MASK;
                value &= ~R_LPUART_STAT_LBKDIF_MASK;
            }
            /* RXEDGEIF never occurs */
            value &= ~R_LPUART_STAT_RXEDGIF_MASK; 
            if (value & R_LPUART_STAT_LBKDE_MASK)
			    qemu_log_mask(LOG_GUEST_ERROR, "UART: Line break detection is unimplemented.\n");
            value &= ~R_LPUART_STAT_RAF_MASK; /* TODO */
            value &= ~R_LPUART_STAT_TDRE_MASK; /* TODO */
            value &= ~R_LPUART_STAT_TC_MASK; /* TODO */
            value &= ~R_LPUART_STAT_RDRF_MASK; /* TODO */
            value &= ~R_LPUART_STAT_IDLE_MASK;
            value &= ~R_LPUART_STAT_OR_MASK;
            value &= ~R_LPUART_STAT_NF_MASK; /* There is never noise */
            value &= ~R_LPUART_STAT_FE_MASK; /* There is never a framing error */
            value &= ~R_LPUART_STAT_PF_MASK; /* There is never a parity error */
            /* NOTE: Match not yet implemented */
            value &= ~R_LPUART_STAT_MA1F_MASK;
            value &= ~R_LPUART_STAT_MA2F_MASK;
            value &= ~R_LPUART_STAT_TSF_MASK; /* TODO */
            value &= ~R_LPUART_STAT_MSF_MASK; /* TODO */
            s->regs[R_LPUART_STAT] |= value;
            break;

        case A_LPUART_CTRL:
            s->regs[R_LPUART_CTRL] = value;
            break;
            
		default:
			qemu_log_mask(LOG_GUEST_ERROR, "UART: Write to unsupported register or bad offset: 0x%02lx\n", addr);
            break;
    }
}

static const MemoryRegionOps uart_ops = {
    .read = s32k358_lpuart_read,						//Assigns the functions for reading the UART registers.
    .write = s32k358_lpuart_write,                    //Assigns the functions for writing the UART registers.
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {						//Restricts register access to sizes between 1 and 4 bytes
        .min_access_size = 1,
        .max_access_size = 4,
    },
};

static void s32k358_lpuart_realize(DeviceState *dev, Error **errp) {
    S32K358LPUARTState *s = UART(dev);

    if (!clock_has_source(s->clk)) {
        error_setg(errp, "Clock source not set");
        return;
    }

    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq);
    memory_region_init_io(&s->mmio, OBJECT(s), &uart_ops, s, "uart-mmio", 0x4000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->mmio);
    qemu_chr_fe_set_handlers(&s->chr, s32k358_lpuart_can_receive, s32k358_lpuart_receive, NULL, NULL, s, NULL, true);
}

static void s32k358_lpuart_instance_init(Object *obj) {
    S32K358LPUARTState *s = S32K358_LPUART(obj);

    s->clk = qdev_init_clock_in(DEVICE(s), "clk", NULL, s, 0);
}

static const Property s32k358_lpuart_properties[] = {
    DEFINE_PROP_CHR("chardev", S32K358LPUARTState, chr),
};

static void s32k358_lpuart_class_init(ObjectClass *klass, const void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k358_lpuart_realize;

    //object_class_property_add_link(klass, "chardev", TYPE_CHARDEV, offsetof(S32K358LPUARTState, chr), object_property_allow_set_link, 0);

    device_class_set_props(dc, s32k358_lpuart_properties);
    /*Ideally I should add a resettable interface but I have no time */
    device_class_set_legacy_reset(dc, s32k358_lpuart_reset);
}

static const TypeInfo s32k358_lpuart_info = {
    .name = TYPE_S32K358_LPUART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K358LPUARTState),
    .instance_init = s32k358_lpuart_instance_init,
    .class_init = s32k358_lpuart_class_init,
};

// Registers the UART device type during QEMU initialization.
static void s32k358_lpuart_register_types(void) {
    type_register_static(&s32k358_lpuart_info);
}
// Ensures the device is initialized when QEMU starts.
type_init(s32k358_lpuart_register_types)
