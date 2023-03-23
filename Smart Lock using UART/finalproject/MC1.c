
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "std_types.h"
#include <avr/io.h>
#include <avr/delay.h>
/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/
#define PASSWORD_LENGTH			4
#define KEYPAD_INPUT_DELAY		500
#define DISPLAY_MESSAGE_DELAY	1000
/*Random numbers to prevent MC1 and MC2 from Sending at the same time*/
#define PASSWORD_MATCHED		1
#define PASSWORD_MISMATCHED		0
#define WAIT1					0x01
#define WAIT2					0x02
#define WAIT3					0x03
#define WAIT4					0x04
#define WAIT5					0x05
#define WAIT6					0X06

/*******************************************************************************
 *                      		Variables                                    *
 *******************************************************************************/
uint8 password[PASSWORD_LENGTH];
uint8 flag = 0;
/*******************************************************************************
 *                      Function Prototypes                                  *
 *******************************************************************************/
void initializePassword();
void getPassword(uint8 * arrayName);
void DisplayOptions();
void DoorOpeningTask();
void sendPassword(uint8 * passwordArray);
/*******************************************************************************
 *                       	Main Code                                    *
 *******************************************************************************/

void main(){
	SREG |=(1<<7);
	UART_init();//initialize UART
	LCD_init();//initialize LCD
	initializePassword(); //initialize first-time password
	DisplayOptions();//Display Options

	uint8 receivedByte=0,key=0;
	while(1){
		key = KeyPad_getPressedKey();
		if (key == '+') {
			LCD_clearScreen();
			LCD_displayString("Enter Password");
			getPassword(password);
			UART_sendByte(WAIT1);
			sendPassword(password);
			UART_sendByte('+');
			receivedByte = UART_recieveByte();
			if (receivedByte == WAIT4) {
				DoorOpeningTask();

			} else if (receivedByte == WAIT5) {
				LCD_clearScreen();
				LCD_displayString("Wrong password");
				_delay_ms(DISPLAY_MESSAGE_DELAY);
			}
			DisplayOptions();


		} else if (key == '-') {
			LCD_clearScreen();
			LCD_displayString("Enter Password");
			getPassword(password);
			UART_sendByte(WAIT1);
			sendPassword(password);
			UART_sendByte(WAIT3);

			receivedByte = UART_recieveByte();
			if (receivedByte == WAIT6) {
				initializePassword();
				LCD_clearScreen();
			} else if (receivedByte == WAIT5) {
				LCD_clearScreen();
				LCD_displayString("Wrong password");
				_delay_ms(DISPLAY_MESSAGE_DELAY);
			}
			DisplayOptions();
		}
	}
}



/*******************************************************************************
 *                      		Functions                                   *
 *******************************************************************************/

void getPassword(uint8 * arrayName){
	LCD_goToRowColumn(1, 0);
	uint8 key;
	for(int i=0;i<PASSWORD_LENGTH;i++){
		key = KeyPad_getPressedKey();
		if (key >= 0 && key <= 9) {
			LCD_displayCharacter('*');
			*(arrayName + i) = key;
		}
		_delay_ms(KEYPAD_INPUT_DELAY);
}
}

void DisplayOptions(){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "+: open door");
	LCD_displayStringRowColumn(1, 0, "-: change password");
}

void initializePassword(){
	while(flag == PASSWORD_MISMATCHED){
		LCD_clearScreen();
		LCD_displayString("Enter New Pass");
		LCD_goToRowColumn(1, 0);
		getPassword(password);
		UART_sendByte(WAIT1);
		while (UART_recieveByte() != WAIT2);
		sendPassword(password);


		LCD_clearScreen();
		LCD_displayStringRowColumn(0, 0, "Re-enter Pass");
		LCD_goToRowColumn(1, 0);
		getPassword(password);
		UART_sendByte(WAIT1);
		while (UART_recieveByte() != WAIT2);
		sendPassword(password);


		while (UART_recieveByte() != WAIT1);
		flag = UART_recieveByte();

		if (flag == PASSWORD_MISMATCHED){
			LCD_clearScreen();
			LCD_displayString("Mismatch!!");
			_delay_ms(DISPLAY_MESSAGE_DELAY);
		}
	}
	flag = PASSWORD_MISMATCHED;
}

void DoorOpeningTask(){

	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "Door is Opening");
	_delay_ms(5000);

	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "Door is Closing");
	_delay_ms(5000);

}

void sendPassword(uint8 * passwordArray){
	uint8 cnt;
	for (cnt=0;cnt<PASSWORD_LENGTH;cnt++){
		UART_sendByte(passwordArray[cnt]);
		_delay_ms(100);
	}
}
