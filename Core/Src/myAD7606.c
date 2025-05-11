#include <stdint.h>
#include "spi.h"
#include "tim.h"
#include "myAD7606.h"

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

void AD7606Init(void)
{
    AD_CS_HIGH();
    AD7606Reset();
}

void AD7606Start(void)
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);//启动AD7606转换
}

void AD7606Stop(void)
{
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);//停止AD7606转换
    AD_CS_HIGH();
}
int temp;

void AD7606BusyIrqCallback(uint16_t *ad7606Val,uint8_t ad7606Chl)
{
    AD_CS_LOW();
    HAL_SPI_Receive(&hspi3,(uint8_t *)ad7606Val,ad7606Chl,100000);//AD7606中断中读取
    temp = ad7606Val[0];
    AD_CS_HIGH();
};

float AD7606ConvValue(uint16_t bin)
{
    int _val;
    float adcValue;
    _val = bin&0x8000 ? (-((~bin+1)&0x7fff)) : bin;
    adcValue = 5.0*_val/32768.0;
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
 * @retval None
 */
void AD7606SetOverSampling(uint8_t overSample)
{
    if(overSample > 7  || overSample == 7)
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
