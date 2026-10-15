# STM32H7 ADC / DMA / FFT 实验工程

基于 STM32H743 的 ADC 采样、CMSIS-DSP FFT、双通道全相位 FFT（APFT）和 AD9910 控制实验。本文依据提交 `53b800880d8f6db45cd60565ec3032cc47a2deec`（`hADC_optimise_firstpush`）的源码和工程配置整理。

> **当前默认程序不是自动运行的频谱仪。** `main()` 完成初始化、校准和 DMA 装载，配置 AD9910 为 1 MHz，然后每秒延时。`ADC1_Channel3()`、`main_apft()` 的主循环调用及 TIM6 启动均被注释。默认不应期待串口连续输出测量结果。本次仅补充文档，未改动这些行为。

## 1. 硬件与接线

工程选定 **STM32H743IIT6 / LQFP176 / Cortex-M7**，不是任意 H7 板卡的通用工程。仓库没有板卡型号、原理图或 BOM，无法确认具体开发板；需要具有对应引脚、供电及 **25 MHz 外部晶振**的板卡。`main.c` 使用 HSE 晶振模式、LDO 供电和 VOS0；使用其他晶振或供电方式时不能直接照搬。

| 用途 | MCU 引脚 / 配置 | 说明 |
| --- | --- | --- |
| 单通道 FFT 输入 | PC3_C，ADC3_INP1 | 注意是 PC3_C 模拟引脚，代码打开 PC3 模拟开关；按实际封装/板卡确认引出位置 |
| 双通道输入 1 | PA6，ADC1_INP3 | 算法预期位于打包字低 16 位 |
| 双通道输入 2 | PC4，ADC2_INP4 | 算法预期位于打包字高 16 位 |
| 串口 | PA9 TX、PA10 RX | USART1，115200、8N1、无流控；与 USB-TTL 交叉接线并共地 |
| SWD | PA13 SWDIO、PA14 SWCLK | 调试器另接 GND、目标电压参考，建议连接 NRST |
| HSE | PH0 / PH1 | 外部晶振模式，代码按 25 MHz 计算 |

ADC 输入必须限制在实际 VDDA/VREF 和数据手册允许范围内，交流信号需要适当直流偏置，不能直接输入负电压。FFT 电压换算固定假设参考电压为 3.3 V，并不代表板卡实际参考已校准。短采样时间还要求合适的信号源阻抗与模拟前端。

默认会操作 AD9910；完整复现实验需兼容模块、电源和正确参考时钟。驱动频率换算假设 DDS 系统时钟 **1 GHz**，模块参考源及 PLL 配置是否满足该条件需自行核验，不能仅凭写入 1 MHz 就确认实际输出频率。

| AD9910 驱动宏 | MCU 引脚 |
| --- | --- |
| AD9910_PWR / PROFILE1 / OSK / DRCTL | PI9 / PI10 / PF10 / PG13 |
| AD9910_SDIO / SCLK / CS / MAS_REST | PE4 / PE3 / PE6 / PE5 |
| PROFILE2 / PROFILE0 / UP_DAT | PB9 / PB8 / PB7 |
| DROVER / DRHOLD | PB5 / PB4 |

以上按 `Core/AD9910/ad9910.h` 的宏名记录；连接模块前仍需核对信号方向和电平，尤其不要仅根据宏名推断模块端口。

## 2. 文件入口

| 文件 / 目录 | 用途 |
| --- | --- |
| [Core/Src/main.c](Core/Src/main.c) | 初始化、默认运行行为、两条测量入口和频率跟踪调用 |
| [Core/Src/adc.c](Core/Src/adc.c)、[tim.c](Core/Src/tim.c)、[stm32h7xx_it.c](Core/Src/stm32h7xx_it.c) | ADC/DMA、触发时钟、中断与完成标志 |
| [Core/DSPFFT](Core/DSPFFT) | 4096 点单通道实数 FFT、幅度/频率估计、波形判别 |
| [Core/APFT](Core/APFT) | 双通道全相位 FFT 与相位差 |
| [Core/FreqAdjust](Core/FreqAdjust)、[Core/AD9910](Core/AD9910) | 相位漂移频率修正、DDS 驱动 |
| [Core/TR](Core/TR) | 阻塞式串口格式化发送 |
| `Drivers/` | 随仓库提供的 HAL / CMSIS 文件，不替代外部 DSP Pack |
| `MDK-ARM/` | Keil 工程、启动文件、调试和链接配置 |
| `H7DSPFFT and ADC optimise.ioc` | CubeMX 配置，供查阅和后续受控再生成 |
| [APFT相位差知识.txt](APFT相位差知识.txt)、[DMA&&SRAM.png](DMA%26%26SRAM.png) | 原有学习说明和内存示意，不能代替实测报告 |

## 3. 编译环境与打开方式

以下是工程文件记录的环境，**不是本次已成功编译验证的环境**：

| 组件 | 记录版本 / 来源 |
| --- | --- |
| Keil µVision | RTE 文件记录 5.40.0.0 |
| Arm Compiler | **6.22 / ARMCLANG**，`.uvprojx` 的实际编译器选择 |
| 器件 Pack | `Keil.STM32H7xx_DFP` **4.1.3** |
| CMSIS CORE Pack | `ARM.CMSIS` **6.1.0** |
| CMSIS-DSP Pack | `ARM.CMSIS-DSP` **1.15.0**，DSP **Source** 组件 |
| STM32CubeMX | `.ioc` 记录 **6.17.0** |
| STM32Cube H7 固件包 | `.ioc` 记录 **V1.13.0** |

`.ioc` 的目标工具链标签为 MDK-ARM V5.32，且带有 `CompilerLinker=GCC` 元数据；这不是当前 Keil 工程实际使用 GCC 的证据。复现优先以 `.uvprojx` 的 Arm Compiler 6.22 为准。仓库没有 CMake/Makefile 或可直接复现的 GCC 构建入口。

1. 将仓库克隆到本地，例如 Windows 的 `D:\DSH-Workspaces\Projects`：

   ```powershell
   git clone https://github.com/LiangMao520/H7DSPFFT-and-ADC-optimise.git
   cd H7DSPFFT-and-ADC-optimise
   git rev-parse HEAD
   ```

2. 安装 Keil MDK、Arm Compiler 6.22 和上述三个 Pack。克隆仓库不会自动安装它们；在 Pack Installer / Manage Run-Time Environment 检查 CORE 和 DSP Source 组件已解析。DSP 需提供 `arm_hanning_f32`、`arm_conv_f32`、RFFT 和 radix-4 CFFT 等实现；不要只补头文件。
3. 打开 `MDK-ARM/H7DSPFFT and ADC optimise.uvprojx`，选择同名 Target，确认 Device 是 STM32H743IITx、编译器为 6.22。
4. 先检查下节的 RAM/链接设置，再执行 **Rebuild**。查看 Build Output 中错误、警告以及 `.map` 文件；不要用旧产物代替本机重建。
5. 输出目录为 `MDK-ARM/H7DSPFFT and ADC optimise/`，输出名同工程名；工程启用了 HEX 生成，成功构建后应有 `.axf` / `.hex`。这些产物已被 `.gitignore` 排除，仓库未提供可直接烧录的固件或成功构建日志。

可选的 Windows 命令行构建（按本机安装位置修改）：

```powershell
& 'C:\Keil_v5\UV4\UV4.exe' -b 'MDK-ARM\H7DSPFFT and ADC optimise.uvprojx' -t 'H7DSPFFT and ADC optimise' -o 'build.log'
Get-Content 'build.log'
```

仅查看配置不需要重新生成代码。若打开 `.ioc`，先保留工作区并检查迁移提示；不要直接 Generate 覆盖现有自定义代码、RTE 或内存配置。

## 4. DMA 内存放置：烧录前必须检查

DMA1/2 不能访问 H743 的 DTCM。**`ADC1_3` 与 `wave_packed` 必须最终位于 DMA 可访问 SRAM，不能位于 `0x20000000` 的 DTCM。**

| 区域 | 起始地址 | 大小 | 本工程注意点 |
| --- | --- | --- | --- |
| DTCM | `0x20000000` | 128 KiB | CPU 工作数据可用；不能放 DMA1 缓冲区 |
| AXI SRAM | `0x24000000` | 512 KiB | DMA 缓冲区可选位置，须同时检查缓存一致性 |
| D2 SRAM1 / SRAM2 / SRAM3 | `0x30000000` / `0x30020000` / `0x30040000` | 128 / 128 / 32 KiB | DMA 可用候选，当前所附 scatter 未单列这些区域 |

当前两数组均为普通全局变量，无显式 section / 固定地址：`ADC1_3[4096]` 为 8192 字节，`wave_packed[2048]` 为 8192 字节。所附 `.sct` 对 DTCM 和 AXI SRAM 都使用 `.ANY (+RW +ZI)`，**不能保证特定数组分配到 AXI SRAM**。且 `.uvprojx` 当前采用 Target 内存布局生成链接脚本（`umfTarg=1`、`useFile=0`、`ScatterFile` 为空），不能把仓库中的 `.sct` 当成强制生效的手工放置规则。

复现检查：

1. Rebuild 后在 map / Debug 符号地址中查 `ADC1_3`、`wave_packed`，记录实际地址和长度。
2. 如落入 DTCM，在本地实验分支调整链接/内存配置。例如让全部 RW/ZI 使用 AXI SRAM（Target 中保留 AXI RAM 并作为数据区、排除 DTCM 数据分配），或使用显式 section 和手工 scatter 精确放置；重新链接并再次确认地址。本文没有替仓库实施此修改。
3. `MPU_Config()` 的背景保护配置不等于已为 DMA 建立独立的非缓存 SRAM 区。当前 Core 源码未见显式启用 I/D Cache；如果后续开启 D-Cache，必须设计非缓存 DMA 区或在 DMA/CPU 交接时做正确的缓存维护，并按缓存行对齐缓冲区。`volatile` 无法解决缓存一致性。
4. FFT/APFT 还有大量浮点工作数组；更换 RAM 方案后检查总容量和栈余量。启动文件记录栈 `0x400`、堆 `0x200`，不能视为压力测试通过。

## 5. ADC → DMA → FFT 数据流

按 25 MHz HSE 及当前 PLL 配置推算：SYSCLK 480 MHz，HCLK 240 MHz，APB1 120 MHz；在当前定时器倍频条件下 TIM3/TIM6 时钟为 240 MHz。两者 PSC=9、ARR=23，更新触发率为：

```text
Fs = 240000000 / (9 + 1) / (23 + 1) = 1000000 Hz
```

这是配置推算，尚未用示波器或计数器实测。ADC 公共时钟配置为 PLL2 输出 72 MHz，ADC1/2 另设异步 /2；不能把 ADC 时钟直接当成采样率。采样由 TRGO 决定，且必须满足实际转换时序。

### 单通道：ADC3 / 4096 点 FFT

`TIM3 update TRGO → ADC3_INP1 (16 bit) → DMA1_Stream2 (半字、循环) → ADC1_3[4096] → FFT_Calculation()`。

- 变量名 `ADC1_3` 和入口名 `ADC1_Channel3()` 是历史命名，实际配置对应 **ADC3**。
- 初始化装载 ADC3 DMA，但不启动 TIM3。`ADC1_Channel3()` 启动 TIM3，等待 `AD1_3Flag`；DMA 全完成中断置标志并停止 TIM3，使本帧暂时稳定。
- 算法去均值、加 Hann 窗、4096 点实数 FFT、求幅度谱并寻峰，收集峰附近三条谱线能量并作窗补偿。频率输出为 `max_index * Fs / NPT`；名义频点间隔约 **244.140625 Hz**，不是高精度插值测频。
- `V` 使用代码中的比例式与 3.3 V 假设；其峰值/峰峰值/RMS 含义及误差尚需用已知幅值校准，不承诺绝对电压精度。
- `Wave_Judging()` 再执行一次 FFT，用三、五次谐波关系区分三角波和正弦波；返回 0 也可能是边界/无有效基波等情况，不能等价为严格识别成功。
- 串口计划输出 `V=...`、`f=...` 和 `tri` / `sin`，当前默认未调用此入口。

### 双通道：ADC1 + ADC2 / APFT

`TIM6 update TRGO → ADC1/ADC2 规则同步 → ADC 公共数据 → DMA1_Stream0 (NORMAL) → wave_packed → APFT → Phase`。

- ADC1 为主 ADC、ADC2 为从 ADC；算法按每个 `uint32_t` 的低/高 16 位分离两路。
- `APFT_NPT=1024`；缓冲区分配 2048 个打包字，APFT 实际使用前 2047 组样本，经 Hann 窗卷积归一化、重叠折叠后，各做 1024 点复数 FFT。
- 两路分别寻峰后计算 `Phase = Wave_1.phase - Wave_2.phase` 并折返到约 `[-180°, 180°]`。两路应是具有可比相位的同频信号；任意不同频率各自寻峰的结果不能直接作为稳定相位差。
- 完成中断置 `APFT1Flag` 且继续调用 `HAL_DMA_IRQHandler()`。`main_apft()` 等待完成、停 TIM6 和多模式 DMA、处理与发送，再重新装载 DMA 并启动 TIM6；这是有处理间隙的分帧流程，不是无缝流式采集。

**双通道尚有配置一致性疑点：** `DualModeData` 选择 `ADC_DUALMODEDATAFORMAT_32_10_BITS`，但 DMA 外设/内存宽度均为 HALFWORD，启动长度为 `APFT_NPT*2=2048`。按该 DMA 宽度只写 4096 字节，而数组有 8192 字节；仅指针转换成 `uint32_t*` 不会改变 DMA 宽度。不能据此断言已得到 2048 组正确打包样本。上板前应结合 H743 多模式数据格式、DMA NDTR 和两路已知输入检查实际字节布局、传输数量及解包结果；确认方案后另行修改配置，本次不修正逻辑。

### DDS 频率跟踪入口

`main_apft()` 还包含粗测频捕获和相位漂移修正：频率明显变化阈值 3 Hz，细调间隔至少 100 ms，`FreqAdjust` 增益 0.5、单次最大 0.5 Hz、总修正范围相对起点 ±20 Hz。

但当前工程只初始化 TIM3/TIM6；未见设置 `TIM15IsUpdate` / `TIMgetfreq` 的有效粗测频生产路径。仅取消 `main_apft()` 注释不足以完成锁频，还需恢复并验证粗测频链路。不可将该入口描述为默认可用的闭环锁频功能。

## 6. 烧录与分级运行

### A. 原样固件：初始化 / DDS 检查

1. 断电核对芯片、25 MHz 晶振、LDO 供电和接线，连接 SWD 调试器并上电；USB-TTL 使用兼容的逻辑电平，共地。
2. 在 µVision `Options for Target → Debug` 选择实际调试器与 SWD 接口。随附 `.uvoptx` 含 ST-LINK 等配置记录，但不保证与本机探针一致。
3. 在 `Utilities / Flash Download` 检查对应 H743 2 MiB 内部 Flash 算法；随附配置记录 `STM32H7x_2048.FLM`，Flash 基址为 `0x08000000`。
4. 完成编译和 DMA 地址核查后，执行 Download；成功后复位运行。也可用 STM32CubeProgrammer 经 SWD 下载本机生成 HEX（含地址），该替代流程本次未实测。
5. Debug 单步确认经过 `SystemClock_Config()`、ADC 校准和 DMA 启动，到达主循环；若停在 `Error_Handler()` / Fault Handler，回溯首个失败调用，不要把停机当成正常等待。
6. 有 AD9910 时用示波器/频率计检查名义 1 MHz 正弦输出。默认幅度码 16383、相位码 0 不等同于已知输出电压；无模块时不能验证 DDS。
7. 默认 TIM3/TIM6 没启动，DMA 缓冲区不更新、USART1 无测量输出是预期现象，不应仅以“串口无字”判断烧录失败。

### B. 单通道 FFT：本地实验分支

此级需要**人工启用现有入口**，原样固件不会执行；本 README 提交未改代码。

1. 先解决 DMA 地址要求，再将已知正弦信号加偏置后送到 PC3_C；建议先用远低于 500 kHz 的信号，例如 10 kHz，避免一开始在 Nyquist 边缘测试。
2. 在本地实验分支启用主循环中的 `ADC1_Channel3()`，先保持 `main_apft()` 关闭，Rebuild 并烧录；该函数自己启动 TIM3，不需要另加启动。
3. 打开 115200 8N1 串口，检查 `AD1_3Flag`、原始 ADC 码是否随输入变化、是否削顶，以及 `V/f/tri/sin` 输出。
4. 改变已知频率/幅值并记录结果；频率预期量化到 FFT 频点，幅值须校准。采样完成标志一直不来时检查 TIM3、DMA 地址/错误、IRQ 与 ADC 触发。

### C. 双通道 APFT：先验数据布局，再算相位

1. 先解决前述双 ADC 的 DMA 宽度/数量疑点和内存地址要求，不能直接假定当前缓冲区有效。
2. 将同一已知信号分到 PA6、PC4，检查两路解包结果；再提供已知相位差，比较符号、零点偏差、跨 ±180° 行为。不要直接把默认 DDS 1 MHz 输出接入名义 1 MS/s 采样链作为有效测量信号。
3. 在本地实验分支启用初始化后的 `HAL_TIM_Base_Start(&htim6)` 和主循环的 `main_apft()`。首次进入该函数前必须已有触发，否则它会停在 `while(!APFT1Flag)`；保持另一处理入口关闭。
4. 观察 `APFT1Flag`、`wave_packed`、两路 FFT 峰与 `Phase=...`。正常帧应经历采集、停止、处理、重装、再触发。
5. 锁频另作为独立实验：补齐粗测频更新来源、核对输入/输出接到哪一路及相位反馈符号后，再验证捕获、跟踪与稳定性。

## 7. 已核对内容与尚未验证项目

本次为文档整理与静态核对，**没有 Keil 工具链和目标硬件进行构建、烧录或性能测试**。仓库未附成功构建日志、自动化测试或可对应的实测结果，不能声称已有硬件功能通过验收。

| 项目 | 当前可确认状态 | 后续验证 |
| --- | --- | --- |
| MCU、工具版本、引脚和初始化配置 | 已按工程/源码核对 | 板卡实物和时钟/供电匹配 |
| 默认初始化和 DDS 写入流程 | 源码存在 | 本机 Rebuild、HAL 返回值、实际 DDS 输出 |
| ADC3 + DMA + FFT / 波形判别 | 实现存在，默认入口关闭 | DMA 地址、采样率、幅频标定、判别准确度 |
| ADC1/2 同步采样 + APFT | 实现存在，DMA 数据布局待核验 | 双路有效样本、相位误差、噪声与边界 |
| DMA SRAM 放置 | 未显式保证 | map 地址与 DMA 错误/传输结果 |
| Cache / MPU 一致性 | 未提供完整 DMA 缓存策略证明 | 开启 Cache 后的正确性及压力测试 |
| 自动捕获 / 细锁频 | 缺少当前有效粗测频更新路径 | 完整闭环、动态响应、漂移与失锁恢复 |
| 长时间稳定性及最高性能 | 无测试报告 | 连续运行、溢出/丢帧、CPU 占用、栈余量 |
| 其他 H7 / GCC / CubeIDE | 无对应构建入口或验证记录 | 独立移植与回归 |

建议每次实测记录：提交 SHA、板卡与晶振、工具/Pack 版本、本地改动、map 中两 DMA 缓冲区地址、输入频率/幅值/偏置、实际采样率、输出日志及误差。这样才能将“源码已有”逐项升级为可复现的硬件验证结果。
