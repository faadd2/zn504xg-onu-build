# libmicroxml — easycwmp 的 XML 解析依赖（pivasoftware/microxml，mxml 精简 fork）。
# easycwmp.org 下载已失效，改用 GitHub master 分支归档。

include $(TOPDIR)/rules.mk

PKG_NAME:=libmicroxml
PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_SOURCE:=master.tar.gz
PKG_SOURCE_URL:=https://github.com/pivasoftware/microxml/archive/refs/heads
PKG_HASH:=skip

PKG_BUILD_DIR:=$(BUILD_DIR)/microxml-master

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

# microxml 的 Makefile.in 安装目标用 $(BUILDROOT)$(libdir)，不吃 OpenWrt 默认传的
# DESTDIR（run 38557167777/38568491129/38576535281 三跑实证：make install 直接写
# /usr/lib 权限拒绝）。关键坑：MAKE_FLAGS/MAKE_INSTALL_FLAGS 的默认值定义在
# package-defaults.mk，而它是被 package.mk 在 182 行（文件末尾）才 include 的——
# 赋值写在本文件 include package.mk 之前会被 `MAKE_FLAGS =` 整体覆盖（第五跑实证：
# 编译/安装命令行里 BUILDROOT 完全不出现）。所以必须放在 include 之后。
MAKE_FLAGS += BUILDROOT="$(PKG_INSTALL_DIR)"
MAKE_INSTALL_FLAGS += BUILDROOT="$(PKG_INSTALL_DIR)"

define Package/libmicroxml
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Micro XML library (minimal mxml fork)
  URL:=https://github.com/pivasoftware/microxml
  ABI_VERSION:=1
endef

define Package/libmicroxml/description
 A minimal XML parsing library, fork of mxml, used by easycwmp.
endef

TARGET_CFLAGS += -D_GNU_SOURCE

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/microxml.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmicroxml.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/microxml.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libmicroxml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmicroxml.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmicroxml))
