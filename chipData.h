#include "Arduino.h"

#ifndef CHIPDATA
#define CHIPDATA

#include <avr/pgmspace.h>


typedef struct chipSpec {
  char chipName[20];
  long signature;
  int  eepromSize;
  byte defaultFuse[3]; 
} chipSpec_t;

PROGMEM chipSpec_t chipList[]={
  {"ATmega8"    , 0x1E9307,  256, 0xe1, 0xd9, 0xff} ,
  {"ATmega48A"  , 0x1E9205,  256, 0x62, 0xdf, 0xff} ,
  {"ATmega48PA" , 0x1E920A,  256, 0x62, 0xdf, 0xff} ,
  {"ATmega88A"  , 0x1E930A,  512, 0x62, 0xdf, 0xf9} ,
  {"ATmega88PA" , 0x1E930F,  512, 0x62, 0xdf, 0xf9} ,
  {"ATmega168A" , 0x1E9406,  512, 0x62, 0xdf, 0xf9} ,
  {"ATmega168PA", 0x1E940B,  512, 0x62, 0xdf, 0xf9} ,
  {"ATmega328"  , 0x1E9514, 1024, 0x62, 0xD9, 0xff} ,
  {"ATmega328P" , 0x1E950F, 1024, 0x62, 0xD9, 0xff} , 
};

#endif

