CXX := ../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
CC 	:= ../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
AR 	:= ../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-ar

BUILD_TARGET = libRTL8367API.a

NETRTLFILES += acl.o cpu.o dot1x.o eee.o i2c.o igmp.o interrupt.o \
			l2.o leaky.o led.o mirror.o oam.o port.o ptp.o qos.o \
			rate.o rldp.o rtk_switch.o rtl8367c_asicdrv.o rtl8367c_asicdrv_acl.o \
			rtl8367c_asicdrv_cputag.o rtl8367c_asicdrv_dot1x.o rtl8367c_asicdrv_eav.o \
			rtl8367c_asicdrv_eee.o rtl8367c_asicdrv_fc.o rtl8367c_asicdrv_green.o \
			rtl8367c_asicdrv_hsb.o rtl8367c_asicdrv_i2c.o rtl8367c_asicdrv_igmp.o \
			rtl8367c_asicdrv_inbwctrl.o rtl8367c_asicdrv_interrupt.o rtl8367c_asicdrv_led.o \
			rtl8367c_asicdrv_lut.o rtl8367c_asicdrv_meter.o rtl8367c_asicdrv_mib.o \
			rtl8367c_asicdrv_mirror.o rtl8367c_asicdrv_misc.o rtl8367c_asicdrv_oam.o \
			rtl8367c_asicdrv_phy.o rtl8367c_asicdrv_port.o rtl8367c_asicdrv_portIsolation.o \
			rtl8367c_asicdrv_qos.o rtl8367c_asicdrv_rldp.o rtl8367c_asicdrv_rma.o \
			rtl8367c_asicdrv_scheduling.o rtl8367c_asicdrv_storm.o rtl8367c_asicdrv_svlan.o \
			rtl8367c_asicdrv_trunking.o rtl8367c_asicdrv_unknownMulticast.o rtl8367c_asicdrv_vlan.o \
			smi.o stat.o storm.o svlan.o trap.o trunk.o vlan.o
		  
EXTRA_CFLAGS += -g -Wall -Wstrict-prototypes -Dlinux -DLINUX -DMDC_MDIO_OPERATION -DRTK_X86_CLE
EXTRA_CFLAGS += -I./

%.o: %.c
	$(CC) $(EXTRA_CFLAGS) -c -o $@ $<

all: clean $(NETRTLFILES)
	$(AR) rcs $(BUILD_TARGET) $(NETRTLFILES)
	rm -rf lib
	mkdir lib
	cp $(BUILD_TARGET) ./lib/$(BUILD_TARGET)

clean:
	rm -rf *.o $(BUILD_TARGET)
