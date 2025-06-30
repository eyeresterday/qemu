#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "hw/qdev-properties-system.h"
#include "qemu/log.h"
#include "qapi/error.h"
#include "hw/dma/s32k358_dma.h"

static const uint32_t dma_reset_vec[] = {
    0x00300000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
};

static const uint32_t chn_reset_vec[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00008002,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
};

static const uint32_t tcd_reset_vec[8] = {
    0x00000000,
};

static void s32k358_dma_run(S32K358DMAState *s) {
    static const int transfer_sizes[] = {1, 2, 4, 8};

    hwaddr saddr = s->tcd[0][R_TCD0_SADDR];
    hwaddr daddr = s->tcd[0][R_TCD0_DADDR];
    uint32_t dsize = transfer_sizes[s->tcd[0][R_TCD0_ATTR] & R_TCD0_ATTR_DSIZE_MASK >> R_TCD0_ATTR_DSIZE_SHIFT];
    uint32_t dmod = (s->tcd[0][R_TCD0_ATTR] & R_TCD0_ATTR_DMOD_MASK) >> R_TCD0_ATTR_DMOD_SHIFT;
    uint32_t ssize = transfer_sizes[(s->tcd[0][R_TCD0_ATTR] & R_TCD0_ATTR_SSIZE_MASK) >> R_TCD0_ATTR_SSIZE_SHIFT];
    uint32_t smod = (s->tcd[0][R_TCD0_ATTR] & R_TCD0_ATTR_SMOD_MASK) >> R_TCD0_ATTR_SMOD_SHIFT;
    int32_t slast = s->tcd[0][R_TCD0_SLAST];
    int32_t dlast = s->tcd[0][R_TCD0_DLAST_SGA];
    hwaddr soff = s->tcd[0][R_TCD0_SOFF];
    hwaddr doff = s->tcd[0][R_TCD0_DOFF];
    uint32_t int_flag = s->tcd[0][R_TCD0_CSR] & R_TCD0_CSR_INTMAJOR_MASK;
    uint32_t nbytes = (s->tcd[0][R_TCD0_MLOFF] & R_TCD0_MLOFF_NBYTES_MASK) >> R_TCD0_MLOFF_NBYTES_SHIFT;
    int readc = 0;
    uint64_t sbuf;
    /* Multibyte writes/reads are unimplemented */
    if (!smod)
        smod = 0xFFFFFFFF;
    if (!dmod)
        dmod = 0xFFFFFFFF;

    for (size_t i = 0; i < nbytes; i++) {
        uint64_t readval = 0;
        address_space_read(&s->downstream_as, saddr, MEMTXATTRS_UNSPECIFIED, &readval, ssize);
        readc += ssize;
        sbuf = (readval << (8 << ssize)) | readval;
        if (readc >= dsize) {
            while (readc) {
                address_space_write(&s->downstream_as, daddr, MEMTXATTRS_UNSPECIFIED, &sbuf, dsize);
                readc -= dsize;
                sbuf >>= (8 << dsize);
            }
        }
        saddr += soff;
        daddr += doff;
    }
    s->tcd[0][R_TCD0_SADDR] = saddr + slast;
    s->tcd[0][R_TCD0_DADDR] = daddr + dlast;
    s->tcd[0][R_CH0_CSR] &= R_CH0_CSR_ACTIVE_MASK;
    s->tcd[0][R_CH0_CSR] |= R_CH0_CSR_DONE_MASK;
    if (int_flag) {
        s->tcd[0][R_CH0_INT] |= 1;
        s->edma_regs[R_DMA_INT] |= 1;
        qemu_set_irq(s->irq, 1);
    }
}

static void s32k358_dma_reset(DeviceState *dev) {
    S32K358DMAState *s = S32K358_DMA(dev);
    (void) s;

    memcpy(&s->edma_regs, dma_reset_vec, sizeof(s->edma_regs));
    memcpy(&s->chn[0], chn_reset_vec, sizeof(s->chn[0]));
    /*Ideally we should reset all 32 channels but for now we're only using only
     * one
     */
    memcpy(&s->tcd[0], tcd_reset_vec, sizeof(s->tcd[0]));
}

static uint64_t s32k358_dma_read(void *opaque, hwaddr offset, unsigned size) {
    S32K358DMAState *s = (S32K358DMAState *) opaque;

    if (offset > 0x4021003E || (0x40210000 > offset && offset > 0x4020000C)) {
        qemu_log_mask(LOG_GUEST_ERROR, "DMA: Read from unimplemented channel or arbitration group: 0x%02lx\n", offset);
        return 0;
    }
    
    if (offset <= 0x4020000C) {
        return s->edma_regs[(offset & 0x0F) / 4];
    }
    if (0x40210000 <= offset && offset < 0x40210020)
        return s->chn[0][offset & 0xFF];
    else
        return s->tcd[0][(offset & 0xFF) - 0x20];
}

static void s32k358_dma_write(void *opaque, hwaddr offset, uint64_t value, unsigned size) {
    S32K358DMAState *s = (S32K358DMAState *) opaque;

    if (offset > 0x4021003E || (0x40210000 > offset && offset > 0x4020000C)) {
        qemu_log_mask(LOG_GUEST_ERROR, "DMA: Read from unimplemented channel or arbitration group: 0x%02lx\n", offset);
        return;
    }
    
    if (offset <= 0x4020000C) {
        switch (offset & 0xFF) {
            case A_DMA_CSR:
                value &= ~0x60FFFC09;
                break;
            case A_DMA_ES:
                value &= ~0x60FFFCC00;
                break;
            case A_DMA_GRPRI:
                value &= ~0xFFFFFFE0;
                break;
            default:
                break;
        }
        s->edma_regs[(offset & 0x0F) / 4] = value;
    }
    if (0x40210000 <= offset && offset < 0x40210020) {
        switch (offset & 0xFF) {
            case A_CH0_CSR:
                if (value & (R_CH0_CSR_EBW_MASK || R_CH0_CSR_EEI_MASK || R_CH0_CSR_EARQ_MASK || R_CH0_CSR_ERQ_MASK)) {
                    qemu_log_mask(LOG_GUEST_ERROR, "DMA: Attempt to enable unimplemented feature: 0x%02lx\n", offset);
                    break;
                }
                s->chn[0][offset & 0xFF] = value & 0xC00F;
                break;
            case A_CH0_ES:
            case A_CH0_INT:
                s->chn[0][offset & 0xFF] &= ~value;
                s->edma_regs[R_DMA_INT] &= ~0x0001;
                qemu_set_irq(s->irq, 0);
                break;
            case A_CH0_SBR:
                break;
            case A_CH0_PRI:
               qemu_log_mask(LOG_GUEST_ERROR, "DMA: Attempt to enable unimplemented feature: 0x%02lx\n", offset);
               break;
            default:
                s->chn[0][offset & 0xFF] = value;
        }
    } else {
        switch ((offset & 0xFF) - 0x20) {
            case A_TCD0_CSR:
                if (value & R_TCD0_CSR_START_MASK)
                    s32k358_dma_run(s);
            /* fallthrough */
            default:
                s->tcd[0][(offset & 0xFF) - 0x20] = value;
        }
    }
}

static const MemoryRegionOps s32k358_dma_ops = {
    .read = s32k358_dma_read,
    .write = s32k358_dma_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void s32k358_dma_realize(DeviceState *dev, Error **errp) {
    S32K358DMAState *s = S32K358_DMA(dev);
    if (!s->downstream) {
        error_setg(errp, "S32K358 downstream link not set");
        return;
    }
    address_space_init(&s->downstream_as, s->downstream, "s32k358-downstream");
}

static void s32k358_dma_instance_init(Object *obj) {
    S32K358DMAState *s = S32K358_DMA(obj);

    /* There is a discontinuity between the eDMA memory regions, so for now
     * we'll only initialize the memory map for one channel
     */
    memory_region_init_io(&s->mmio, OBJECT(s), &s32k358_dma_ops, s, "s32k358_dma", 0x8000);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);
}

static const Property s32k358_dma_properties[] = {
    DEFINE_PROP_LINK("downstream", S32K358DMAState, downstream, TYPE_MEMORY_REGION, MemoryRegion *),
};

static void s32k358_dma_class_init(ObjectClass *oc, const void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = s32k358_dma_realize;
    device_class_set_props(dc, s32k358_dma_properties);
    device_class_set_legacy_reset(dc, s32k358_dma_reset);
}

static const TypeInfo s32k358_dma_info = {
    .name = TYPE_S32K358_DMA,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K358DMAState),
    .instance_init = s32k358_dma_instance_init,
    .class_init = s32k358_dma_class_init,
};

static void s32k358_dma_register_types(void) {
    type_register_static(&s32k358_dma_info);
}

type_init(s32k358_dma_register_types)
