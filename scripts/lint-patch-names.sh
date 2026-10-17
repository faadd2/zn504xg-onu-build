#!/usr/bin/env bash
# 补丁编号 lint（命名约定方案1，SESSION-SYNC 10-17 19:50 条目）
#
# 规则：
#   kernel-patches/  进内核 series，必须排在 ponwrt 上游 930 之后 → 只用 9[4-9]x 区段
#   dts-patches/     打在 OpenWrt 目标树（target dts 不进内核树），独立 series → 只用 2xx 区段
#                    （dts 历史编号为两级 201-NN，编号整体取 "201-NN" 判重）
#   每个区段内编号不复用；文件名格式 <编号>-<英文短横线主题>.patch
#
# 用法：scripts/lint-patch-names.sh   （在仓库根目录运行，退出码非 0 = 有违规）
set -u

fail=0

# 从文件名提取编号：最前面的数字段，dts 允许 201-04 这种两级号
num_of() {
    local base="$1"
    if [[ "$base" =~ ^([0-9]+(-[0-9]+)?)- ]]; then
        echo "${BASH_REMATCH[1]}"
    fi
}

lint_dir() {
    local dir="$1" regex="$2" desc="$3"
    if [ ! -d "$dir" ]; then
        echo "WARN: 目录不存在: $dir"
        return
    fi
    local seen="" f base num
    for f in "$dir"/*.patch; do
        [ -e "$f" ] || continue
        base="$(basename "$f")"
        if ! [[ "$base" =~ ^${regex}(-[0-9]+)?-.+\.patch$ ]]; then
            echo "FAIL: $dir/$base  不符合 ${desc}"
            fail=1
            continue
        fi
        num="$(num_of "$base")"
        if [ -n "$seen" ] && printf '%s\n' "$seen" | grep -qx "$num"; then
            echo "FAIL: $dir/$base  编号 $num 重复"
            fail=1
        fi
        seen="$seen
$num"
    done
}

lint_dir kernel-patches '9[4-9][0-9]' '94x-99x 区段（kernel series 须排 ponwrt 930 之后）'
lint_dir dts-patches   '2[0-9][0-9]' '2xx 区段（201-NN 两级号合法）'

# 跨目录同号提示（不判失败；每个同号组合只报一次）
knums="$(for f in kernel-patches/*.patch; do [ -e "$f" ] && num_of "$(basename "$f")"; done)"
for g in dts-patches/*.patch; do
    [ -e "$g" ] || continue
    dnum="$(num_of "$(basename "$g")")"
    if printf '%s\n' "$knums" | grep -qx "$dnum"; then
        echo "WARN: kernel-patches 与 dts-patches 同号 $dnum（两 series 合法，日志引用请带目录名）"
    fi
done

if [ "$fail" -eq 0 ]; then
    echo "OK: 补丁编号全部符合约定"
else
    echo "存在违规，登记表与规则见 docs/patch-registry.md"
fi
exit "$fail"
