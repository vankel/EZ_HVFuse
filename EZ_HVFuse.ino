/*
 EZ_HVFuse 
 written by @maris_HY
 2014/05/20 
*/ 

/* 
 Original program ideas  
 http://mightyohm.com/blog/2008/09/arduino-based-avr-high-voltage-programmer/
 HVFuse.pde

 HVFuse - Use High Voltage Programming Mode to Set Fuses on ATmega48/88/168
 09/23/08  Jeff Keyzer  http://mightyohm.com                
 The HV programming routines are based on those described in the
 ATmega48/88/168 datasheet 2545M-AVR-09/07, pg. 290-297
 This program should work for other members of the AVR family, but has only
 been verified to work with the ATmega168.  If it works for you, please
 let me know!  http://mightyohm.com/blog/contact/
 */

#include "chipData.h"

// Pin Assignments
#define  VCC     7   // target VCC control
#define  WR      A0  // Analog inputs 0-5 can be addressed as digital pin
#define  BS1     A1
#define  XTAL1   A2
#define  XA0     A3
#define  XA1     A4    
#define  DATA0   A5
#define  DATA1   2
#define  DATA2   3
#define  DATA3   4
#define  DATA4   5
#define  DATA5   6
#define  DATA6   8
#define  DATA7   9

#define  BS2     10
#define  OE      11
#define  RDYBSY  12
#define  RST     13

#define  LFUSE 0
#define  HFUSE 1
#define  EFUSE 2
#define  LOCKB 3

#define  CHIPERASE B10000000
#define  WRITEFUSE B01000000
#define  WRITELOCK B00100000
#define  READSIG   B00001000
#define  READFUSE  B00000100

const byte data_pin[8] = {DATA0, DATA1, DATA2, DATA3,  
                      DATA4, DATA5, DATA6, DATA7};

const char * fuseName[] = {"Low  Fuse Bits", "High Fuse Bits", 
                          "Ext  Fuse Bits","Lock Bits"};
byte fuseValue[4] = {0xdf,0x62,0xff,0xff};

byte chipId = 255;
long signature = 0;
chipSpec_t const *chip;

void setup()  // run once, when the sketch starts
{
  byte data;
  pinMode(RST, OUTPUT);
  digitalWrite(VCC, HIGH);

  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, LOW);

  Serial.begin(9600);
  for(byte i=0;i<3;i++){
    Serial.println();
  }
  Serial.println(F(".............................."));
  Serial.println(F("EZ HV Fuse"));
  Serial.println(F(" for ATmega48/88/168/328"));
  Serial.println();
  Serial.println(F("Type D and Enter Key to Detect a chip."));
  Serial.println();
  while(1){
    while(Serial.available() == 0);
    data = Serial.read();
    if((data == 'D') || (data == 'd'))
      break;
  }
  standby();
  ditectChip();    
}

void loop()  // run over and over again
{
  byte cmd;
  standby();

  displayCommandList();
  cmd = input1byteCmd();

  switch (cmd) {
  case 'C':  case 'c':
    askChipErase();
    break;
  case 'W':  case 'w':
    askWriteDefaultFuse();
  case 'R':  case 'r':
    displayFuse();
    break;
  case 'S':  case 's':
    displaySignature();
    break;
  case 'H':  case 'h':
    askWriteFuse(HFUSE);
    break;
  case 'L':  case 'l':
    askWriteFuse(LFUSE);
    break;
  case 'E':  case 'e':
    askWriteFuse(EFUSE);
    break;
  case 'K':  case 'k':
    askWriteLockBits();
    break;
  case 'D':  case 'd':
    ditectChip();
    break;
  } 
  Serial.println();
}

void displayCommandList(){
  Serial.print(F("Target Chip:"));
  printTarget();
  Serial.println();
  Serial.println(F("D:Detect Chip."));
  Serial.print(F("W:Write default value to fuse bytes. (l:"));
  printHEX(fuseValue[LFUSE]);
  Serial.print(F(" h:"));
  printHEX(fuseValue[HFUSE]);
  Serial.print(F(" e:"));
  printHEX(fuseValue[EFUSE]);
  Serial.println(F(" )"));
  Serial.println(F("R:Read fuse bytes."));
  Serial.println(F("C:Chip erase."));
  Serial.println(F("S:read Signature bytes."));
  Serial.println(F("L: H: E: input fuse value and write it."));
  Serial.println(F("K: input locK bits value and write it."));
  Serial.println();
  Serial.print(F("Type Command and Enter."));
  Serial.println();
}

void standby(){
  digitalWrite(RST, HIGH);
  digitalWrite(VCC, LOW);

  pinMode(WR, INPUT);
  pinMode(OE, INPUT);
  pinMode(BS1, INPUT);
  pinMode(BS2, INPUT);
  pinMode(XTAL1, INPUT);
  pinMode(XA0, INPUT);
  pinMode(XA1, INPUT);

  setDataPinInput();
}

void init_pins(){
  digitalWrite(VCC, HIGH); // Apply VCC to start programming process
  
  pinMode(WR, OUTPUT);
  digitalWrite(WR, HIGH);

  pinMode(OE, OUTPUT);
  digitalWrite(OE, HIGH);
  
  pinMode(BS1, OUTPUT);
  pinMode(BS2, OUTPUT);
  pinMode(XTAL1, OUTPUT);
  pinMode(XA0, OUTPUT);
  pinMode(XA1, OUTPUT);

  digitalWrite(BS1, LOW);
  digitalWrite(BS2, LOW);
  digitalWrite(XTAL1, LOW);
  digitalWrite(XA0, LOW);
  digitalWrite(XA1, LOW);

  setDataPinOutput();
    
  digitalWrite(RST, LOW);
  delay(100);
}

void askChipErase()
{
  byte ans;
  flushSerial();
  Serial.println(F("The chip will be erased. Are you sure? (Y/N)"));
  ans = input1byteCmd();
  switch(ans) {
  case 'Y': case 'y':
    chipErase();
    Serial.println(F("Chip was erased!")); 
    break;
  }
}  


void askWriteDefaultFuse()
{
  byte ans;
  flushSerial();
  Serial.println(F("Default values will be written to fuses. Are you sure? (Y/N)"));
  ans = input1byteCmd();
  switch(ans) {
  case 'Y': case 'y':
    Serial.println(F("Now writeing Fuse!"));
    for(int i = 0;i<3;i++){
      writefuse(i, fuseValue[i]);
    }
    Serial.println(F("Finish!"));
  }
}

void askWriteLockBits()
{
  byte ans;
  byte value;
  Serial.println(F("Input Lock bits value."));
  flushSerial();
  value = inputHEX();
  Serial.println();
  Serial.println(F("Lock bits can only be cleared by executing Chip Erase."));
  Serial.println(F("Lock bits will be written. Are you sure? (Y/N)"));
  flushSerial();
  ans = input1byteCmd();
  switch(ans) {
  case 'Y': case 'y':
    Serial.println(F("Now writeing lock bits!"));
    writeLockBits(value);
    Serial.println(F("Finish!"));
  }
}

void displayFuse()
{
  for(int i = 0;i<4;i++){
    Serial.print(fuseName[i]);
    Serial.write(':');
    printHEX(readfuse(i));
    Serial.println();
  }
}

void displaySignature()
{
  int i;
  signature = 0;
  init_pins();
  for(i = 0;i<3;i++){
    signature = (signature << 8 )+ readSignature(i);
  }
  Serial.print(F("Signature:"));
  Serial.println(signature,HEX);
}

void ditectChip()
{
  int i;
  displaySignature();
  for(i = 0; i < (int)(sizeof(chipList)/sizeof(chipSpec_t)); i++){
    chip = &chipList[i];
    if(signature == (long)pgm_read_dword(&chip->signature)) {
      chipId = i;
      break;
    }
  }
  if(chipId == 255){
    Serial.println(F("Chip Unknown. Check the circit."));
    while(1);        
  } else {
    for(i=0;i<3;i++){
      fuseValue[i]= pgm_read_byte(&chip->defaultFuse[i]);
    }
  }
}

void askWriteFuse(byte fuse)
{
  byte ans;
  byte value;
  Serial.print(F("Input "));
  Serial.print(fuseName[fuse]);
  Serial.println(F(" value"));
  flushSerial();
  value = inputHEX();
  Serial.println();
  Serial.print(fuseName[fuse]);
  Serial.print(F(" will be written "));
  printHEX(value);
  Serial.println(F(". Are you sure? (Y/N)"));
  flushSerial();
  ans = input1byteCmd();
  switch(ans) {
  case 'Y': case 'y':
    Serial.println(F("Now writeing Fuse!"));
    writefuse(fuse, value);
    Serial.println(F("Finish!"));
  }
}


// high level functions

void chipErase()
{
  init_pins();
  delay(1);
  setDataPinOutput();
  loadCommand(CHIPERASE);  // Send command to read signature.
  wrPulse();
  while(digitalRead(RDYBSY)==LOW);
  delay(1);  
}

void writefuse(byte targetFuse, byte fuseValue)  // write high or low fuse to AVR
{
  init_pins();
  delay(1);
  setDataPinOutput();
  loadCommand(WRITEFUSE);  // Send command to enable fuse programming mode
  loadDataLowByte(fuseValue);   // Send fuse data.
  
  switch (targetFuse) {
  case LFUSE:
    digitalWrite(BS1, LOW);
    digitalWrite(BS2, LOW);
    break;
  case HFUSE:
    digitalWrite(BS1, HIGH);
    digitalWrite(BS2, LOW);
    break;
  case EFUSE:
    digitalWrite(BS1, LOW);
    digitalWrite(BS2, HIGH);
    break;
  }    
  wrPulse();
  while(digitalRead(RDYBSY)==LOW);
  delay(1);
}

void writeLockBits(byte value) 
{
  init_pins();
  delay(1);
  setDataPinOutput();
  loadCommand(WRITELOCK);  // Send command to enable fuse programming mode
  loadDataLowByte(value);  // Send fuse data.
  wrPulse();
  while(digitalRead(RDYBSY)==LOW);
  delay(1);
}

byte readfuse(byte targetFuse)
{  
  byte data = 0;

  init_pins();
  delay(1);
  setDataPinOutput();
  loadCommand(READFUSE);  // Send command to read fuse mode.
  setDataPinInput();
  delay(1);  
  digitalWrite(OE, LOW);

  switch (targetFuse) {
  case LFUSE:
    digitalWrite(BS1, LOW);
    digitalWrite(BS2, LOW);
    break;
  case HFUSE:
    digitalWrite(BS1, HIGH);
    digitalWrite(BS2, HIGH);
    break;
  case EFUSE:
    digitalWrite(BS1, LOW);
    digitalWrite(BS2, HIGH);
    break;
  case LOCKB:
    digitalWrite(BS1, HIGH);
    digitalWrite(BS2, LOW);
    break;
  }    
  delay(1);  
  data = getData();
  digitalWrite(OE, HIGH);
  delay(1);  
  
  return data;
}

byte readSignature(byte address)
{  
  byte data = 0;

  init_pins();
  delay(1);
  setDataPinOutput();
  loadCommand(READSIG);  // Send command to read signature.
  loadAddressLowByte(address);  // Send command to read signature.
  setDataPinInput();
  delay(1);  
  digitalWrite(OE, LOW);
  digitalWrite(BS1, LOW);
  delay(1);  
  data = getData();
  digitalWrite(OE, HIGH);
  delay(1);  
  
  return data;
}

///////////////////////////////////////
// middle layer functions

void loadCommand(byte command)  // A
{
  setDataPinOutput();
  digitalWrite(XA1, HIGH);
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
  setData(command);
  xtal1Pulse();
}

void loadAddressLowByte(byte adress) // B 
{
  digitalWrite(XA1, LOW);
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
  setData(adress);
  xtal1Pulse();
}

void loadDataLowByte(byte data) // C
{
  digitalWrite(XA1, LOW);
  digitalWrite(XA0, HIGH);
  setData(data);  
  xtal1Pulse();
}

///////////////////////////////////////
// Low layer functions

void setDataPinInput(){
  for(byte i=0;i<8;i++){
    pinMode(data_pin[i],INPUT);
  }
}  

void setDataPinOutput(){
  for(byte i=0;i<8;i++){
    pinMode(data_pin[i],OUTPUT);
    digitalWrite(data_pin[i],LOW);
  }
}


void xtal1Pulse()
{
  digitalWrite(XTAL1, HIGH);  // positive pulse XTAL1
  delay(1);
  digitalWrite(XTAL1, LOW);
}

void wrPulse()
{
  delay(1);
  digitalWrite(WR, LOW);  // negative pulse WR
  delay(1);
  digitalWrite(WR, HIGH);
}


void setData(byte data){
  byte mask = 1;
  for(byte i=0;i<8;i++){
    if(data & mask){
      digitalWrite(data_pin[i],HIGH);
    } else {
      digitalWrite(data_pin[i],LOW);
    }
    mask = mask << 1;
  }
}

byte getData(){
  byte data = 0;
  byte mask = 1;
  for(byte i=0;i<8;i++){
    if(digitalRead(data_pin[i]) == HIGH){
      data = data + mask;
    }
    mask = mask << 1;
  }
  return data;
}

void printHEX(byte data){
  if(data < 16){
    Serial.print("0");
  }
  Serial.print(data,HEX);
}

byte input1byteCmd(){
  byte data;
  flushSerial();
  Serial.print(">");
  while(1){
    while(Serial.available() == 0);
    data = Serial.read();
    if(((data >= 'A') && (data <='Z')) || ((data >= 'a') && (data <='z'))){
      Serial.write(data);
      Serial.println();
      break;
    }
  }
  return data; 
}

int inputHEX(){
  int i=0;
  int data;
  Serial.print(F(">"));
  while(1){
    while(Serial.available() == 0);
    data = Serial.read();
    if((data >= '0') && (data <= '9')){
      Serial.write(data);
      i = i * 16 + (data - '0');
    } else if((data >= 'A') && (data <= 'F')){
      Serial.write(data);
      i = i * 16 + (data - 'A') + 10;
    } else if((data >= 'a') && (data <= 'f')){
      Serial.write(data);
      i = i * 16 + (data - 'a') + 10;
    } else if((data == 0x0d) || (data == 0x0a)){
      break;
    }
  }  
  return i;
}

void flushSerial()
{
  delay(10);
  while(Serial.available() > 0){
    byte data = Serial.read();
    delay(10);
  }
}

void printTarget(){
  char c;
  char const *f = chip->chipName;
  while(1){
    c = pgm_read_byte(f);
    if(c == (char)NULL) break;
    Serial.write(c);
    f++;
  }
  Serial.println();
}
