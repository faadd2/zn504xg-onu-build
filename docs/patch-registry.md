# 补丁编号登记表（命名约定方案1，2026-10-17 起生效）

## 规则

| 目录 | 打入位置 | 编号区段 | 理由 |
|---|---|---|---|
| `kernel-patches/` | 内核 source series（`target/linux/airoha/patches-6.18/`） | **940–999** | 必须排在 ponwrt 上游 930 之后，否则 quilt 中途应用必 FAILED（build-25 实证） |
| `dts-patches/` | OpenWrt 目标树（`target/linux/airoha/dts/`，DEVICE_DTS_DIR 不进内核树） | **200–299** | 独立 series，与内核编号无关 |

- 区段内编号**单调递增、不复用**；废弃补丁标记"废止"并移出目录，不把号留给新补丁。
- 文件名格式：`<编号>-<英文短横线主题>.patch`。
- 两目录**允许同号**（series 独立），但日志/同步文档引用时必须带目录名，避免歧义；lint 会对同号发 WARN。
- 提交前自检：`bash scripts/lint-patch-names.sh`（CI 接入待议）。

## kernel-patches（94x 区段）

| 编号 | 主题 | 状态 |
|---|---|---|
| 940 | PPE bind 诊断插桩 v2（33 处锚点，20106-ppe: 前缀日志） | 现行（build-3 实机验证通过） |
| 941 | PPE insert 方向探针（orientation probe：mismatch 双向 dump + REVERSED match 试算 + bucket empty 分离计数，B 侧 `1d19ea2` 产物） | 现行（build-3 已带，实机日志实证） |
| 942 | PPE insert tuple 审计（20107: add/hw 双向 dump，补 941 的桶空盲区；上下文与 941 不相交，quilt 940→941→942 顺序应用双路验证通过） | 现行（待 build-4 编译实机） |
| ~~201-01~~ | ~~AFE dt-binding~~ | 已移出（10-17 Path A 清理） |
| ~~201-02~~ | ~~wm8960 dt-binding~~ | 已移出（同上） |
| ~~201-03~~ | ~~le9643 machine 驱动~~ | 已移出（同上；build.yml 的 SND_SOC 四行同步删除） |

## dts-patches（2xx 区段）

| 编号 | 主题 | 状态 |
|---|---|---|
| ~~201-04~~ | ~~sound-slic 根级节点 + AFE 使能~~ | 已移出（10-17 Path A 清理） |
| ~~201-05~~ | ~~AFE reset 接线（EN7581_I2S_RST）~~ | 已移出（同上） |
| 201-06 | Path B 语音：pcm0+zsi0+slic0(Le9643)+pinctrl，自包含（取代 201-04/05） | 现行 |
