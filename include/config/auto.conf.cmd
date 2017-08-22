deps_config := \
	lib/Kconfig \
	drivers/chr_drv/Kconfig \
	drivers/blk_drv/Kconfig \
	drivers/Kconfig \
	mm/Kconfig \
	fs/Kconfig \
	init/Kconfig \
	arch/x86/Kconfig \
	Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(ARCH)" "x86"
include/config/auto.conf: FORCE
endif
ifneq "$(SRCARCH)" "x86"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
