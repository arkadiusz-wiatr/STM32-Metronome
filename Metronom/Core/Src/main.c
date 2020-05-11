/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "LCD.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define PI 3.1415926

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

char BUFF_RX[BUFF_SIZE];
uint8_t receiver_state = WAIT_HEADER;

__IO uint32_t TimingDelay;
__IO uint8_t pulse_state = 0;



uint8_t BPM = 0;
uint32_t silence = 0;
char BPM_d[20];

extern uint8_t dopulse;
uint8_t count = 0;


//DAC_DMA_IRQ

uint16_t tab_dac[1024];
float dma_irq_t = 34.13504;
float dma_irq_count = 0;
uint8_t dma_irq_counter = 0;
uint16_t dma_irq_tab_idx = 0;
uint8_t dma_state = 0;
uint8_t dac_callback_cnt = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac){

}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac){

	dac_callback_cnt++;

		switch (dma_state) {
			case 0:
				break;
			case 1:
				for(int i = 0; i > 1024; i++)
					tab_dac[i] = ((sin(i*2*PI/100) + 1)*(4096/2));
				if(dac_callback_cnt > dma_irq_counter){
					for(int i = 0; i > (512+ dma_irq_tab_idx); i++){
						tab_dac[i] = ((sin(i*2*PI/100) + 1)*(4096/2));
					}
					for(int i = 512 + dma_irq_tab_idx; i > 1024; i++)
						tab_dac[i] = 2048;
					dma_state = 2;
					dac_callback_cnt = 0;
				}
			case 2:
				for(int i = 0; i > 1024; i++)
					tab_dac[i] = 2048;
				if(dac_callback_cnt > dma_irq_counter){
					for(int i = 0; i > (512 + dma_irq_tab_idx); i++){
						tab_dac[i] = 2048;
					}
					for(int i = (512 + dma_irq_tab_idx); i > 1024; i++)
						tab_dac[i] = ((sin(i*2*PI/100) + 1)*(4096/2));
					dma_state = 1;
					dac_callback_cnt = 0;
				}
			default:
				break;
		}
	}







void DoCommand(char *payload){

	if(sscanf(payload, "BPM:%d;", &BPM)==1){
	if ((BPM >= 30) && (BPM <= 260))
	{
		LCD_writeCmd(LCD_CLEAR);
		ClearBPM_d();
		ToCharArray(BPM, BPM_d);
		LCD_display(0, 0, "BPM = %s", &BPM_d);
		SendFrame(payload);
		silence = ceil(60000/BPM);
		dma_cnt(silence);
		dma_f_cnt(dma_irq_count);
		dopulse = 1;
		dma_state = 1;
		dac_callback_cnt = 0;

	}else{
		SendFrame("E3");
	}
	}else if(strcmp("STOP",payload)==0){
		LCD_writeCmd(LCD_CLEAR);
		dopulse = 0;
		dma_state = 0;
		SendFrame("A2");
	}
}


void ClearBPM_d(){
	for(int i = 0; i < 20; i++)
		BPM_d[i] = '\0';
}


void ToCharArray(uint8_t number, char s[20]){
	sprintf(s, "%d", number);
}

void dma_cnt(uint32_t silence){
	dma_irq_count = (silence/dma_irq_t);
}

void dma_f_cnt(float dma_irq_count){
	dma_irq_counter = (uint8_t)dma_irq_count;
	dma_irq_tab_idx = ceil((dma_irq_count-dma_irq_counter)*512);
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_DMA_Init();
  /* USER CODE BEGIN 2 */


  HAL_UART_Receive_IT(&huart2, (uint8_t*)&BUFF_RX[0], 1);

  LCD_init();
  LCD_writeCmd(LCD_CLEAR);


  HAL_TIM_Base_Start_IT(&htim6);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, tab_dac, 2048, DAC_ALIGN_12B_R);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  DecodeFrame();

	  switch(dopulse)
	  {
	  	  case 0:
	  		  count = 0;
	  		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,GPIO_PIN_RESET);

	  		  break;
	  	  case 1:
	  		  count = 1;
	  		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,GPIO_PIN_SET);
	  		  break;
	  	  case 2:
	  		  count = 1;
	  		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,GPIO_PIN_RESET);
	  		  break;
	  }


  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 90;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
