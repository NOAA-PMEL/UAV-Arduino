/************************************************
*                        *
*   HYT271               *
*   Inital Code from:    *
*   ing. Roger D'Joos    *
*/
#include "Wire.h"
#define addHYT271 0x28

String Fast_t_rh;
long Counter = 0;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
   Counter= millis();
   Fast_t_rh = ReadHYT271();
   int indx = Fast_t_rh.indexOf("|");
   String strTT= Fast_t_rh.substring(0,indx);
   String strRH = Fast_t_rh.substring(indx+1);
   Counter = millis() - Counter;
   //  Now Display
   Serial.print(millis());
   Serial.print("  ");
   String OutString= "  T= " + strTT;
   OutString += "  RH= " + strRH;
   OutString += "  Timer= " + String(Counter);
   Serial.println(OutString);
   delay(998);
}

String ReadHYT271()
{
float T271= -999;
float RH= -999;
unsigned int Answer1=0;
unsigned int Answer2=0;
byte LowByte1=0;
byte HighByte1=0;
byte LowByte2=0;
byte HighByte2=0;
String RR = "-99|-99";
  
  Wire.beginTransmission(addHYT271);
  Wire.requestFrom(addHYT271, 4);
  if (Wire.available())
  {
    HighByte1 = Wire.read();
    LowByte1 = Wire.read();
    HighByte2 = Wire.read();
    LowByte2 = Wire.read();
  }
  Wire.endTransmission();
  HighByte1 = HighByte1 & 0x3F;
  Answer1 = HighByte1 << 8 | LowByte1;
  Answer2 = HighByte2 << 8 | LowByte2 & 0xFC;
  RH = Answer1 / 163.84;
  T271 = ((Answer2 / 65536.0) * 165.0) - 40.0;  
  RR=String(T271) + "|" + String(RH);
  //Serial.println(RR);
  return RR;
}
