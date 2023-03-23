
#include "external_eeprom.h"
#include <avr/io.h>
#include <avr/delay.h>
#include "uart.h"

/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/
#define PASSWORD_LENGTH						4
#define NUMBER_OF_WRONG_PASSWORD_ATTEMPTS 	3
#define MOTOR_ON_TIME 						5000
#define BUZZER_ON_TIME 						3000
/*Random numbers to prevent MC1 and MC2 from Sending at the same time*/
#define PASSWORD_MATCHED		1
#define PASSWORD_MISMATCHED		0
#define WAIT1					0x01
#define WAIT2					0x02
#define WAIT3					0x03
#define WAIT4					0x04
#define WAIT5					0x05
#define WAIT6					0X06


#define EEPROM_STORE_ADDREESS 0x00
#define BUZZER_DDR  		  DDRC
#define BUZZER_PORT 		  PORTC
#define BUZZER_PIN 			  PC3
#define LED_DDR 		      DDRC
#define LED_PORT		      PORTC
#define RED_LED_PIN 		  PC7
#define GREEN_LED_PIN 		  PC6
#define MOTOR_DDR 		      DDRD
#define MOTOR_PORT		      PORTD
#define MOTOR_PIN2 		      PD6
#define MOTOR_PIN1 		      PD7
/*******************************************************************************
 *                      		Variables                                *
 *******************************************************************************/
uint8 Password_Received[PASSWORD_LENGTH];
uint8 Saved_Password[PASSWORD_LENGTH];
uint8 counter=0;
uint16 seconds = 0;


/*******************************************************************************
 *                     Function Prototypes                                 *
 *******************************************************************************/
uint8 compare_passwords(uint8 a_password1[PASSWORD_LENGTH],uint8 a_password2[PASSWORD_LENGTH]);
void initializePassword();
void Open_Door();
void receivePassword(uint8 * passwordArray);
void readPassword();
void savePassword();
void DPIO();
void BZR_LED_ON();
void BZR_LED_OFF();
void LED_ON();
void LED_OFF();
void Timer1_PhaseCorrectPWM_init();
void clockwise();
void anticlockwise();
int main(void){
	SREG |=(1<<7);
	UART_init();
	EEPROM_init();
	DPIO();
	initializePassword();
	Timer1_PhaseCorrectPWM_init();
	uint8 receivedByte=0;
	while(1){
		if (UART_recieveByte() == WAIT1){
			receivePassword(Password_Received);
			receivedByte = UART_recieveByte();

			if ( receivedByte == '+'){
				if (compare_passwords(Saved_Password, Password_Received) == PASSWORD_MATCHED){
					UART_sendByte(WAIT4);
					Open_Door();
				}else{
					UART_sendByte(WAIT5);
					counter++;
					if (counter == NUMBER_OF_WRONG_PASSWORD_ATTEMPTS){
						BZR_LED_ON();
						_delay_ms(BUZZER_ON_TIME);
						BZR_LED_OFF();
						counter=0;
					}
				}


			} else if (receivedByte == WAIT3) {
				if (compare_passwords(Saved_Password, Password_Received) == PASSWORD_MATCHED) {
					UART_sendByte(WAIT6);
					initializePassword();
				}else{
					UART_sendByte(WAIT5);
				}
			}
		}
	}
}


/*******************************************************************************
 *                     			Functions                                   *
 *******************************************************************************/
void DPIO(){
	SET_BIT(BUZZER_DDR,BUZZER_PIN);
	SET_BIT(LED_DDR,RED_LED_PIN);
	SET_BIT(LED_DDR,GREEN_LED_PIN);
	SET_BIT(MOTOR_DDR,MOTOR_PIN1);
	SET_BIT(MOTOR_DDR,MOTOR_PIN2);
}
void Timer1_PhaseCorrectPWM_init(){
	TCNT1=0;
	DDRD|=(1<<PD5);//setting OC1A as a output pin(PD5)
	TCCR1A|=(1<<WGM10)|(1<<COM1A1);//PWM phase correct 8-bit
	TCCR1B|=(1<<CS10);//no pre-scaler
}
uint8 compare_passwords(uint8 password1[PASSWORD_LENGTH],uint8 password2[PASSWORD_LENGTH]) {
	readPassword();
	uint8 i;
	for (i = 0; i < PASSWORD_LENGTH; i++) {
		if (password1[i] != password2[i]) {
			return PASSWORD_MISMATCHED;
		}
	}
	return PASSWORD_MATCHED;
}
void clockwise(){
	CLEAR_BIT(MOTOR_PORT,MOTOR_PIN1);
	SET_BIT(MOTOR_PORT,MOTOR_PIN2);
}
void anticlockwise(){
	SET_BIT(MOTOR_PORT,MOTOR_PIN1);
	CLEAR_BIT(MOTOR_PORT,MOTOR_PIN2);
}
void stop(){
	OCR1A=0;
}
void start(){
	OCR1A=255;
}
void Open_Door(){
	start();
	LED_ON();
	clockwise();
	_delay_ms(MOTOR_ON_TIME);
	anticlockwise();
	_delay_ms(MOTOR_ON_TIME);
	stop();
	LED_OFF();

}

void initializePassword(){
	uint8 TempPassword[PASSWORD_LENGTH];
	uint8 check=0;
	while(!check){
		while (UART_recieveByte() != WAIT1);
		UART_sendByte(WAIT2);
		receivePassword(Password_Received);

		while (UART_recieveByte() != WAIT1);
		UART_sendByte(WAIT2);
		receivePassword(TempPassword);

		if (compare_passwords(Password_Received, TempPassword) == PASSWORD_MATCHED){
			UART_sendByte(WAIT1);
			UART_sendByte(PASSWORD_MATCHED);
			savePassword();
			check=1;
		}else{
			UART_sendByte(WAIT1);
			UART_sendByte(PASSWORD_MISMATCHED);
		}
	}
}

void receivePassword(uint8 * passwordArray){
	uint8 cnt;
	for (cnt=0;cnt<PASSWORD_LENGTH;cnt++){
		*(passwordArray+cnt) = UART_recieveByte();
		_delay_ms(100);
	}
}

void readPassword(){
	uint8 i;
	for (i=0;i<PASSWORD_LENGTH;i++){
		EEPROM_readByte(EEPROM_STORE_ADDREESS+i, Saved_Password+i);
	}
}

void savePassword(){
	uint8 i;
	for (i = 0; i < PASSWORD_LENGTH; i++) {
		EEPROM_writeByte(EEPROM_STORE_ADDREESS + i, Password_Received[i]);
		_delay_ms(100);
	}
}
void BZR_LED_ON(){
	SET_BIT(BUZZER_PORT,BUZZER_PIN);
	SET_BIT(LED_PORT,RED_LED_PIN );

}
void BZR_LED_OFF(){
	CLEAR_BIT(BUZZER_PORT,BUZZER_PIN);
	CLEAR_BIT(LED_PORT,RED_LED_PIN );
}
void LED_ON(){
	SET_BIT(LED_PORT,GREEN_LED_PIN );
}
void LED_OFF(){
	CLEAR_BIT(LED_PORT,GREEN_LED_PIN );
}
