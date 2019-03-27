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


void setup() {
  
  pinMode(ledPin1, OUTPUT);
  //pinMode(ledPin2, OUTPUT);
  
  Serial.begin(9600);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
 
 rtc.begin(); // Call rtc.begin() to initialize the library
 tcaselect(2);
 
 Serial.println("SHT31 (2) test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  tcaselect(7);
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

  tcaselect(2);
  float t2 = sht31.readTemperature();
  float h2 = sht31.readHumidity();
  tcaselect(7);
  float t7 = sht31.readTemperature();
  float h7 = sht31.readHumidity();
  
  Serial.print("Temps  2   7:   ");
  Serial.print(t2);
  Serial.print("    ");
  Serial.println(t7);
  
  Serial.print("RH     2   7:   ");
  Serial.print(h2);
  Serial.print("    ");
  Serial.println(h7);
  
  Serial.println();
   
 // Serial.print("Temp-7 *C = "); Serial.println(t7);
 // Serial.print("Hum.-7 % = "); Serial.println(h7);
 // Serial.println();

  delay(1000);

}
