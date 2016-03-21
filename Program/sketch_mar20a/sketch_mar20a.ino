#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
void ledFuntion(short *);
bool LEDMODE=0;
bool BUTTONSTATUS=0;
int rep=3;

int main()
{
  DDRB = B00111000;
  short Mask1[3]= {B00000000,B00111000,B00000000};
  short Mask2[3]= {B00100000,B00010000,B00001000};
  short* Masks[2]= {Mask1, Mask2};
  int i=0;
  while(1){
    if(LEDMODE){
      for(short i=0; i<rep; i++){
        ledFuntion(*(Masks+1));
      }
      LEDMODE=0;
    }
    ledFuntion(*Masks);
    if(PINB && !BUTTONSTATUS){
      LEDMODE=1;
      BUTTONSTATUS=1;  
    }
    if(!PINB && BUTTONSTATUS){
      BUTTONSTATUS=0;
    }
  }
}

void ledFuntion(short * mask){
  int delayTime=300;
  for(int i=0; i<3; i++){
    PORTB = *(mask+i);
    _delay_ms(delayTime);
  }
}

