
# AD7606 使用说明



## 使用说明 添加 .c .h 按照ad7606.h配置引脚 在main.c配置宏定义跟中断函数



## 硬件连接
- **SPI接口**：配一个MISO 一个CLK
  - SCK/MISO 连接到对应SPI引脚
  - SPI在CUBEMX中配置 **MSB 16bits** 格式，速率 <20MHz（无需DMA或中断）
- **CONVST**： 使用TIM输出占空比
  - 连接定时器PWM输出（TIM3_CH1）用于触发采样
  - 推荐配置：10kHz频率，占空比98%
- **RESET**：GPIO输出  用于芯片复位
- **CS**：   GPIO输出  用于SPI设备选择
- **BUSY**： 连接外部中断引脚，下降沿触发转换完成中断

- **下面这四个引脚无所谓**
- 康威科技需要5V驱动
- **OS0/OS1/OS2**：连接GPIO设置过采样率（需5V电平驱动）
- **RANGE**：连接GPIO设置输入范围（±5V/±10V，需5V电平驱动）

---

## 软件配置

### 宏定义（在main.c中配置）
```c
#define SAMPLING_POINTS   1024    // 每批次采样点数
#define SAMPLING_CHANNEL   8      // AD7606通道数

uint16_t ad7606Buff[SAMPLING_POINTS][SAMPLING_CHANNEL];  // 采样缓冲区
volatile uint8_t ad7606SamplingDoneFlag = 0;             // 采样完成标志
```

---

## 使用流程
1. **初始化**：调用 `AD7606Init()`
2. **可选配置**：
   - 调用 `AD7606SetOverSampling()` 设置过采样率
   - 调用 `AD7606SetRange()` 设置输入量程
3. **启动采样**：调用 `AD7606Start()`
4. **等待完成**：循环检测 `ad7606SamplingDoneFlag` 标志
5. **数据处理**：处理 `ad7606Buff` 中的数据
6. **重启采样**：完成处理后再次调用 `AD7606Start()`

---

## 中断处理 放在main.c的回调当中
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == AD_BUSY_Pin) {
    if (nums < SAMPLING_POINTS) {
      AD7606BusyIrqCallback(ad7606Buff[nums], SAMPLING_CHANNEL);
      nums++;
      ad7606SamplingDoneFlag = 0;
    } else {
      ad7606SamplingDoneFlag = 1;
    }
  }
}
```

---

## 电压量程 发送是补码 代码目前只写了5V的转换
| 量程范围 | 分辨率      | 数字量范围 |
|----------|-------------|------------|
| ±5V      | ≈0.153mV   | -32768~32767 |
| ±10V     | ≈0.305mV   | -32768~32767 |


## 注意事项
⚠️ **硬件相关**：
- AD7606 需 **5V电源供电**（STM32的3.3V可能无法驱动）

⚠️ **软件相关**：
- 采样缓冲区需按 `[SAMPLING_POINTS][SAMPLING_CHANNEL]` 二维数组访问
  
  **有问题请发ISSUE 被7606折磨坏了**
