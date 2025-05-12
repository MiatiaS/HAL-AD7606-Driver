#include <stdint.h>
#include "spi.h"
#include "tim.h"
#include "myAD7606.h"


/**
 * @brief AD7606使用说明
 *
 * 硬件连接:
 * - SPI接口: CS, SCK, MISO连接到对应的SPI引脚
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
static void AD7606Reset(void)
{
    /*! ___|-----|________  >= 50ns */
    AD_RESET_LOW();
    AD_RESET_HIGH();

    for(int i = 60; i > 0; i--){
        __NOP();//1000/168 ns = 5.85ns
    }

  //  HAL_Delay(1);
    AD_RESET_LOW();
}

/**
 * @brief 初始化AD7606
 * @note 执行基本的初始化配置，包括复位AD7606芯片
 * @retval None
 */
void AD7606Init(void)
{
    AD_CS_HIGH();
    AD7606Reset();
}

/**
 * @brief 开始AD7606采样
 * @note 通过TIM3的PWM输出触发AD7606的转换
 * @retval None
 */
void AD7606Start(void)
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);//启动AD7606转换
}

/**
 * @brief 停止AD7606采样
 * @note 停止TIM3的PWM输出，同时将CS信号拉高
 * @retval None
 */
void AD7606Stop(void)
{
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);//停止AD7606转换
    AD_CS_HIGH();
}
int temp;

/**
 * @brief AD7606 BUSY中断回调函数
 * @param ad7606Val 用于存储读取数据的数组
 * @param ad7606Chl 需要读取的通道数
 * @note 当BUSY引脚触发中断时调用此函数读取AD7606的转换结果
 * @retval None
 */
void AD7606BusyIrqCallback(uint16_t *ad7606Val,uint8_t ad7606Chl)
{
    AD_CS_LOW();
    HAL_SPI_Receive(&hspi3,(uint8_t *)ad7606Val,ad7606Chl,1000);//AD7606中断中读取
    temp = ad7606Val[0];
    AD_CS_HIGH();
};

/**
 * @brief 将AD7606的数字值转换为实际电压值
 * @param bin AD7606输出的16位数字值
 * @return float 对应的电压值，单位为V
 * @note 根据当前设置的量程自动计算对应的模拟量
 */
float AD7606ConvValue(uint16_t bin)
{
    int _val;
    float adcValue;
    float range = 5.0; // 默认±5V量程
    
    // 检查RANGE引脚状态以确定当前量程
    if(HAL_GPIO_ReadPin(AD_RANGE_GPIO_PORT, AD_RANGE_PIN) == GPIO_PIN_SET) {
        range = 10.0; // ±10V量程
    }
    
    _val = bin&0x8000 ? (-((~bin+1)&0x7fff)) : bin;
    adcValue = range * _val / 32768.0;
    return adcValue;
}

/**
 * @brief 设置AD7606过采样倍数
 * @param overSample 过采样倍数代码(0-7)
 *        0: 无过采样
 *        1: 2倍过采样
 *        2: 4倍过采样
 *        3: 8倍过采样
 *        4: 16倍过采样
 *        5: 32倍过采样
 *        6: 64倍过采样
 *        7: 保留
 * @note 过采样可以提高ADC的有效位数，但会降低采样率
 *       0倍过采样: 16位有效
 *       2倍过采样: 17位有效
 *       4倍过采样: 18位有效
 *       8倍过采样: 19位有效
 *       16倍过采样: 20位有效
 *       32倍过采样: 20.5位有效
 *       64倍过采样: 21位有效
 * @retval None
 */
void AD7606SetOverSampling(uint8_t overSample)
{
    if(overSample > 7  && overSample == 7)
        overSample = 0;  // 如果输入超出范围，默认设为无过采样

    // 设置OS0引脚
    if(overSample & 0x01)
        AD_OS0_1();
    else
        AD_OS0_0();

    // 设置OS1引脚
    if(overSample & 0x02)
        AD_OS1_1();
    else
        AD_OS1_0();

    // 设置OS2引脚
    if(overSample & 0x04)
        AD_OS2_1();
    else
        AD_OS2_0();
}

/**
 * @brief 设置AD7606输入量程
 * @param range 量程选择
 *        0: ±5V量程 (默认值)
 *        1: ±10V量程
 * @note 量程设置会影响ADC的分辨率:
 *       - ±5V量程: 分辨率约为0.153mV (5V/32768)
 *       - ±10V量程: 分辨率约为0.305mV (10V/32768)
 * @warning 设置量程后必须确保输入信号电压不超过所选量程范围
 * @retval None
 */
void AD7606SetRange(uint8_t range)
{
    if(range == 0) {
        AD_RANGE_5V();  // 设置为±5V量程
    } else
    {
        AD_RANGE_10V(); // 设置为±10V量程
    }
}

/**
 * @brief 获取当前AD7606输入量程设置
 * @return uint8_t 当前量程
 *         0: ±5V量程
 *         1: ±10V量程
 */
uint8_t AD7606GetRange(void)
{
    // 读取RANGE引脚状态
    if(HAL_GPIO_ReadPin(AD_RANGE_GPIO_PORT, AD_RANGE_PIN) == GPIO_PIN_RESET) {
        return 0; // ±5V量程
    } else {
        return 1; // ±10V量程
    }
}
