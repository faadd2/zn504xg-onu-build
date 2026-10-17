# 语音内核补丁 · 编译验证交接（给编译会话）

> 任务来源：主会话（语音移植）→ 编译会话（在 faadd2 账号下编译，faadd22 Actions 解禁前）
> 补丁作者：主会话。问题先找 `devices/ZN504XG-D/voice/` 工作区（本地独立 git 仓库）。

## 背景一句话

为 ZN504XG-D 移植电话语音（Le9643 SLIC → AN7581 AFE → baresip），第一步是让 AFE 声卡在内核里注册。**补丁已按 ponwrt master 上游 `200-02-ASoC-airoha-Add-AFE-and-I2S-driver-for-Airoha-AN7581.patch` 重构**（2026-10-14，本会话）：AFE/I2S 驱动本体改由上游提供，本地只保留增量补丁，并已在 6.18.55 真实源文件 + ponwrt master dtsi 上逐一 `git apply` 验证通过。

## 补丁清单（当前有效）

| 补丁 | 作用 | 去向 |
|---|---|---|
| 上游 `200-02`（ponwrt 自带） | AFE + I2S(ETDM) 驱动 + wm8960 machine，新建 `sound/soc/mediatek/an7581/` | 内核 series，无需注入 |
| `kernel-patches/201-01-dt-binding-airoha-an7581-afe.patch` | AFE dt-binding YAML（6.18 主线未含，已核 404） | 内核 series |
| `kernel-patches/201-02-dt-binding-airoha-an7581-wm8960.patch` | wm8960 dt-binding YAML | 内核 series |
| `kernel-patches/201-03-an7581-le9643-slic-machine.patch` | Le9643 machine 驱动（codec 暂用 snd-soc-dummy），DAI 名 DL1/UL1/ETDM 与上游 200-02 一致 | 内核 series，排 200-02 之后 |
| `dts-patches/201-04-arm64-dts-an7581-zn504xg-d-voice.patch` | DTS：`sound-slic` 根级节点 + `&afe status=okay` | **打 OpenWrt 目标树**（workflow 独立步骤 `git -C ponwrt apply`） |

**已删除**：`v6-00641-004-afe-driver.patch`（与上游 200-02 重复，上游还多带 wm8960）；
`v6-00699-003-mtk-common-irq-regmap.patch`（上游合并版驱动只用全局 `afe->regmap`，grep 全文确认无 per-irq regmap 用法，不需要）。

workflow 注入步骤保留 `voice_kernel_patches == 'yes'` 开关；注意现在有两个步骤：内核 series 注入 + dts-patches 目标树应用。

## 你要做的事

1. **构建**：dispatch build.yml 并设 `voice_kernel_patches=yes`。onu-frag.config 已加 `CONFIG_SOUND/SND/SND_SOC/SND_SOC_AN7581/SND_SOC_AN7581_LE9643=y`。
2. **看补丁应用日志**：补丁已按 6.18.55 与 ponwrt master 精确上下文验证，**出现 fuzz/reject = 上游又漂移了，停下来找主会话**。
3. **内核 config 确认**（重点，上次踩过）：frag 里的 `CONFIG_SND_*` 不保证进主 `.config`（内核符号在 target 编译期才合并）。构建日志里 grep：
   ```
   grep -E "CONFIG_SND_SOC_AN7581|CONFIG_SOUND|CONFIG_SND_SOC " build_log
   ```
   若 `CONFIG_SND_SOC_AN7581=m/y` 没出现，说明 symbol 没进内核 config——别烧机，直接找主会话。
4. **烧机验证**（ZN504XG-D，192.168.1.1，root）：
   ```sh
   dmesg | grep -iE "asoc|snd|sound|an7581|afe"
   ls /proc/asound/cards /sys/class/sound/ 2>/dev/null
   zcat /proc/config.gz 2>/dev/null | grep SND_SOC_AN7581   # 内核 config 二次确认
   ```
5. **成功标准（分两级）**：
   - ✅ 最低通过：内核编译完成 + dmesg 出现 an7581 AFE / snd-soc 相关 probe 日志、无 oops/panic
   - 🎯 完全通过：`/proc/asound/cards` 出现 `an7581-le9643` 声卡（codec 是 snd-soc-dummy 占位，没有真实录音功能是正常的，别当 bug 报）
6. **回报**：构建链接 + 补丁应用摘要 + config 确认输出 + dmesg 相关片段，写进会话记录或 `devices/ZN504XG-D/voice/` 下的验证笔记。

## 已知风险（按可能性排序）

1. **上游再漂移**：验证基准是今天的 ponwrt master + 6.18.55；上游更新后 200-02 内容可能变化。
2. **依赖缺失**：上游 AFE 驱动依赖 mainline `sound/soc/mediatek/common`（mtk-base-afe 框架）；ponwrt 若裁剪会编译报错。
3. **DTS 不匹配**：201-04 基于今日 ponwrt master 的 `an7581-znxt-zn50xg-d-common.dtsi`（403 行版本）。
4. `SND_SOC_AN7581_LE9643` 是 201-03 自造 config，menuconfig 里看不到属正常（defconfig/frag 直写）。

## 不要做的事

- 不要把 dts-patches/ 的补丁拷进内核 series（target dts 不进内核源码树，`DEVICE_DTS_DIR := ../dts`，补丁会 reject）。
- 不要应用 `voice/kernel-patches/reference-not-applied/` 里的 wm8960 machine（005）——那是参考，不进构建。
- 不要为了让编译过而删掉任一 201-* 补丁后静默继续（可以删，但必须知会主会话）。
- 烧机回退：sysupgrade 保留配置刷回上一版固件即可，504 当前 PPPoE 配置在 /etc/config，备份好再刷。
