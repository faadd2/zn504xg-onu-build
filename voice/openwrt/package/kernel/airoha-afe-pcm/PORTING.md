# AN7581 AFE PCM 驱动移植说明

## 参考源码

`src/` 下的文件取自 naoki66/airoha_sdk（`linux/sound/soc/econet/`，Econet 原始实现）：

- `i2s-afe-pcm.c`（1570 行）：AFE 平台驱动——ETDM 收发配置、DMA 分配、FE/BE DAI 注册
- `i2s-afe-common.h` / `i2s-base-afe.h` / `i2s-afe-platform-driver.h` / `i2s-afe-fe-dai.h`：mtk-base-afe 风格的公共结构
- `i2s-evb.c`（264 行）：EVB 机器驱动（DAI link 组织方式的最佳参考）
- `i2s-reg.h` / `i2s-evb_register.c`：寄存器定义

## 移植要点（主线 6.18+）

1. **依赖 mtk-base-afe 框架**：代码基于 `sound/soc/mediatek/common/base.h`
   （`mtk_base_afe` / `mtk_base_afe_memif` / `mtk_base_afe_irq`）。
   该框架**在主线内核已有**，位于 `sound/soc/mediatek/common/`，
   但 Kconfig 符号被 mediatek target 限定，需要把公共部分对
   TARGET_airoha 开放（复制到独立目录或在 patch 中去掉依赖限定）。

2. **DT binding**：绑定名 `airoha,an7581-afe`（主线 an7581.dtsi 已有
   `afe@1fbe2200` 节点，disabled）。需要写：
   `Documentation/devicetree/bindings/sound/airoha,an7581-afe.yaml`。

3. **命名适配**：SDK 里的 `i2s_io_write()` 等平台读写封装要对齐主线
   `regmap` 惯例；中断、时钟用 devm_* 重写。

4. **验证路径**（不依赖 SLIC）：
   - 先做 **ETDM loopback**（SDK i2s-afe-pcm.c 里已有现成配置代码，
     搜索 "loopback"），aplay/arecord 验证 DMA 通路；
   - 再接 SLIC 的 PCM（Le9643 作 slave，SoC 提供 PCLK/FS）。

5. **工作量估计**：约 400-600 行适配代码 + binding + Makefile，
   1-2 个编译-烧写-验证循环。

## 与 machine 驱动的接口

移植后的 CPU DAI 名（`snd_soc_dai_driver.name`，如 "afe-pcm"）要填到
`snd-soc-airoha-slic` 的 `airoha_slic_dai_link.cpu_dai_name`，
或在 machine 驱动里改用 `cpu_of_node`（当前骨架已用 of_node 方式）。
