
#include "stm32f4xx_hal.h"
#include "LCD.h"
#include "stdarg.h"


//HAL_GIPO_WritePin - ustawianie stanów logicznych na określonych pinach
//Ustawienie stanów sygnalizujemy zboczem opadającym na linii E

void LCD_writeHalf(uint8_t data) {
	LCD_E_HIGH;
	HAL_GPIO_WritePin(GPIOx, D4, (data & 0x01));
	HAL_GPIO_WritePin(GPIOx, D5, (data & 0x02));
	HAL_GPIO_WritePin(GPIOx, D6, (data & 0x04));
	HAL_GPIO_WritePin(GPIOx, D7, (data & 0x08));
	LCD_E_LOW;
}

//Funkcja pozwalająca na zapisanie całego bajta
void LCD_writeByte(uint8_t data) {
	LCD_pinsOutput();

	LCD_writeHalf(data >> 4);
	LCD_writeHalf(data);

	while(LCD_busyFlag() & 0x80);

}


void LCD_writeCmd(uint8_t cmd) {
	LCD_RS_LOW;
	LCD_RW_LOW;
	LCD_writeByte(cmd);
}


void LCD_writeData(uint8_t data) {
	LCD_RS_HIGH;
	LCD_RW_LOW;
	LCD_writeByte(data);
}

void LCD_init() {
	LCD_writeCmd(LCD_FUNC | LCD_4_BIT | LCDC_TWO_LINE | LCDC_FONT_5x7);
	LCD_writeCmd(LCD_ONOFF | LCD_DISP_ON);
	LCD_writeCmd(LCD_CLEAR);
	LCD_writeCmd(LCDC_ENTRY_MODE | LCD_EM_SHIFT_CURSOR | LCD_EM_LEFT);
}

void LCD_display(uint8_t x, uint8_t y, char *string, ...) {
	char BUFF_TMP[255];

	va_list valist;
	va_start(valist, string);
	vsprintf(BUFF_TMP, string, valist);
	va_end(valist);

	LCD_cursorPosition(x, y);
	for(int i = 0; BUFF_TMP[i] != '\0'; i++)
		LCD_writeData(BUFF_TMP[i]);
}


void LCD_cursorPosition(uint8_t x, uint8_t y) {
	switch(y) {
		case 0:
			LCD_writeCmd(LCDC_SET_DDRAM | (LCD_LINE1 + x));
			break;

		case 1:
			LCD_writeCmd(LCDC_SET_DDRAM | (LCD_LINE2 + x));
			break;
	}
}

void LCD_pinsInput() {
	  GPIO_InitTypeDef GPIO_InitStruct;
	  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void LCD_pinsOutput() {
	  GPIO_InitTypeDef GPIO_InitStruct;
	  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

uint8_t LCD_readHalf() {
	  uint8_t tmp = 0;

	  LCD_E_HIGH;
	  tmp |= (HAL_GPIO_ReadPin(GPIOx, D4) << 0);
	  tmp |= (HAL_GPIO_ReadPin(GPIOx, D5) << 1);
	  tmp |= (HAL_GPIO_ReadPin(GPIOx, D6) << 2);
	  tmp |= (HAL_GPIO_ReadPin(GPIOx, D7) << 3);
	  LCD_E_LOW;

	  return tmp;
}

uint8_t LCD_readByte() {
	uint8_t result = 0;

	LCD_pinsInput();
	LCD_RW_HIGH;

	result = (LCD_readHalf() << 4);
	result |= LCD_readHalf();

	return result;
}




uint8_t LCD_busyFlag() {
	LCD_RS_LOW;
	return LCD_readByte();
}

