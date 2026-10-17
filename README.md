# ZN504XG-D OpenWrt 固件

面向 **ZNXT ZN504XG-D**（Airoha AN7581，10G-EPON 光猫 + 路由一体机，带电话口）的
PonWrt 定制固件。在 [pbs05/ponwrt](https://github.com/pbs05/ponwrt) 基础上叠加本仓库的
管理界面套件、驱动补丁与默认配置，实现"光猫 + 路由器"一体：
PON 注册认证、四业务拨号（上网 / IPTV / 语音 / TR-069 管理通道）全部在设备本机完成。

## 固件信息

| 项 | 说明 |
|---|---|
| 目标设备 | ZNXT ZN504XG-D（主）；Gemtek XG2010G（AN7581 同平台，随构建产出） |
| 基线 | PonWrt（ImmortalWrt SNAPSHOT / OpenWrt 主线 airoha target） |
| 内核 | 6.18 LTS 系列（随上游） |
| 变体 | `onu-suite`（完整版，推荐）/ `stock`（不注入自定义包的对照版） |
| 镜像格式 | `znxt_zn504xg-d-squashfs-sysupgrade.itb`（FIT 签名，sysupgrade 直刷） |

## 相对原版 PonWrt 的改进

**管理界面（luci-app-onu 超级 ONU 套件）**
- PON 状态 / 光链路（光功率、FEC 误码、趋势图）/ 认证配置 / 硬件身份 / 事件日志 / 光路诊断
- 上网 / IPTV / 语音 三条业务线路的独立配置页
- ACS 管理页（TR-069 通道状态、参数存档、7547/9090 原始抓包）
- 配置迁移向导：上传原厂光猫备份（ctromfile 等）自动解析并迁移上网/IPTV/语音配置
- 芯片状态面板（NPU / Frame Engine / PPE 条目，端口标签按 504 板级布局适配）
- 网速测试（speedcheck / netspeedtest）

**线路与驱动**
- 北京联通 OAM 补丁（airoha-pond 双版本 + 开关化）：未知 SET 一律确认、未知 GET 返回 0，
  联通严格 provisioning 档案下注册/开链路的必需项；LuCI 可一键切换原厂/补丁版
- 语音内核补丁（201-01..05）：AN7581 AFE + LE9643 SLIC 声卡驱动与 PCM 双工通路
  （build-22 起实机验证通过：`an7581-le9643` 声卡 + playback/capture PCM）
- EN8811H 2.5G PHY 驱动与固件（lan1 2.5G 口）
- ppp 固定 2.5.1（规避 2.5.4 的 ppp-up 子脚本崩溃回归）
- pond-watchdog：pppd 子脚本异常时的拨号状态兜底

**系统默认值**
- 首启 CPU governor = ondemand（powersave@500MHz 会把路由压到 ~250M）
- 防火墙 flow offloading（软 + 硬）默认开启
- 失效 apk 软件源自动屏蔽

## 部署须知

1. **刷机**：`sysupgrade -v /tmp/xxx-sysupgrade.itb`（保留配置）；出厂/uboot 救砖走
   192.168.0.1 的 u-boot 恢复页。刷前务必备份 `/etc/config` 与 crontab。
2. **首启**：系统默认 root 无密码，请立即设置强密码再接入不可信网络。
3. **⚠ 已知内核缺陷（务必阅读）**：本机 `ip link set eth0 down` 及任何触发 eth0
   拆除的操作（`network restart`、整体网络重建）会**整机无声挂死**（无 oops，
   只能断电恢复）。日常运维请用单接口 `ifdown/ifup`，切勿 `network restart`；
   远程操作前确认有物理断电条件。上游 issue 草稿与证据链见
   `reports/upstream-issue-eth0-teardown-hang.md`。
4. **线路配置**：默认面向北京联通 10G-EPON（补丁开关默认开）。其他省份/运营商需
   自行核对 VLAN ID 与 LOID 认证方式（LuCI → ONU → PON 诊断页有补丁开关与状态）。
5. **已知限制**：本机 PPPoE 拨号场景 PPE 硬件转发不生效（上游 airoha_eth 缺陷，
   见 pbs05/ponwrt#41），500M 宽带实测约 590/22 Mbps 为软件转发天花板；
   桥接/透传模式不受影响。

## 分享/转发注意事项

- **运营商基准**：以北京联通 10G-EPON 为验证基准，其他环境参见上文第 4 条。
- **许可证**：镜像内含 Airoha 专有 NPU 固件、EN8811H PHY 固件及修改过的 airoha-pond
  二进制，闭源组件的分发无官方授权，仅供个人研究使用；OpenWrt/GPL 部分以相应许可证为准。

## 项目结构

```
files/            叠加层：pond 双版本二进制 + 补丁切换脚本 + watchdog + 首启钩子
kernel-patches/   内核补丁（语音 ASoC 201-01..03）
dts-patches/      设备树补丁（语音 201-04/201-05）
voice/            语音驱动开发树
feed-easycwmp/    easycwmp 单包 CI（CWMP 客户端备用，不进固件）
.github/workflows/ 云编译流水线（manifest 硬门禁：EN8811H/迁移包/ppp 版本）
```

## 代码来源

- [pbs05/ponwrt](https://github.com/pbs05/ponwrt) —— 固件基线（PonWrt / airoha target）
- [pbs05/openwrt-pon-userspace](https://github.com/pbs05/openwrt-pon-userspace) ——
  PON 用户态（ponctl/pond/pon-debug）与 luci-app-pon（本套件的重构蓝本，保留其上游历史）
- [naoki66/openwrt-pon-userspace](https://github.com/naoki66/openwrt-pon-userspace)、
  [naoki66/ImmortalWrt-for-Gemtek-brightspeed](https://github.com/naoki66) ——
  PON 驱动 feed 与语音 PCM 链路参考
- [luanmuc/luci-app-airoha-npu](https://github.com/luanmuc) —— 芯片状态面板（中文改进 fork）
- [pivasoftware/easycwmp](https://github.com/pivasoftware/easycwmp) —— TR-069 客户端
- OpenWrt / ImmortalWrt —— 构建系统与软件源

## 鸣谢

- **pbs05** —— PonWrt 与 airoha 补丁系列，本项目的直接上游
- **naoki66** —— AN7581 PON 数据通路驱动的先行工作
- **huladabang / pbs05 社区及恩山 right.com.cn 的群友** —— ONU 厂商标识（YHTC 等）、
  联通 provisioning 行为等一线情报
- **mtk_eth_soc 维护者** —— PPPoE 硬件转发的现成范式（airoha PPE 修复的参考实现）

## 版权声明

GPL-2.0（OpenWrt 及衍生代码）；闭源固件 blob（NPU / PHY / pond 二进制）归属各自厂商，
仅随镜像提供运行所需，不随本仓库授予任何额外权利。本项目与 ZNXT、Airoha、中国联通
无任何隶属或授权关系，"联通"相关字样仅描述兼容性验证环境。
