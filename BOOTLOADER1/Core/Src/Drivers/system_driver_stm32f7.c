/*
 * system_driver_stm32f7.c
 *
 *  Created on: Jan 28, 2026
 *      Author: mertk
 */
#include "system_interface.h"
#include "stm32f7xx_hal.h"
#include "tiny_printf.h"

UART_HandleTypeDef huart1;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MPU_Config(void);

void System_Interface_Init(void) {
	    MPU_Config();
	    HAL_Init();
	    SystemClock_Config();
	    MX_GPIO_Init();
	    MX_USART1_UART_Init();
	    tfp_init(&huart1);
}

void System_Interface_Reset(void) {
    HAL_NVIC_SystemReset();
}

void System_Interface_Delay(uint32_t ms) {
    HAL_Delay(ms);
}

uint8_t System_Interface_ReadUserButton(void) {
    return (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_11) == GPIO_PIN_SET) ? 1 : 0;
}

void System_Interface_ToggleHeartbeatLed(void) {
    HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);
}
uint32_t System_Interface_GetTick(void) {
    return HAL_GetTick();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
      HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);
      HAL_Delay(100);
  }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 10;
  RCC_OscInitStruct.PLL.PLLN = 210;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}
void System_Interface_Error_Handler(void) {
	Error_Handler();
}
/* --- STM32 Native Error Handler --- */
/* Bu fonksiyonu HAL kütüphanesi veya CubeMX kodları çağırabilir */


#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Kullanıcı buraya kendi raporlama kodunu ekleyebilir */
  /* Örneğin UART üzerinden hata basılabilir: */
  /* printf("Assert Failed: file %s on line %d\r\n", file, line); */
}
#endif /* USE_FULL_ASSERT */
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PI1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PI11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* MPU Configuration */

void MPU_Config(void)
{
 MPU_Region_InitTypeDef MPU_InitStruct = {0};

 /* Disables the MPU */
 HAL_MPU_Disable();

 /** Initializes and configures the Region and the memory to be protected
 */
 MPU_InitStruct.Enable = MPU_REGION_ENABLE;
 MPU_InitStruct.Number = MPU_REGION_NUMBER0;
 MPU_InitStruct.BaseAddress = 0x08000000;
 MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
 MPU_InitStruct.SubRegionDisable = 0x0;
 MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
 MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO;
 MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
 MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
 MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
 MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

 HAL_MPU_ConfigRegion(&MPU_InitStruct);

 /** Initializes and configures the Region and the memory to be protected
 */
 MPU_InitStruct.Number = MPU_REGION_NUMBER1;
 MPU_InitStruct.BaseAddress = 0x08010000;
 MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
 MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
 MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
 MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

 HAL_MPU_ConfigRegion(&MPU_InitStruct);
 /* Enables the MPU */
 HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}
