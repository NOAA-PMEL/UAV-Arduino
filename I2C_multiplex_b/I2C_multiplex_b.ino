

//   This program uses the Adafruit TCA9548A I2C multiplexer
//   The Multiplexer address is:  0x70
//   The subroutine tcaselect( int )  will switch among the 8 I2C connections to
//   the multiplexer

//   This routine is using two SHT35 Temperature/Humidity Sensors (both on  0x44)
//   This also blinks the light (Dim red) with the simple AnalogWrite() command
//        that implements a simple PWM that can only be used on some pins


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <SparkFunDS1307RTC.h>  // for RTC 


#define TCAADDR 0x70
Adafruit_SHT31 sht31 = Adafruit_SHT31();

const int ledPin1 = 11;
//const int ledPin2 = 13;

int ledState= LOW;

unsigned long tt1;
unsigned long tt2;
unsigned long ts1;
unsigned long ts2;
unsigned long tr1;
unsigned long tr2;

void setup() {
  
  pinMode(ledPin1, OUTPUT);
  //pinMode(ledPin2, OUTPUT);

  pinMode(A2, OUTPUT);     
  digitalWrite(A2, HIGH);   // I2C socket power on

  
  Serial.begin(9600);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
 
 rtc.begin(); // Call rtc.begin() to initialize the library
 tcaselect(1);
 
 Serial.println("SHT31 (2) test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  tcaselect(2);
  Serial.println("SHT31 (7) test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}


void loop() {
  //tt1=millis();
  rtc.update();  
  Serial.print("Second is: ");
  Serial.println(rtc.second());

      // change LED 
   if (ledState == LOW) {
     ledState = HIGH;
     analogWrite(ledPin1, 1);
     //ledState2  = HIGH;
   } else {
     ledState = LOW;
      analogWrite(ledPin1, 0);
   }
   //digitalWrite(ledPin1, ledState);
   //digitalWrite(ledPin2, ledState);
  tt1 = millis();
  ts1=millis();
  tcaselect(1);
  ts2=millis();
  tr1=millis();
  float t2 = sht31.readTemperature();
  tr2=millis();
  float h2 = sht31.readHumidity();
  tcaselect(2);
  float t7 = sht31.readTemperature();
  float h7 = sht31.readHumidity();

  tt2= millis();
  int delta = tt2-tt1;
  
  


  
  Serial.print("Temps  1   2:   ");
  Serial.print(t2);
  Serial.print("    ");
  Serial.println(t7);
  
  Serial.print("RH     1   2:   ");
  Serial.print(h2);
  Serial.print("    ");
  Serial.println(h7);
  
  Serial.println();
  Serial.println("Delta= " + String(delta));
  delta= ts2-ts1;
  Serial.println("SwitchDelta= " + String(delta));
  delta= tr2-tr1;
  Serial.println("ReadDelta= " + String(delta));

   
 // Serial.print("Temp-7 *C = "); Serial.println(t7);
 // Serial.print("Hum.-7 % = "); Serial.println(h7);
 // Serial.println();

  delay(1000);

}
