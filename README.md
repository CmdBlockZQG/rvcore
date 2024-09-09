# RVCORE

CmdBlock的RISC-V处理器核

## 子模块说明

- `include`
  - 公用头文件
    - 配置宏
    - ISA全局定义
- `common`
  - 全局公用模块
    - Log
- `sptenv`
  - Support Environment
  - 处理器核实现的支持环境，和core一起编译
    - 模拟内存和外设
    - 实现无关的trace工具 (TODO)
- `core`
  - 处理器核实现，以及实现相关的工具
    - 处理器核本身
    - 性能计数器
    - difftest ref/dut (TODO)
    - 实现相关的trace工具
  - `core-emu`：软件ISA模拟器实现
  - `core-rtl-a`：简单短流水线rtl实现 (TODO)
- `testbench`
  - 测试平台，驱动core运行
    - difftest monitor (TODO)
    - sdb - Simple Debugger (TODO)

## ISA支持

- RISC-V 32 IMAC
- zicsr zifencei zicntr
- 支持S模式和U模式
- 只支持单Hart
- 中断只由CLINT和PLIC提供
- 不支持指令集功能选择，misa只读，写入被忽略
- 仅支持小端序，mstatus.MBE/SBE/UBE均硬编码为0
- U模式执行WFI将立即触发非法指令异常，mstatus.TW可写入但被忽略
- mconfigptr硬编码为0
- 性能监视器, xenvcfg, mseccfg, PMP未实现
- wip实现为nop

## 目录结构

- `/build`
  - 构建目录
  - 包含运行时加载的动态库文件
- `/include`
  - 公共头文件，任意模块都可以引用
  - 特别地，common模块的头文件放置在此处，可供所有模块使用
- `/<module>`
  - 各个子模块的目录
  - `/<module>/include`
    - 模块对外暴露接口的头文件，仅调用该模块的模块可包含
    - 以构建脚本添加include目录的方式控制依赖关系，非法引用将编译错误
    - core模块编译产生动态库供testbench运行时加载，故无暴露头文件。接口定义位于`testbench/core.h`
