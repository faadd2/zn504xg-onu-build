# ZN504XG-D 语音移植工作区（Le9643 SLIC → AN7581 AFE → baresip）

> 本目录是工作区 `devices/ZN504XG-D/voice/` 的归档副本（PAT 为 fine-grained、
> 无法推送到 PAT 授权清单之外的独立仓库，故并入本构建仓库）。
> **编译实际使用的 6 个内核补丁在本仓库根目录 `kernel-patches/`**（见 commit 1aa2f42），
> 本目录下 `kernel-patches/` 只存放不参与构建的参考补丁。本地工作区与仓库副本以本地为准同步。

目标：在刷机后的 ZN504XG-D（OpenWrt）上恢复 POTS 电话业务，注册北京联通 IMS。

## 架构

```
电话机 ── 模拟线 ── Le9643 SLIC ── PCM(I2S/ETDM) ── AN7581 AFE ── ALSA ── baresip ── SIP ── pon0.3010 (voip VLAN) ── 北京联通 IMS
                      │
                      SPI（控制面：初始化/BORSCHT/振铃/摘挂机检测/DTMF）
                      │
              slic-le9643.ko（本工作区，SPI 控制驱动）
```

## 组件清单与状态

| 组件 | 位置 | 状态 |
|---|---|---|
| SIP 账号（北京联通 IMS） | 已取得，存 `../../PT1570-R10/backups/voip_sip_account_20261012.txt` | ✅ 就绪 |
| 数据通路（voip VLAN 3010，10.143.218.196/23） | 504 上已配置 | ✅ 就绪 |
| slic-le9643 控制驱动骨架 | `openwrt/package/kernel/slic-le9643/` | 🚧 骨架，SPI 时序待 VP-API/数据手册 |
| AFE PCM 驱动 | **主线方案**：上游 ponwrt 已合并 Christian Marangi 的 ASoC AN7581 AFE/I2S 驱动（`200-02`），本地不再重复携带 | ✅ 由上游提供 |
| machine 驱动（an7581-le9643） | `kernel-patches/201-03-an7581-le9643-slic-machine.patch`（基于上游 200-02 重建，codec 暂用 snd-soc-dummy） | ✅ 已验证待编译 |
| DTS 补丁（使能 AFE + sound-slic 根级节点） | `dts-patches/201-04-arm64-dts-an7581-zn504xg-d-voice.patch`（打目标树，非内核 series） | ✅ 已验证待编译（SLIC SPI/GPIO 节点待丝印确认后补） |
| 云编译集成 | `zn504xg-onu-build/kernel-patches/` + workflow 注入步骤 + onu-frag.config 内核选项 | ✅ 已提交待 CI |
| baresip 配置 | `baresip/` | 🚧 草稿 |
| VP-API-II 源码 | 无 | ❌ 待获取（Microchip SLA / Lite，或 bcm963xx GPL dump） |

## 关键事实（调查结论，勿重复劳动）

- Le9643 = Microsemi miSLIC（非 Lantiq），官方驱动 VP-API-II（C 源码），Linux 主线无驱动。
- naoki66/airoha_sdk 有 SoC 侧 AFE 驱动参考（i2s-afe-pcm.c，MediaTek ETDM 架构，2859 行已存 `openwrt/package/kernel/airoha-afe-pcm/src/`）。
- naoki66/immortalwrt_pon 的 ZN504XG-D DTS 无语音节点（只有电话 LED GPIO24）。
- 原厂固件及其语音栈在本地备份中零残留（256MB NAND 扫描确认），原厂 DTB 丢失。
- 504 当前内核 6.18.52 无任何 sound/ASoC 模块；spi 只有 spi0.0（NAND）；i2c0 上 0x50=校准 EEPROM、0x51=EN7572。
- 控制面备选：若 AN7581 无第二路硬件 SPI，可用 spi-gpio 位 bang（Le9643 SPI 时钟要求不高）。
- 参考：主线 `sound/soc/mediatek/common`（mtk-base-afe 框架，mainline 已有）是 AFE 移植的依赖基础。

## 建议实施顺序

1. 获取 VP-API-II（Microchip 官网申请 Lite 版，或挖 Zyxel/Netgear bcm963xx GPL 包）。
2. 移植 airoha-afe-pcm（依赖 mtk-base-afe；先让 `aplay/arecord` 在 AFE 上跑通 loopback）。
3. 填 slic-le9643 的 SPI/HBI 时序（VP-API 提供），验证摘挂机/振铃。
4. machine 驱动接通 AFE↔codec，baresip 注册 IMS，整机联调。
