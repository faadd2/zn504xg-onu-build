#
# EN75xx voice stack (PCM/TDM + ZSI + Le9642/43 FXS) for ZN504XG-D.
#
# Out-of-tree kmod feed wrapping Sirherobrine23/airoha_voip at a pinned
# commit.  Only the modules the ZN504XG-D needs are packaged:
#
#   kmod-en75xx-lec          OSLEC line echo canceller
#   kmod-en75xx-pcm          PCM/TDM engine + /dev/en75xx-fxsN char device
#   kmod-en75xx-slic-le9642  ZSI transport + Microsemi Le9641/42/43 driver
#
# The ISI/ProSLIC modules (en75xx-isi-spi, en75xx-slic-si3219x and the
# vendor/proslic tree) are trimmed by patches/002: they target Si3219x
# boards and only add build risk here.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=en75xx-voip
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=ZN504XG-D voice porting project

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Sirherobrine23/airoha_voip
# Pinned, do not float: en7581 compat additions live in ./patches.
PKG_SOURCE_VERSION:=4222576d980856f3c9ffc323d2fd78a28f94f085
# Hash 实证来源：voice-pkg 快检 run 39034044216 download 步打印
# （2026-10-17，zarq 官方流程；改 PKG_SOURCE_VERSION 必须重取）。
PKG_MIRROR_HASH:=101b017ee9f909491825f9a2cd2799f69120c6d765767412bb6d78d8397faf5a

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/en75xx-lec
  SUBMENU:=Voice over IP
  TITLE:=OSLEC line echo canceller (EN75xx voice)
  DEPENDS:=@TARGET_airoha
  FILES:=$(PKG_BUILD_DIR)/src/en75xx-lec.ko
  AUTOLOAD:=$(call AutoLoad,49,en75xx-lec)
endef

define KernelPackage/en75xx-pcm
  SUBMENU:=Voice over IP
  TITLE:=Airoha EN75xx PCM/TDM controller + FXS char device
  DEPENDS:=@TARGET_airoha +kmod-en75xx-lec
  FILES:= \
	$(PKG_BUILD_DIR)/src/en75xx-pcm.ko \
	$(PKG_BUILD_DIR)/src/en75xx-voice.ko
  AUTOLOAD:=$(call AutoLoad,50,en75xx-pcm en75xx-voice)
endef

define KernelPackage/en75xx-pcm/description
 TDM engine behind the FXS ports, plus the /dev/en75xx-fxsN character
 device carrying 8 kHz 16-bit linear audio to userspace (baresip bridges
 it to SIP).
endef

define KernelPackage/en75xx-slic-le9642
  SUBMENU:=Voice over IP
  TITLE:=Microsemi Le9641/Le9642/Le9643 FXS (ZSI)
  DEPENDS:=@TARGET_airoha +kmod-en75xx-pcm
  FILES:= \
	$(PKG_BUILD_DIR)/src/en75xx-zsi.ko \
	$(PKG_BUILD_DIR)/src/en75xx-slic-le9642.ko
  AUTOLOAD:=$(call AutoLoad,51,en75xx-zsi en75xx-slic-le9642)
endef

define KernelPackage/en75xx-slic-le9642/description
 Le9641/Le9642/Le9643 miSLIC driven over ZSI (control channel multiplexed
 onto the PCM pins).  Loads the OEM raw MPI profiles, runs the codec in
 G.711 and exposes hook, ring and line feed.  The ZN504XG-D wires its
 Le9643 to pcm_spi + pcm_spi_cs1 (see dts-patches/201-06).
endef

# The repo layout already is src/ + include/ + vendor/, and the upstream
# src/Makefile resolves its ../include and ../vendor paths against that, so
# the default extract is all Prepare needs.
define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/src modules
endef

$(eval $(call KernelPackage,en75xx-lec))
$(eval $(call KernelPackage,en75xx-pcm))
$(eval $(call KernelPackage,en75xx-slic-le9642))
