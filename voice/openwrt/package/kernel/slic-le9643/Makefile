#
# Copyright (C) 2026 ZN504XG-D voice porting project
# This is free software, licensed under GPL-2.0.
#

define KernelPackage/slic-le9643
  SUBMENU:=Sound support
  TITLE:=Microsemi miSLIC Le9643 control driver (voice FXS)
  DEPENDS:=@TARGET_airoha +kmod-spi-dev
  KCONFIG:=
  FILES:=$(PKG_BUILD_DIR)/slic-le9643.ko
  AUTOLOAD:=$(call AutoLoad,60,slic-le9643)
endef

define KernelPackage/slic-le9643/description
  SPI control-plane driver for the Microsemi (Zarlink) miSLIC Le9643
  SLIC on ZN504XG-D. Provides sysfs status and ring control; full
  telephony feature set (BORSCHT, DTMF, cadencing) requires the
  VP-API-II user of this driver to be added later.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,slic-le9643))
