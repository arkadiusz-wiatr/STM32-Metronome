/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

extern char BUFF_RX[BUFF_SIZE];
char BUFF_TX[BUFF_SIZE] = {0};

uint8_t sender_byte = 0;
uint8_t receiver_byte = 0;

__IO uint8_t TX_BUSY = 0;
__IO uint8_t TX_EMPTY = 0;
__IO uint8_t RX_BUSY = 0;
__IO uint8_t RX_EMPTY = 0;

char payload[BUFF_SIZE] = {0};
int received = 0;


extern uint8_t receiver_state;



/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart2) //
			{
		if (TX_EMPTY != TX_BUSY) {
			uint8_t temp = BUFF_TX[TX_BUSY];
			TX_BUSY++;
			if (TX_BUSY >= BUFF_SIZE)
				TX_BUSY = 0;
			HAL_UART_Transmit_IT(&huart2, &temp, 1);
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart2) {
		RX_EMPTY++;
		if (RX_EMPTY >= BUFF_SIZE)
			RX_EMPTY = 0;
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&BUFF_RX[RX_EMPTY], 1);
	}
}






char UART_GetChar(){
	__IO char idx;
	if(RX_EMPTY != RX_BUSY)
	{
		idx = BUFF_RX[RX_BUSY];
		RX_BUSY++;
		if(RX_BUSY >= BUFF_SIZE)
			RX_BUSY = 0;
		return idx;
	}else
		return -1;
}

void UART_Send_Tx(char* text, ...){
	char BUFF_TMP[BUFF_SIZE];
	__IO int idx = TX_EMPTY;

	va_list valist;
	va_start(valist,text);
	vsprintf(BUFF_TMP, (char *)text, valist);
	va_end(valist);

	for(int i = 0; i < strlen(BUFF_TMP); i++){
		BUFF_TX[idx] = BUFF_TMP[i];
		idx++;

		if(idx >= BUFF_SIZE)
			idx = 0;
	}

	__disable_irq();

	if((TX_EMPTY == TX_BUSY) && (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == SET)){
		TX_EMPTY = idx;

		uint8_t tmp = BUFF_TX[TX_BUSY];
		TX_BUSY++;

		if(TX_BUSY >= BUFF_SIZE)
			TX_BUSY = 0;

		HAL_UART_Transmit_IT(&huart2, &tmp, 1);
	}else
		TX_EMPTY = idx;

	__enable_irq();
}

void AppendChar(char *frame, char c ){
	int frame_len = strlen(frame);
	if(frame_len < 55)
		frame[frame_len] = c;
}

int SendFrame(char *text, ...){

	char payload[50] = {0};
	va_list valist;
	va_start(valist, text);
	vsprintf(payload, (char *)text, valist);
	va_end(valist);

	uint32_t payload_len = strlen(payload);
	uint32_t sum_byte = 0;
	char frame[55] = {0};

	AppendChar(frame, END);

	if(payload_len <= 50){
		for (int i = 0; payload[i] != '\0'; i++) {
			switch(payload[i]){
			case END:
				sum_byte += END;
				AppendChar(frame, ESC);
				AppendChar(frame, ESC_END);
				break;
			case ESC:
				sum_byte += ESC;
				AppendChar(frame, ESC);
				AppendChar(frame, ESC_ESC);
				break;
			default:
				sum_byte += payload[i];
				AppendChar(frame, payload[i]);
			}
		}
	}else {
		SendFrame("E1");
		return -1;
	}

	AppendChar(frame, 0xDC);
	AppendChar(frame, 0xDD);

	sum_byte += 0xDC + 0xDD;

	AppendChar(frame, (char)(sum_byte % 256));
	AppendChar(frame, END);


	UART_Send_Tx("%s",frame);

	return 0;
}

uint8_t CheckSum() {

	int sum_byte = payload[received-1];
	uint32_t result = 0;

	for (int i = 0; i < received-1 ; i++) {
			result += payload[i];
	}

	result %= 256;

	return ( sum_byte == result ? 1 : 0);
}


void ClearPayload(){

	for(int i = 0; i < received; i++)
		payload[i] = '\0';

	received = 0;
}

void ClearChecksumAndAddresses() {
	for(int i = received -1; i > received - 4; i--)
		payload[i] = '\0';
}

void AnalyzeFrame(){
	if(received > 2){
		if(CheckSum()){
			sender_byte = payload[received - 3];
			receiver_byte = payload[received - 2];
			ClearChecksumAndAddresses();
			SendFrame("A1");
			DoCommand(payload);
		}else
			SendFrame("E2");
	}
}


void DecodeFrame() {
	char c = 0;

	if(RX_EMPTY != RX_BUSY) {

		c = UART_GetChar();


		switch(receiver_state) {
			case WAIT_HEADER:
				if(c == END) {
					ClearPayload();
					receiver_state = IN_MSG;

				}
				break;
			case IN_MSG:
				if(c == ESC) {
					receiver_state = AFTER_ESC;
				} else if(c == END) {
					AnalyzeFrame();
					ClearPayload();
					receiver_state = IN_MSG;
				} else
					if(received < 53){
					payload[received++] = c;
					}else{
						SendFrame("E1");
						ClearPayload();
		                receiver_state = WAIT_HEADER;

					}
				break;
			case AFTER_ESC:
				switch(c) {
					case ESC_END:
						c = END;
						break;
					case ESC_ESC:
						c = ESC;
						break;
					default:
						ClearPayload();
		                receiver_state = WAIT_HEADER;

				}
				if(received < 53) {
					payload[received++] = c;
					receiver_state = IN_MSG;
				}
				else {
					SendFrame("E1");
					ClearPayload();
	                receiver_state = WAIT_HEADER;

				}
				break;
		}
	}
}





/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
