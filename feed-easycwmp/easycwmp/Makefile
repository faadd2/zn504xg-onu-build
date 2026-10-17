# Copyright (C) 2012-2016 PIVA Software <www.pivasoftware.com>
# OpenWrt package Makefile — 取自 easycwmp 源码 ext/openwrt/build/Makefile，
# 仅改源码获取方式：easycwmp.org 现只提供 JS 单页应用（下载链接全部失效），
# 改用 pivasoftware/easycwmp GitHub 标签归档。

include $(TOPDIR)/rules.mk

PKG_NAME:=easycwmp
PKG_VERSION:=1.8.6
PKG_RELEASE:=1
PKG_SOURCE:=EasyCwmp-1.8.6.tar.gz
PKG_SOURCE_URL:=https://github.com/pivasoftware/easycwmp/archive/refs/tags
PKG_HASH:=skip


PKG_CONFIG_DEPENDS:= \
	CONFIG_EASYCWMP_DEBUG \
	CONFIG_EASYCWMP_DEVEL

PKG_BUILD_DIR:=$(BUILD_DIR)/easycwmp-EasyCwmp-1.8.6

include $(INCLUDE_DIR)/package.mk

define Package/easycwmp
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CWMP client (using libcurl)
  DEPENDS:=+libubus +libuci +libubox +libmicroxml +libjson-c +libcurl +curl
endef

define Package/easycwmp/description
 An open source implementation of the client CWMP (TR-069) protocol
endef

define Package/easycwmp/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-fcommon

# -fcommon 原因：gcc 10 起默认 -fno-common，2016 年代码把变量定义在头文件里
# （event_code_array 在 backup/config/cwmp/easycwmp/http/json/ubus/xml 九个编译单元
# 重复定义），run 38656190474 链接报 multiple definition。

TARGET_LDFLAGS += \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

CONFIGURE_ARGS += \
	--with-uci-include-path=$(STAGING_DIR)/usr/include \
	--with-libubox-include-path=$(STAGING_DIR)/usr/include \
	--with-libubus-include-path=$(STAGING_DIR)/usr/include

# ponwrt（新 OpenWrt）的 PKG_FIXUP:=autoreconf 对纯 autoconf 工程（microxml）有效，
# 对 automake 工程（本包）不可靠：run 38589353232 实证 autoreconf 未产出 configure，
# 而 Build/Configure/Default 对缺失的 configure 是静默跳过（if [ -x $(CONFIGURE_CMD) ]），
# 最终编译报 no makefile found。改在 Build/Prepare 里显式 autoreconf -fi -v，
# 成败都在日志里可见，不再依赖 fixup 的隐式行为。
define Build/Prepare
	$(call Build/Prepare/Default)
	( cd $(PKG_BUILD_DIR) && autoreconf -f -i -v )
endef

ifeq ($(CONFIG_EASYCWMP_DEBUG),y)
CONFIGURE_ARGS += \
	--enable-debug
endif

ifeq ($(CONFIG_EASYCWMP_DEVEL),y)
CONFIGURE_ARGS += \
	--enable-devel
endif

ifeq ($(CONFIG_EASYCWMP_BACKUP_DATA_CONFIG),y)
CONFIGURE_ARGS += \
	--enable-backupdatainconfig
endif

CONFIGURE_ARGS += \
	--enable-jsonc=1

define Package/easycwmp/conffiles
/etc/config/easycwmp
/usr/share/easycwmp/defaults
endef

define Package/easycwmp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/easycwmpd $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/ext/openwrt/config/easycwmp $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/init.d/easycwmpd $(1)/etc/init.d
ifeq ($(EASYCWMP_BACKUP_DATA_FILE),y)
	$(INSTALL_DIR) $(1)/etc/easycwmp
endif
ifeq ($(CONFIG_EASYCWMP_SCRIPTS_FULL),y)
	$(INSTALL_DIR) $(1)/usr/share/easycwmp/functions/
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/defaults $(1)/usr/share/easycwmp
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/common/* $(1)/usr/share/easycwmp/functions/
ifeq ($(CONFIG_EASYCWMP_DATA_MODEL_TR181),y)
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/tr181/* $(1)/usr/share/easycwmp/functions/
else
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/tr098/* $(1)/usr/share/easycwmp/functions/
endif
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/scripts/easycwmp.sh $(1)/usr/sbin/easycwmp
	chmod +x $(1)/usr/share/easycwmp/functions/*
else
	$(INSTALL_DIR) $(1)/usr/share/easycwmp/functions/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/common/ipping_launch $(1)/usr/share/easycwmp/functions/ipping_launch
endif
endef

$(eval $(call BuildPackage,easycwmp))
