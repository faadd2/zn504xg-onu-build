# baresip 北京联通 IMS 配置（草稿）

> 注意：真实 IMS 账号属个人隐私，**不要写入本仓库**。下列账号行为占位示例，
> 实际账号从原光猫备份提取（见 `devices/PT1570-R10/backups/voip_sip_account_*.txt`，本地存档）。

## 账号

`~/.baresip/accounts`：

```
<sip:+86XXXXXXXXXXX@bj.ims.chinaunicom.cn>;auth_pass=<IMS密码>;regint=1800;outbound="sip:<代理IP>:5060;transport=udp";sipnat=outbound;ptime=20
```

北京联通 IMS 代理（运营商内网地址，经语音 VLAN 可达）：`10.203.255.17:5060` / `10.203.255.1:5060`，
主代理不可达时切换备用。

## 网络绑定（关键）

IMS 注册必须走 voip 口（VLAN 3010，10.143.218.196/23，网关 10.143.218.1），
不能走路由器默认出口。两种方式：

1. baresip 配置 `sip_listen` 不指定地址，靠路由表：到 10.203.255.17 的路由
   必须走 pon0.3010（现有 ims_route 静态路由已配）→ 源地址自动选对。
2. 保险做法：用 network namespace 或 ` LD_PRELOAD` 绑定。OpenWrt 上更简单的
   是方案 1，先用 `curl --interface pon0.3010` 之类验证源地址。

## 编解码与 DTMF（对齐原光猫 voip 配置）

- 首选 G.711A（PCMA），ptime 20ms
- DTMF：RFC2833（telephone-event PT=97，与原光猫一致），baresip 里
  `audio_sdp_...`/`#dtmf` 模块：module `event`（sipevent）+ `menu` 交互
- 回声消除：Le9643 片内无 EC，软件 EC 用 baresip `webrtc_aec` 模块

## 模块（~/.baresip/config 节选）

```
module			stdio.so
module			g711.so
module			auloop.so
module			alsa.so
audio_player		alsa,default
audio_source		alsa,default
audio_alert		alsa,default
```

（device 名待 machine 驱动注册后按 `aplay -L` 实际名字填。）

## 拨号规则

北京联通 IMS 通常允许直接拨号；长途加 0 或 00 的规则如需，
在 baresip `oe0`/`x BSTN`... 用模块 `menu` 的 dial 命令即可，
进阶可写 `mwi`/`presence` 不需要。
