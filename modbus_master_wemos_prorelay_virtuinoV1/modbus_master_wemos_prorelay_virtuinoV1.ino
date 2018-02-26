#include <SoftwareSerial.h>
SoftwareSerial Serial100(5, 4);


// virtuino
#include <ESP8266WiFi.h>
#include "Virtuino_ESP_WifiServer.h"

WiFiServer server(8000);                      // Server port
Virtuino_ESP_WifiServer virtuino(&server);

int _modbusMasterDataTable_4_reg_1[2];
int _modbusMasterAddressTable_4_reg_1[2] = {5, 7};
byte _modbusMasterBufferSize = 0;
byte _modbusMasterState = 1;
long _modbusMasterSendTime;
byte _modbusMasterLastRec = 0;
long _modbusMasterStartT35;
byte _modbusMasterBuffer[64];
byte _modbusMasterCurrentReg = 0;
byte _modbusMasterCurrentVariable = 0;
struct _modbusMasterTelegramm {
  byte slaveId;        
  byte function;        
  int startAddres;   
  int numbeRegs;   
  int valueIndex;
};
_modbusMasterTelegramm _modbusTelegramm;
long _startTimeMasterRegs[1];
long _updateTimeMasterRegsArray[] = {1000};
byte _readWriteMasterVars[] = {3};
const unsigned char _modbusMaster_fctsupported[] = {3, 6, 16};
byte _gtv1 = 0;
byte _gtv2 = 0;

void setup()
{
Serial.begin(9600);
//Serial.println();
Serial100.begin(9600);
pinMode(2, OUTPUT);
digitalWrite(2, LOW);
for(int i=0; i<1; i++) {_startTimeMasterRegs[i] =  millis();}
/////////////////////
virtuino.DEBUG=false; 
virtuino.password="1234";
WiFi.mode(WIFI_AP);                                     // Config module as Acces point only.  Set WiFi.mode(WIFI_AP_STA); to config module as Acces point and station
boolean result = WiFi.softAP("NodeMCU", "12345678");      // SSID: NodeMCU   Password:12345678
// ---- Start WIfi server
  server.begin();

}

byte temp1 = 255;
byte temp2 = 255;
char incomingByte;   // переменная для хранения полученного байта

void loop()
{
    byte _tempVariable_byte;

_gtv1 = (_modbusMasterDataTable_4_reg_1[0]);
_tempVariable_byte = _gtv2;
if (! (_tempVariable_byte == _modbusMasterDataTable_4_reg_1[1])) {_readWriteMasterVars[0] = 6;};
_modbusMasterDataTable_4_reg_1[1] = _tempVariable_byte;



/////////////////////////////////////
switch ( _modbusMasterState ) {
    case 1:
      _nextModbusMasterQuery();
      break;
    case 2:
      pollModbusMaster();
      break;
  }
//////////////////////////////////////

//if (temp1 != _gtv1) {Serial.print("Inputs = ");Serial.println(_gtv1, BIN); temp1 = _gtv1;}
//if (temp2 != _gtv2) {Serial.print("Outputs = ");Serial.println(_gtv2, BIN); temp2 = _gtv2;}
//
//if (Serial.available() > 0) {  //если есть доступные данные
//        // считываем байт
//        incomingByte = char(Serial.read());
//        if (incomingByte == '1') _gtv2 = 1;
//        if (incomingByte == '0') _gtv2 = 0;
//    }

virtuino.run();
_gtv2 = virtuino.vDigitalMemoryRead(0);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(1)<<1);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(2)<<2);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(3)<<3);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(4)<<4);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(5)<<5);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(6)<<6);
_gtv2 = _gtv2 | (virtuino.vDigitalMemoryRead(7)<<7);
virtuino.vMemoryWrite(0, _gtv1);
virtuino.vDigitalMemoryWrite(8, !bool(_gtv1&1));
virtuino.vDigitalMemoryWrite(9, !bool(_gtv1&2));
virtuino.vDigitalMemoryWrite(10, !bool(_gtv1&4));
virtuino.vDigitalMemoryWrite(11, !bool(_gtv1&8));
virtuino.vDigitalMemoryWrite(12, !bool(_gtv1&16));
virtuino.vDigitalMemoryWrite(13, !bool(_gtv1&32));
virtuino.vDigitalMemoryWrite(14, !bool(_gtv1&64));
virtuino.vDigitalMemoryWrite(15, !bool(_gtv1&128));

}
bool _isTimer(unsigned long startTime, unsigned long period )
  {
  unsigned long currentTime;
currentTime = millis();
if (currentTime>= startTime) {return (currentTime>=(startTime + period));} else {return (currentTime >=(4294967295-startTime+period));}
  }
int modbusCalcCRC(byte length, byte bufferArray[])
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < length; i++) {
    temp = temp ^ bufferArray[i];
    for (unsigned char j = 1; j <= 8; j++) {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)   temp ^= 0xA001;
    }
  }
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  return temp;
}
void _nextModbusMasterQuery()
{
_selectNewModbusMasterCurrentReg(_modbusMasterCurrentReg, _modbusMasterCurrentVariable);
if (_modbusMasterCurrentReg == 0)  return;
_createMasterTelegramm();
_modbusMasterSendQuery();
}
void _selectNewModbusMasterCurrentReg(byte oldReg, byte oldVar)
{
bool isNeeded = 1;
if (oldReg == 0) {_selectNewModbusMasterCurrentReg(1, 0); return;}
if (!(_isTimer  ((_startTimeMasterRegs[oldReg - 1]),(_updateTimeMasterRegsArray[oldReg -1])))) {isNeeded = 0;}
if( ! isNeeded ) {if(oldReg < 1) {_selectNewModbusMasterCurrentReg(oldReg+1, 0); return;} else {_modbusMasterCurrentReg = 0; _modbusMasterCurrentVariable = 0; return;}}
if (oldVar == 0) {_modbusMasterCurrentReg = oldReg; _modbusMasterCurrentVariable = 1; return;}
byte temp;
switch (oldReg) {
case 1:
temp = 2;
 break;
 }
if (oldVar < temp) {_modbusMasterCurrentReg = oldReg; _modbusMasterCurrentVariable = oldVar +1; return;}
_startTimeMasterRegs[oldReg -1] = millis();
if(oldReg < 1) { _selectNewModbusMasterCurrentReg(oldReg+1, 0); return;} 
_modbusMasterCurrentReg = 0; _modbusMasterCurrentVariable = 0; return;
}
void _createMasterTelegramm()
{
switch (_modbusMasterCurrentReg) {
case 1:
_modbusTelegramm.slaveId = 1;
switch (_modbusMasterCurrentVariable) {
case 1:
_modbusTelegramm.function = 3;
_modbusTelegramm.startAddres = 5;
_modbusTelegramm.numbeRegs = 1;
_modbusTelegramm.valueIndex = 0;
break;
case 2:
_modbusTelegramm.function = _readWriteMasterVars[0];
_modbusTelegramm.startAddres = 7;
_modbusTelegramm.numbeRegs = 1;
_modbusTelegramm.valueIndex = 1;
_readWriteMasterVars[0] = 3;
break;
}
break;
}
}
void _modbusMasterSendQuery()
{
int intTemp;
byte currentIndex = _modbusTelegramm.valueIndex;
  _modbusMasterBuffer[0]  = _modbusTelegramm.slaveId;
  _modbusMasterBuffer[1] = _modbusTelegramm.function;
  _modbusMasterBuffer[2] = highByte(_modbusTelegramm.startAddres );
  _modbusMasterBuffer[3] = lowByte( _modbusTelegramm.startAddres );
  switch ( _modbusTelegramm.function ) {
case 3:
 _modbusMasterBuffer[4] = highByte(_modbusTelegramm.numbeRegs );
      _modbusMasterBuffer[5] = lowByte( _modbusTelegramm.numbeRegs );
      _modbusMasterBufferSize = 6;
      break;
case 6:
 switch ( _modbusMasterCurrentReg ) {
case 1 :
intTemp = _modbusMasterDataTable_4_reg_1[currentIndex];
break;
}
      _modbusMasterBuffer[4]      = highByte(intTemp);
      _modbusMasterBuffer[5]      = lowByte(intTemp);
      _modbusMasterBufferSize = 6;
      break;
}
  _modbusMasterSendTxBuffer();
  _modbusMasterState = 2;
}
void _modbusMasterSendTxBuffer()
{
 byte i = 0;
int crc = modbusCalcCRC( _modbusMasterBufferSize, _modbusMasterBuffer );
  _modbusMasterBuffer[ _modbusMasterBufferSize ] = crc >> 8;
_modbusMasterBufferSize++;
 _modbusMasterBuffer[ _modbusMasterBufferSize ] = crc & 0x00ff;
 _modbusMasterBufferSize++;
digitalWrite(2, 1 );
delay(5);
Serial100.write( _modbusMasterBuffer, _modbusMasterBufferSize );
digitalWrite(2, 0 );
Serial100.flush();
  _modbusMasterBufferSize = 0;
  _modbusMasterSendTime = millis();
}
void pollModbusMaster()
{
if (_modbusTelegramm.slaveId == 0) {   _modbusMasterState = 1;   return;}
  if (_isTimer(_modbusMasterSendTime, 1000)) {
    _modbusMasterState = 1;
    return;
  }
  byte avalibleBytes = Serial100.available();
  if (avalibleBytes == 0) return;
  if (avalibleBytes != _modbusMasterLastRec) {
    _modbusMasterLastRec = avalibleBytes;
    _modbusMasterStartT35 = millis();
    return;
  }
  if (!(_isTimer(_modbusMasterStartT35, 5 ))) return;
  _modbusMasterLastRec = 0;
  byte readingBytes = _modbusMasterGetRxBuffer();
  if (readingBytes < 5) {
    _modbusMasterState = 1;
    return ;
  }
byte exeption = validateAnswer();
  if (exeption != 0) {
 _modbusMasterState = 1;
    return;
  }
 switch ( _modbusMasterBuffer[1] ) {
 case 3:
 get_FC3(4);
break;
}
  _modbusMasterState = 1;
  return;
}
byte _modbusMasterGetRxBuffer()
{
boolean bBuffOverflow = false;digitalWrite(2, LOW );
 _modbusMasterBufferSize = 0;
  while (Serial100.available() ) {
    _modbusMasterBuffer[ _modbusMasterBufferSize ] = Serial100.read();
    _modbusMasterBufferSize ++;
    if (_modbusMasterBufferSize >= 64) bBuffOverflow = true;
  }
  if (bBuffOverflow) {return -3;}
  return _modbusMasterBufferSize;
}
byte validateAnswer()
{
uint16_t u16MsgCRC =    ((_modbusMasterBuffer[_modbusMasterBufferSize - 2] << 8) | _modbusMasterBuffer[_modbusMasterBufferSize - 1]);
  if ( modbusCalcCRC( _modbusMasterBufferSize - 2,_modbusMasterBuffer ) != u16MsgCRC ) { return 255; }
  if ((_modbusMasterBuffer[1] & 0x80) != 0) {return _modbusMasterBuffer[2] ;}
  boolean isSupported = false;
  for (byte i = 0; i < sizeof( _modbusMaster_fctsupported ); i++) {
    if (_modbusMaster_fctsupported[i] == _modbusMasterBuffer[1]) {
      isSupported = 1;

      break;
    }
  }
  if (!isSupported) {return 1;}
  return 0;
}
void get_FC3(byte table)
{
int currentIndex = _modbusTelegramm.valueIndex;
  byte currentByte = 3;
int value;
  for (int i = 0; i < _modbusTelegramm.numbeRegs; i++) {
   value = word( _modbusMasterBuffer[ currentByte],   _modbusMasterBuffer[ currentByte + 1 ]);
switch ( _modbusMasterCurrentReg ) {
case 1 :
if(table == 3) {} else {_modbusMasterDataTable_4_reg_1[currentIndex + i] =value;}
break;
}
    currentByte += 2;
  } 
}
