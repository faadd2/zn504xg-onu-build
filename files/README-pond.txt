# airoha-pond 双版本与补丁开关
#
# 本目录常驻两个二进制 + 开关脚本：
#   airoha-pond.patched  md5=0ee93a7a240c04bf248a2cdaca0f447c  size=598752
#   airoha-pond.stock    md5=9abf8905eb5aa1446974999092650239  size=592307
#   airoha-pond          = 当前生效版本（出厂默认=patched；脚本切换后变化）
#
# 补丁版来源：references/pon-fix-toolkit/patches/airoha-pond-ctc-epon-compat.patch
# （会话工作区），作用于 airoha-pond 0.3.x（ponwrt feed，2026-10-11 构建）。
# 补丁行为：未知 OAM SET 一律确认、未知 GET 返回 0。北京联通 10G EPON
# 的严格 provisioning 档案要求该行为，否则注册后永远不开数据通路
# （2026-10-15 实机对比确证：stock 四业务全挂，patched 四业务全通）。
#
# 开关机制：
#   UCI 选项 pon.general.epon_ctc_patch（1=补丁版[默认] / 0=stock）
#   切换工具 /usr/sbin/pond-patch-switch.sh（幂等，md5 判定，自动重启 pond）
#   开机自应用 /etc/init.d/pond-patch（S99，uci-defaults/99-pond-setup 启用）
#   LuCI 入口：状态→ONU→PON 诊断（luci-app-onu diagnostics 视图）
#   刷机行为：二进制回到出厂默认（patched），但 /etc/config/pon 保留——
#             若用户此前关掉开关，首次开机会自动切回 stock，无需重设。
#
# workflow 对两个二进制均做 md5 硬校验，任一不符即失败退出。
