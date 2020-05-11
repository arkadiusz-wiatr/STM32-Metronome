#ifndef LCD_H_
#define LCD_H_

// GPIOx
#define GPIOx GPIOC

// Linie od D4 do D7 którymi będą przesyłane dane
#define D4 GPIO_PIN_4
#define D5 GPIO_PIN_5
#define D6 GPIO_PIN_6
#define D7 GPIO_PIN_7

// Dwie kolumny, każda po 16 znaków
#define LCD_ROWS 2
#define LCD_COLS 16

// Linia RS ( Register Select)
#define LCD_RS_PIN	GPIO_PIN_0
#define LCD_RS_PORT	GPIOB

// Linia RW ( Read/Write)
#define LCD_RW_PIN	GPIO_PIN_1
#define LCD_RW_PORT	GPIOB

// Linia E ( Enable)
#define LCD_E_PIN	GPIO_PIN_2
#define LCD_E_PORT	GPIOB

// Makro dla ustawienia stanu wysokiego i niskiego dla linii RS
#define LCD_RS_HIGH HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);
#define LCD_RS_LOW HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);

// Makro dla ustawienia stanu wysokiego i niskiego dla linii RW
#define LCD_RW_HIGH HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_SET);
#define LCD_RW_LOW HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);

// Makro dla ustawienia stanu wysokiego i niskiego dla linii E
#define LCD_E_HIGH HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
#define LCD_E_LOW HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);

//----------------------------------------------------------------------------
// Makra komend

#define LCD_CLEAR					0x01
#define LCD_HOME					0x02
#define LCDC_ENTRY_MODE				0x04
	#define LCD_EM_SHIFT_CURSOR		    0x00
	#define LCD_EM_SHIFT_DISPLAY	 	0x01
	#define LCD_EM_LEFT		   			0x00
	#define LCD_EM_RIGHT				0x02
#define LCD_ONOFF					0x08
	#define LCD_DISP_ON				    0x04
	#define LCD_CURSOR_ON				0x02
	#define LCDC_CURSOR_OFF				0x00
	#define LCDC_BLINK_ON				0x01
	#define LCDC_BLINK_OFF				0x00
#define LCD_SHIFT					0x10
	#define LCDC_SHIFT_DISP				0x08
	#define LCDC_SHIFT_CURSOR			0x00
	#define LCDC_SHIFT_RIGHT			0x04
	#define LCDC_SHIFT_LEFT				0x00
#define LCD_FUNC					0x20
	#define LCD_8_BIT					0x10
	#define LCD_4_BIT					0x00
	#define LCDC_TWO_LINE				0x08
	#define LCDC_FONT_5x10				0x04
	#define LCDC_FONT_5x7				0x00
#define LCDC_SET_CGRAM				0x40
#define LCDC_SET_DDRAM				0x80


//Adresy pamięci DDRAM odpowiadające pierwszym polom wyświetlacza z każdego wiersza.
//Pierwsza linia wyswietlacza
#define LCD_LINE1 		0x00

//Druga linia wyswietlacza LCD
#define LCD_LINE2 		0x40




void LCD_init();                              //Inicjalizuje wyswietlacz LCD
void LCD_display(uint8_t, uint8_t, char*, ...);    //Wyswietla tekst podany w argumencie
void LCD_writeHalf(uint8_t);                  //Zapisuje polowe bajtu na liniach D4-D7
void LCD_writeByte(uint8_t);                  //Wywoluje 2 x writeHalf, czyli zapisuje bajt
void LCD_writeCmd(uint8_t);                   //Zapisuje komende, wysylajac bajt danych
void LCD_writeData(uint8_t);                  //Zapisuje znak, wysylajac bajt danych
void LCD_cursorPosition(uint8_t, uint8_t);    //Ustawia pozycje kursora na LCD
void LCD_pinsInput();                         //Ustawia piny danych D4-D7 na input
void LCD_pinsOutput();                        //Ustawia piny danych D4-D7 na output
uint8_t LCD_readHalf();                       //Czyta pol bajta danych
uint8_t LCD_readByte();						  //Czyta bajt danych
uint8_t LCD_busyFlag();						  //Sprawdza bit zajętości

#endif
