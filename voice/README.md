# ZN504XG-D 语音移植工作区（Path B：Le9643 → PCM/ZSI → baresip）

> 本目录是工作区 `devices/ZN504XG-D/voice/` 的归档副本（PAT 为 fine-grained、
> 无法推送到 PAT 授权清单之外的独立仓库，故并入本构建仓库）。
> 本地工作区与仓库副本以本地为准同步。

目标：在刷机后的 ZN504XG-D（OpenWrt）上恢复 POTS 电话业务，注册北京联通 IMS。

## 路线（2026-10-17 裁决，勿回退 Path A）

Path A（上游 ASoC AFE 200-02 + machine 驱动）已判死：AFE 手动使能后 CUR
寄存器冻结、零中断，SCU 无音频时钟路由，驱动无 clk 管理。裁决报告见
`devices/ZN504XG-D/reports/语音路线裁决-AFE死局与PCM栈迁移_20261017.md`。

现行路线 = **airoha_voip PCM/ZSI 栈**（Sirherobrine23/airoha_voip，pinned
`4222576d980856f3c9ffc323d2fd78a28f94f085`）：

```
电话机 ── 模拟线 ── Le9643 SLIC(U900) ── ZSI(控制通道复用PCM帧) ── AN7581 PCM1
                      数字音频: PCM bus slot 4 (DMA channel 0, 8kHz 16bit)
                      └── /dev/en75xx-fxs0 字符设备 ── baresip ── SIP ── voip VLAN 3010 ── 联通 IMS
```

## 组件清单与状态

| 组件 | 位置 | 状态 |
|---|---|---|
| SIP 账号（北京联通 IMS） | 已存档（PT1570 backups） | ✅ 注册 200 OK 已验证 |
| 数据通路（voip VLAN 3010） | 504 上已配置 | ✅ 就绪 |
| 驱动补丁 001（en7581 compat + le9643 别名） | `airoha-voip/patches/001-en7581-support.patch` | ✅ 已验证（纯净树 apply 通过） |
| 构建补丁 002（裁剪 ISI/ProSLIC 模块） | `airoha-voip/patches/002-skip-isi-proslic-modules.patch` | ✅ 已验证 |
| feed 包 Makefile（3 个 kmod） | `openwrt/package/en75xx-voip/Makefile` | ✅ 待 B 首次编译回填 PKG_MIRROR_HASH |
| DTS 补丁 201-06（pcm0+zsi0+slic0+pinctrl，废 201-04/05） | 本仓库 `dts-patches/` | ✅ 已验证 |
| baresip 配置 | `baresip/` | ✅ 注册已通；FXS 字符设备桥接待驱动就位后接 |
| naoki66 si3219x 专属补丁（015/016/020/030/031/040-042/111） | — | ❌ 不移植（SLIC 不同） |
| 100（start-pcm-on-activity）、110（中国振铃/硬件 DTMF） | — | 🚧 评估后可选 |

## 关键事实（调查结论，勿重复劳动）

- Le9643 = Microsemi miSLIC，属 le9641/le9642 寄存器家族（源码 docs/07 交叉验证表），
  走 **ZSI 变体**：控制通道 in-band 走 PCM 帧，不碰 flash 共享 SPI。
- pinctrl（主线 pinctrl-an7581.c 已核实）：pcm_spi=GPIO18-25、pcm_spi_cs1=GPIO43，
  与 pcm1(22-25)/pcm2(18-21) 互斥。**504 上 GPIO18/19/20/24 兼作 PON/LOS/Internet/
  phone LED**，pinmux 申请可能与 leds-gpio 冲突（probe 顺序决定谁赢）；SLIC 无应答时
  按 201-06 注释换 pcm1 组重试。
- 复位/时钟 ID（主线头文件已核实）：EN7581_PCM1_RST=35、EN7581_PCM1_ZSI_ISI_RST=29、
  EN7523_CLK_SLIC=3；PCM 中断 GIC_SPI 27（naoki66 XG2010G 同款布局）。
- IRQ 27 与 GPIO21/22/23/25/43 在 SoC/板级 dts 中无占用冲突（已 grep 核实）。
- 实机遗留验证点：IRQ 27 是否真实触发、en7581 是否需要芯片级 PCM 时钟发生器
  （en7528 有，en7581 未确认）、ZSI 读 SLIC ID 应答应答。
- 原厂固件及其语音栈在本地备份中零残留，原厂 DTB 丢失；丝印已完成（U900=Le9643）。

## B 会话编译须知

A 已完成（随 Path B 提交推送）：Path A 死补丁移出（kernel 201-01..03、dts
201-04/05）、build.yml 的 SND_SOC 四行删除、201-06 重生成自包含版（纯净树
验证通过）、lint 与登记表落地（`scripts/lint-patch-names.sh` +
`docs/patch-registry.md`，全绿）。

B 剩余三项（build-3 前）：

1. workflow 把 `voice/openwrt/package/en75xx-voip/` 拷入 `ponwrt/package/`，
   并把 `voice/airoha-voip/patches/` 拷为 `ponwrt/package/en75xx-voip/patches/`
   （OpenWrt 自动应用到下载源码）。
2. config 片段加 `CONFIG_PACKAGE_kmod-en75xx-slic-le9642=y`（依赖链自动带
   lec+pcm）。
3. 首次编译 download 阶段回填 Makefile 里的 PKG_MIRROR_HASH 占位符（预期首跑
   在 download 步失败并打印期望哈希，回填后重跑即可）。
