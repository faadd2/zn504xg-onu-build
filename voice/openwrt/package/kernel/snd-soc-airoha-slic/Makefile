#
# Copyright (C) 2026 ZN504XG-D voice porting project
# GPL-2.0
#

define KernelPackage/snd-soc-airoha-slic
  SUBMENU:=Sound support
  TITLE:=AN7581 AFE <-> Le9643 SLIC ASoC machine driver
  DEPENDS:=@TARGET_airoha +kmod-sound-soc-core
  KCONFIG:=
  FILES:=$(PKG_BUILD_DIR)/airoha-slic.ko
  AUTOLOAD:=$(call AutoLoad,62,airoha-slic)
endef

define KernelPackage/snd-soc-airoha-slic/description
  ASoC machine driver wiring the Airoha AN7581 AFE CPU DAI to the
  Le9643 SLIC PCM interface on ZN504XG-D.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,snd-soc-airoha-slic))
