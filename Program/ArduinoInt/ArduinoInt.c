/**
 * @file    arduinoFirmware.c
 * @author  Arturo Chinchilla S.
 * @author  Malcolm Davis S.
 * @date    March 21, 2016
 * @version 0.4
 * @brief   A simple firmware, which was written in C language using the AVR GCC compiler, 
 *          which handles some features on the Arduino, as multimode LEDS and communication 
 *          using the serial port with a Linux kernel module.
*/ 
#include <avr/io.h>
#include <util/delay.h>     // Header to use _delay_ms() function
#include <stdbool.h>        // Header to use boolean tipes in C
#include <avr/interrupt.h>  // Contain functions to manage interrupts
#define delayTime 250       // Define a delay time used in the code
#define F_CPU 16000000      // Define the Frequency of the Arduino CPU
#define BUAD 9600           // Amount of bauds (characters per second)
#define BRC ((F_CPU/16/BUAD)-1) // Baud Rate Calculate
#define RX_BUFFER_SIZE 128  // Size of Buffter used to recieve serial data
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // Provide access to I/O Registers
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))  // Provide access to I/O Registers

void ledModeZero(void);  // Manage the first LED animation, turn ON/OFF
void ledModeOne(void);   // Manage the second LED animation, LED burst
int checkButton(void);   // Manage the button actions
void serialTx(void);     // Transmit data function
void serialRx(void);     // Receive data function
char getChar(void);      // Gets the firs character, give us the option to be changed(M-> LEDMODE, R-> Rep amount)
char* getBuffChar(void);  // Returns the buffer received 
void selectionMode(void);// Select the current animation
char buffChar[RX_BUFFER_SIZE - 1];// Char with the value to change
char rxBuffer[RX_BUFFER_SIZE];  // Structure that contain the data (Buffer)
uint8_t rxReadPos = 0;   // Read position on the Buffer
uint8_t rxWritePos = 0;  // Write position on the Buffer
bool LEDMODE = 0;          // LEDMODE animation, can be 0 or 1
bool BUTTONSTATUS = 0;     // Status of the button (pressed or not)
int rep = 1;               // Amount of repetitions of the Burst Mode (Mode One)

/*
* @brief The Firmware initialization function, It keeps running, waiting interactions
*         with the Arduino such as pressing the button, change the LEDMODE, or change 
*         the number of repetitions of the burst mode.
*         Don't have return statement and don't receive parameters.
*/
void main(){
	bit_is_set(PINB,0);  // This Macro Test whether bit bit in IO register sfr is set. 
	while(1) {
		if (checkButton() == 1){  // If the button was pressed
			serialTx();
			serialRx();
			selectionMode();			
		}		
	}
}

void selectionMode(){
	serialTx();
	serialRx();
	if(LEDMODE == 0){
		ledModeZero();
	}
	if(LEDMODE == 1){
		ledModeOne();
	}
}

/*
*@brief Manage the first LEDMODE (mode zero), this mode turn on the LEDs when the button
*         is pressed and then turn off the LEDs.
*         This method don't have return statement and don't receive parameters.
*/
void ledModeZero(){
	DDRB = 0b00111001; 
	PORTB = 0b00111000; // Turn ON the LEDs in the PINs 11, 12 and 13
	_delay_ms(40);       // Make a delay time
	PORTB = 0b00000000; // Turn OFF all the LEDs
};

/*
* @brief Manage the second LEDMODE (mode one), this mode make a LEDs burst, turning on and 
*         off each LED sequentially and repeating the "animation" the number indicated, by 
*         default is one.
*         This method don't have return statement and don't receive parameters.
*/
void ledModeOne(){
	int i = 0;  // Valiable for loop
	DDRB = 0b00111001; 
	while(i < rep){
		PORTB = 0b00100000;  // Turn ON only the LED at pin 13
		_delay_ms(delayTime);
		PORTB = 0b00010000;  // Turn ON only the LED at pin 12
		_delay_ms(delayTime);
		PORTB = 0b00001000;  // Turn ON only the LED at pin 11
		_delay_ms(delayTime);
		PORTB = 0b00000000;  // Turn OFF all the LEDs
		i++;		
	}
};

/*
* @brief  Manage the interactions with the button, if the button is pressed initialize the 
*         LEDs animation, depending the LEDMODE
* @return If the button was pressed return 1, else return 0
*/
int checkButton(){
    if(PINB == 1 && !BUTTONSTATUS){  // PINB = 1 means the button is being pressed
      	BUTTONSTATUS=1;  // Button was pressed
      	return 1; 
    }
    if(!PINB && BUTTONSTATUS){  // PINB = 0 means the button isn't pressed
        BUTTONSTATUS = 0; // The button was released
      	return 0;
    }
}

/* 
* @brief Manage the data transmission by serial port, from the arduino
*        This method don't have return statement and don't receive parameters.
*/
void serialTx(void){
	DDRB = 0b00111001; 
	UBRR0H = (BRC >> 8);  // Set baud rate 
	UBRR0L = BRC;         // Set baud rate 
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0);;// Enable receiver and transmitter 
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // Set frame format: 8data, 2stop bit
	UDR0 = '1';  // Set the char to be sent
}

/* 
* @brief Manage the data reception by serial port, from the arduino
*        This method don't have return statement and don't receive parameters.
*/
void serialRx(void){
    /*Set baud rate */
	UBRR0H = (BRC >> 8);
    UBRR0L =  BRC; 
    /*Enable receiver and transmitter */     
    UCSR0B = (1 << RXEN0)  | (1 << RXCIE0);
    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    
    DDRB = (1 << PORTB0);  // The Port B Data Direction Register
    sei();  // Instruction to enable interrupts
    char first = getChar(); // Get first char from getChar function
    char second = getChar();// Get second char from getChar function
	if(first == '0'){ 
		if(second == '1'){
			LEDMODE = 1;
		}
		if(second == '0'){
			LEDMODE = 0;
		}
	}
	/*
	if(first == '1'){	
			rep = second;		
	}*/
			
}

/*
* @brief Get the character of the buffer received by serial port
* @return The last caracter received in the buffer
*/
char getChar(void){
    char ret = '\0';  // Reset variable
    if(rxReadPos != rxWritePos){    // Taking care that the positions aren't the same
        ret = rxBuffer[rxReadPos];  // Set de character readed in the buffer to the variable
        rxReadPos++;  
        if(rxReadPos >= RX_BUFFER_SIZE){  // End of te Buffer
            rxReadPos = 0;
        }
    }
    return ret;
}

char* getBuffChar(void){
    buffChar[0] = '\0';
    if(rxReadPos != rxWritePos){
    	if(rxReadPos >= RX_BUFFER_SIZE){  // End of te Buffer
            rxReadPos = 0;
        }
	    buffChar[0] = rxBuffer[rxReadPos];
	}

    return buffChar; 
}

/*
* @brief Receive 
*/
ISR(USART_RX_vect){
    rxBuffer[rxWritePos] = UDR0;  // Write the data received by the UDR0 (USART I/O Data  
                                  // Register #0) in the buffer     
    rxWritePos++;
     
    if(rxWritePos >= RX_BUFFER_SIZE)  // If the buffer is full overwrite the last data
    {
        rxWritePos = 0;
    }
}

