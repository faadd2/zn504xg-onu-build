#!/bin/sh
# pond-patch-switch.sh —— airoha-pond 补丁开关（北京联通 10G EPON OAM 兼容）
#
# 由 UCI 选项 pon.general.epon_ctc_patch 控制（LuCI「PON 诊断」页可切换）：
#   1（默认）= 补丁版：未知 OAM SET 一律确认、未知 GET 返回 0——联通严格
#             provisioning 档案下注册/开通数据通路的必要条件
#   0       = 原厂 stock 版
#
# 两个版本常驻 /usr/sbin/airoha-pond.{patched,stock}，本脚本按选项把对应
# 版本安装为 /usr/sbin/airoha-pond 并重启 airoha-pond 服务。
# 幂等：当前二进制已匹配目标时直接退出（md5 判定，防误重启）。
# 开机自应用：/etc/init.d/pond-patch（S99，uci-defaults 启用）。
# 每次成功收敛后写 /etc/pon-fix-version 标记（LuCI 诊断页 PATCH_INFO 与
# onu-pond-patch 后端读它显示版本；build-21 前无人生成该文件，页面上版本空白）。

PATCHED_MD5=0ee93a7a240c04bf248a2cdaca0f447c
STOCK_MD5=9abf8905eb5aa1446974999092650239
ROOT=/usr/sbin
MARKER=/etc/pon-fix-version

write_marker() {
    # $1=variant $2=md5
    echo "luci-app-onu pond switch; variant=$1; md5=$2; updated=$(date -Iseconds)" > "$MARKER"
}

enable="$(uci -q get pon.general.epon_ctc_patch)"
enable="${enable:-1}"

cur="$(md5sum "$ROOT/airoha-pond" 2>/dev/null | awk '{print $1}')"

if [ "$enable" = "1" ]; then
    target=patched; target_md5=$PATCHED_MD5
else
    target=stock; target_md5=$STOCK_MD5
fi

if [ "$cur" = "$target_md5" ]; then
    [ -f "$MARKER" ] || write_marker "$target" "$cur"
    echo "pond-patch: already $target (md5 $cur), no action"
    exit 0
fi

if [ ! -f "$ROOT/airoha-pond.$target" ]; then
    echo "pond-patch: ERROR missing $ROOT/airoha-pond.$target" >&2
    exit 1
fi

cp "$ROOT/airoha-pond.$target" "$ROOT/airoha-pond"
chmod 755 "$ROOT/airoha-pond"
new_md5="$(md5sum "$ROOT/airoha-pond" | awk '{print $1}')"
if [ "$new_md5" != "$target_md5" ]; then
    echo "pond-patch: ERROR install verification failed (got $new_md5)" >&2
    exit 1
fi

echo "pond-patch: switched to $target (md5 $new_md5), restarting airoha-pond"
/etc/init.d/airoha-pond restart
write_marker "$target" "$new_md5"
exit 0
