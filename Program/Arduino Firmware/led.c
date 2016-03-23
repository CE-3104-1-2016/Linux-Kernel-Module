#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#define delayTime 250




void ledModeZero(void);
void ledModeOne(void);
int checkButton(void);
bool LEDMODE=1;
bool BUTTONSTATUS=0;
int rep=3;



void main()
{
	DDRB = 0b00111001;
	bit_is_set(PINB,0);


	

	while(1) {
		if (checkButton() == 1){
			if(LEDMODE) {
				ledModeOne();
			}
			if(!LEDMODE) {
				ledModeZero();
			}
		}			
	}
}
void ledModeZero(){
	//checkButton();
	PORTB = 0b00111000;
	_delay_ms(5);
	PORTB = 0b00000000;
	
};
void ledModeOne(){
	int i = 0;
	while(i < rep){
		//checkButton();
		PORTB = 0b00100000;
		_delay_ms(250);
		PORTB = 0b00010000;
		_delay_ms(250);
		PORTB = 0b00001000;
		_delay_ms(250);
		PORTB = 0b00000000;
		i++;
		
	}
};

int checkButton(){
    if(PINB == 1 && !BUTTONSTATUS){
      BUTTONSTATUS=1; 
      return 1; 
    }
    if(!PB0 && BUTTONSTATUS){
      BUTTONSTATUS=0;

      return 0;
    }
}
