#include "stdint.h"
#include "string.h"
#include "main.h" // 添加main.h以使用其中定义的引脚


/* 根据main.h更新GPIO端口和引脚定义 */
#define AD_CS_GPIO_PORT         AD_CS_GPIO_Port
#define AD_CS_PIN               AD_CS_Pin

#define AD_RESET_GPIO_PORT      AD_RESET_GPIO_Port
#define AD_RESET_PIN            AD_RESET_Pin

#define AD_CONVST_GPIO_PORT     AD_CONVST_GPIO_Port
#define AD_CONVST_PIN           AD_CONVST_Pin

#define AD_RANGE_GPIO_PORT      AD_RANGE_GPIO_Port
#define AD_RANGE_PIN            AD_RANGE_Pin

#define AD_OS0_GPIO_PORT        AD_OS0_GPIO_Port
#define AD_OS0_PIN              AD_OS0_Pin

#define AD_OS1_GPIO_PORT        AD_OS1_GPIO_Port
#define AD_OS1_PIN              AD_OS1_Pin

#define AD_OS2_GPIO_PORT        AD_OS2_GPIO_Port
#define AD_OS2_PIN              AD_OS2_Pin

#define AD_SPI_MISO_GPIO_PORT   AD_SPI_MISO_GPIO_Port
#define AD_SPI_MISO_PIN         AD_SPI_MISO_Pin

#define AD_SPI_SCK_GPIO_PORT    AD_SPI_SCK_GPIO_Port
#define AD_SPI_SCK_PIN          AD_SPI_SCK_Pin

#define AD_BUSY_GPIO_PORT       AD_BUSY_GPIO_Port
#define AD_BUSY_PIN             AD_BUSY_Pin

/* 使用main.h中定义的引脚进行控制 */
#define AD_CS_LOW()             HAL_GPIO_WritePin(AD_CS_GPIO_PORT, AD_CS_PIN, GPIO_PIN_RESET)
#define AD_CS_HIGH()            HAL_GPIO_WritePin(AD_CS_GPIO_PORT, AD_CS_PIN, GPIO_PIN_SET)

#define AD_RESET_LOW()          HAL_GPIO_WritePin(AD_RESET_GPIO_PORT, AD_RESET_PIN, GPIO_PIN_RESET)
#define AD_RESET_HIGH()         HAL_GPIO_WritePin(AD_RESET_GPIO_PORT, AD_RESET_PIN, GPIO_PIN_SET)

#define AD_RANGE_5V()           HAL_GPIO_WritePin(AD_RANGE_GPIO_PORT, AD_RANGE_PIN, GPIO_PIN_RESET)
#define AD_RANGE_10V()          HAL_GPIO_WritePin(AD_RANGE_GPIO_PORT, AD_RANGE_PIN, GPIO_PIN_SET)

#define AD_OS0_0()              HAL_GPIO_WritePin(AD_OS0_GPIO_PORT, AD_OS0_PIN, GPIO_PIN_RESET)
#define AD_OS0_1()              HAL_GPIO_WritePin(AD_OS0_GPIO_PORT, AD_OS0_PIN, GPIO_PIN_SET)

#define AD_OS1_0()              HAL_GPIO_WritePin(AD_OS1_GPIO_PORT, AD_OS1_PIN, GPIO_PIN_RESET)
#define AD_OS1_1()              HAL_GPIO_WritePin(AD_OS1_GPIO_PORT, AD_OS1_PIN, GPIO_PIN_SET)

#define AD_OS2_0()              HAL_GPIO_WritePin(AD_OS2_GPIO_PORT, AD_OS2_PIN, GPIO_PIN_RESET)
#define AD_OS2_1()              HAL_GPIO_WritePin(AD_OS2_GPIO_PORT, AD_OS2_PIN, GPIO_PIN_SET)

static void AD7606Reset(void);
void AD7606Init(void);
void AD7606Start(void);
void AD7606Stop(void);
void AD7606BusyIrqCallback(uint16_t *ad7606Val,uint8_t ad7606Chl);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
float AD7606ConvValue(uint16_t bin);
void AD7606SetOverSampling(uint8_t overSample);  // 新增函数声明

