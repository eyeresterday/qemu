#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/error-report.h"
#include "hw/arm/s32k358_soc.h"
#include "hw/arm/boot.h"

//#define SYSCLK_FRQ 240000000ULL
#define  XTAL (50000000UL)
#define  SYSCLK_FRQ (XTAL / 2U)

static void s32k3x8evb_init(MachineState *machine)
{
	DeviceState *dev;
	Clock *sysclk;

  	/* This clock doesn't need migration because it is fixed-frequency */
	sysclk = clock_new(OBJECT(machine), "SYSCLK");
	clock_set_hz(sysclk, SYSCLK_FRQ);

	dev = qdev_new(TYPE_S32K358_SOC);
	object_property_add_child(OBJECT(machine), "soc", OBJECT(dev));
	qdev_connect_clock_in(dev, "sysclk", sysclk);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

	armv7m_load_kernel(ARM_CPU(first_cpu), machine->kernel_filename, S32K358_PFLASH_BASE, S32K358_PFLASH_LENGTH);
}

static void s32k3x8evb_machine_init(MachineClass *mc)
{
	static const char * const valid_cpu_types[] = {
		ARM_CPU_TYPE_NAME("cortex-m7"),
        NULL
	};

	mc->desc = "S32K3X8EVB Machine (Cortex-M7)";
	mc->init = s32k3x8evb_init;
	mc->valid_cpu_types = valid_cpu_types;
    //mc->ignore_memory_transaction_failures = true;
}

DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init);
