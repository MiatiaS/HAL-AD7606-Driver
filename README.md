/**
 * @brief AD7606使用说明
 *
 * 硬件连接:
 * - SPI接口: SCK, MISO连接到对应的SPI引脚 MSB 16BITS 速率在20M以下都能跑 无需使用DMA 或者 IT
 * - CONVST: 连接到定时器PWM输出(TIM3_CH1)用于触发采样 这里使用10K 占空比为98
 * - RESET: 连接到GPIO用于复位芯片
 * - BUSY: 连接到外部中断引脚(PC3)用于通知数据转换完成
 * - CS: 连接到GPIO用于选择SPI设备
 * - OS0/OS1/OS2: 连接到GPIO用于设置过采样率        (注意，康威需要5V驱动)
 * - RANGE: 连接到GPIO用于设置输入电压范围(±5V或±10V)(注意，康威需要5V驱动)
 *
 * 软件配置:
 * 1. 在main.c函数中定义:
 *    #define SAMPLING_POINTS  1024   // 每个采样批次的采样点数
 *    #define SAMPLING_CHANNEL 8      // AD7606的通道数量
 *
 *    uint16_t ad7606Buff[SAMPLING_POINTS][SAMPLING_CHANNEL]; // 存储采样数据的缓冲区
 *    volatile uint8_t ad7606SamplingDoneFlag = 0;          // 采样完成标志
 *
 * 2. 使用流程:
 *    a) 调用AD7606Init()初始化
 *    b) 调用AD7606SetOverSampling()设置过采样率(可选)
 *    c) 调用AD7606SetRange()设置输入电压范围(可选)
 *    d) 调用AD7606Start()开始采样
 *    e) 等待ad7606SamplingDoneFlag置1表示采样完成
 *    f) 处理ad7606Buff中的数据
 *    g) 处理完后调用AD7606Start()开始新一轮采样
*   在回调地方放置
*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == AD_BUSY_Pin)
    {
      if(nums < SAMPLING_POINTS){
        AD7606BusyIrqCallback(ad7606Buff[nums],SAMPLING_CHANNEL);
        nums++;
          ad7606SamplingDoneFlag = 0;
      }
      else
        ad7606SamplingDoneFlag = 1;
  }
}
 * 3. 中断处理:
 *    - BUSY引脚下降沿触发中断
 *    - 中断服务程序自动读取所有通道数据
 *    - 达到SAMPLING_POINTS后设置ad7606SamplingDoneFlag标志
 *
 * 电压范围:
 * - ±5V量程: 分辨率为5V/32768 ≈ 0.153mV
 * - ±10V量程: 分辨率为10V/32768 ≈ 0.305mV
 *
 * 注意事项:
 * - AD7606需要5V电源供电，STM32的3.3V可能无法正常驱动
 * - 输入信号电压不能超过选定的量程范围
 */
