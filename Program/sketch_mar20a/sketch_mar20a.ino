#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#define delayTime 250

void ledFuntion(short *);
bool LEDMODE=0;
bool BUTTONSTATUS=0;
int rep=1;

int main()
{
  DDRB = B00111000;
  short Mask1[3]= {B00000000,B00111000,B00000000};
  short Mask2[3]= {B00100000,B00010000,B00001000};
  short* Masks[2]= {Mask1, Mask2};
  int i=0;
  while(1){
    checkButton();
    if(LEDMODE){
      for(short i=0; i<rep; i++){
        ledFuntion(*(Masks+1));
      }
      LEDMODE=0;
    }
    ledFuntion(*Masks);
  }
}

void ledFuntion(short * mask){
  for(int i=0; i<3; i++){
    PORTB = *(mask+i);
    _delay_ms(delayTime);
    checkButton();
  }
}

void checkButton(){
    if(PINB==1 && !BUTTONSTATUS){
      LEDMODE=1;
      BUTTONSTATUS=1;  
    }
    if(PINB==0 && BUTTONSTATUS){
      BUTTONSTATUS=0;
    }
}

