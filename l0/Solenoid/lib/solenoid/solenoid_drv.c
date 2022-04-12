/******************************************************************************
 * @file low-level button
 * @brief driver example a simple button
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "solenoid_drv.h"
#include "stm32f0xx_hal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief initialize button hardware
 * @param None
 * @return None
 ******************************************************************************/
void solenoid_drv_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // configure gpio clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin : button pin */
    GPIO_InitStruct.Pin  = SOLENOID_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SOLENOID_Port, &GPIO_InitStruct);
}

/******************************************************************************
 * @brief read the button state
 * @param None
 * @return button state
 ******************************************************************************/
void solenoid_drv_write(uint8_t state)
{
    if (state)
    {
        HAL_GPIO_WritePin(SOLENOID_Port, SOLENOID_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(SOLENOID_Port, SOLENOID_Pin, GPIO_PIN_RESET);
    }
}