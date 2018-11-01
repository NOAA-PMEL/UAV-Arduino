
#include <SPI.h>                // for SD card
#include <SD.h>                 // for SD card
#include <SparkFunDS1307RTC.h>  // for RTC clock
#include <Wire.h>               // I2C for RTC
#include <SoftwareSerial.h>

SoftwareSerial SoftSerial(10, 9); // RX, TX
File root;
String DataFname;
bool GoodSD= true;
File dataFile;

const int ledPin= 13;        //sets digital out
int ledState= LOW;

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;
long CurrTimeMills;
float CurrTimeSecs;
//long CTsecs;
long NewSecs;
String TString;
long TimeOffset= 0;
String CycleData = "";

void setup() {
 Serial.begin(9600);
 Serial.println("<Arduino is ready>");
 Serial1.begin(38400);
 Serial2.begin(38400);
 SoftSerial.begin(9600);

 pinMode(ledPin, OUTPUT);
 
 rtc.begin(); // Call rtc.begin() to initialize the library
 // (Optional) Sets the SQW output to a 1Hz square wave.

 
 //Start SD card
 if (!SD.begin(8)) {
    Serial.println("SD card initialization failed!");
    //while (1);
    GoodSD= false;
  }
  //Serial.println("initialization done.");
  if (GoodSD) {
    root = SD.open("/");
    DataFname= FindNewFileName(root);
    Serial.print(DataFname + "    ");
    //dataFile = SD.open(DataFname, FILE_WRITE);
    Serial.println("done!");
  } else {
    Serial.println("SD failed...........");
  }

  // Initialise I2C communication as MASTER
  Wire.begin();


}



void loop() {
  long CT;
  static long LastCT=0;
  long Remain;
  static long LastRemain=300;
  long Delta;
  String strA="";
  static int8_t lastSecond = -1;
  static int8_t lastMinute = -1;
  static String CycleDataMin="";
  String Poll="";
  
  //ReadSer0(); 
  ReadSer1();
  ReadSer2();     // Aerosol Payload
  //ReadSoftSer();
   
  // End of the Second is here
  CT=millis();
  rtc.update(); 

  if (rtc.second() != lastSecond) // If the second has changed
  {
    //set the digital output line

        // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);

   // Now Poll the Aerosol Payload
   Poll= "Sample" + "\r\n";
   Serial2.print(Poll);
   
   
    //printTime(); // Print the new time
    CycleData+= "RTC= " + MakeRTCstring() + "\r\n";
    
    long s = rtc.second();
    long m = rtc.minute();
    long h = rtc.hour();
   
    long RTCsecs= s + 60*m + 3600*h;
    
    Serial.print("RTCsecs= ");
    Serial.println(RTCsecs);
    
    /* Serial.print("CT= ");
    Serial.println(CT);
    */
    
    float CTsecs= CT/1000.0;
    
    float diff= CTsecs + TimeOffset; 
    diff =diff - (float)RTCsecs;
    if (s == 0) {  // New Miniute: add file name and size to data record
      
      CycleData+= "File=" + DataFname + "\n\r";
      dataFile = SD.open(DataFname, FILE_READ);
      unsigned long Fsize= dataFile.size();
      CycleData+= "Fsize=" + String(Fsize) + "\n\r";
      dataFile.close();
      char charD[4];     // Now add Date Stamp
      String Dstring="";
      sprintf(charD,"%4d-", (rtc.year()+2000));
      Dstring= String(charD);
      sprintf(charD,"%02d-", rtc.month());
      Dstring+= String(charD);
      sprintf(charD,"%02d", rtc.date());
      Dstring+= String(charD);
      CycleData+= "Date=" + Dstring + "\n\r";
    
  //String strAA= "RTC TimeString= " + Tstring;
  //Serial.println(strAA);
  //printTime();
        
    }
    
    CycleData+= "Delta Time (secs) = " + String(diff,4) + "\n\r"; 
    Serial1.print(CycleData);
    CycleDataMin+= CycleData;    //  for the one minute SD card write
    CycleData="";   
    lastSecond = s; // Update lastSecond value
    
    if (m != lastMinute)   // If the minute has changed, write to SD card
    {
      dataFile = SD.open(DataFname, FILE_WRITE);
      dataFile.print(CycleDataMin);
      dataFile.close();
      lastMinute= m;
      CycleDataMin="";
    
    } 
  } 

 
  /*
  Remain= CT % 1000;
  Delta= CT - LastCT;

  if ((Remain <=  LastRemain) && (Delta > 100)) {
     //Main Cycle Starts Here, Once every second
    LastCT=CT;
    LastRemain= Remain;
    CurrTimeSecs= float(CT)/1000.0;
    CTsecs= (long) CurrTimeSecs;
    
    CycleData+= "ARD= " +  MakeTimeStr(CTsecs + TimeOffset) + "\r\n";
   
    
    // Read RTC clock
    //rtc.update();
    //printTime(); // Print the new time RTC
    CycleData+= "RTC= " + MakeRTCstring() + "\r\n";

    Serial.print(CycleData);
    
    dataFile = SD.open(DataFname, FILE_WRITE);
    dataFile.print(CycleData);
    dataFile.close();
    CycleData="";
  }
  */

//now including BME280

  unsigned int b1[24];
  unsigned int data[8];
  unsigned int dig_H1 = 0;
  for(int i = 0; i < 24; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((136+i));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);

    // Read 24 bytes of data
    if(Wire.available() == 1)
    {
      b1[i] = Wire.read();
    }
  }

  // Convert the data
  // temp coefficients
  unsigned int dig_T1 = (b1[0] & 0xff) + ((b1[1] & 0xff) * 256);
  int dig_T2 = b1[2] + (b1[3] * 256);
  int dig_T3 = b1[4] + (b1[5] * 256);
  
  // pressure coefficients
  unsigned int dig_P1 = (b1[6] & 0xff) + ((b1[7] & 0xff ) * 256);
  int dig_P2 = b1[8] + (b1[9] * 256);
  int dig_P3 = b1[10] + (b1[11] * 256);
  int dig_P4 = b1[12] + (b1[13] * 256);
  int dig_P5 = b1[14] + (b1[15] * 256);
  int dig_P6 = b1[16] + (b1[17] * 256);
  int dig_P7 = b1[18] + (b1[19] * 256);
  int dig_P8 = b1[20] + (b1[21] * 256);
  int dig_P9 = b1[22] + (b1[23] * 256);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(161);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 1 byte of data
  Wire.requestFrom(Addr, 1);
  
  // Read 1 byte of data
  if(Wire.available() == 1)
  {
    dig_H1 = Wire.read();
  }

  for(int i = 0; i < 7; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((225+i));
    // Stop I2C Transmission
    Wire.endTransmission();
    
    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);
    
    // Read 7 bytes of data
    if(Wire.available() == 1)
    {
      b1[i] = Wire.read();
    }
  }

  // Convert the data
  // humidity coefficients
  int dig_H2 = b1[0] + (b1[1] * 256);
  unsigned int dig_H3 = b1[2] & 0xFF ;
  int dig_H4 = (b1[3] * 16) + (b1[4] & 0xF);
  int dig_H5 = (b1[4] / 16) + (b1[5] * 16);
  int dig_H6 = b1[6];
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control humidity register
  Wire.write(0xF2);
  // Humidity over sampling rate = 1
  Wire.write(0x01);
  // Stop I2C Transmission
  Wire.endTransmission();
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control measurement register
  Wire.write(0xF4);
  // Normal mode, temp and pressure over sampling rate = 1
  Wire.write(0x27);
  // Stop I2C Transmission
  Wire.endTransmission();
  
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select config register
  Wire.write(0xF5);
  // Stand_by time = 1000ms
  Wire.write(0xA0);
  // Stop I2C Transmission
  Wire.endTransmission();
  
  for(int i = 0; i < 8; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((247+i));
    // Stop I2C Transmission
    Wire.endTransmission();
    
    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);
    
    // Read 8 bytes of data
    if(Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }
  
  // Convert pressure and temperature data to 19-bits
  long adc_p = (((long)(data[0] & 0xFF) * 65536) + ((long)(data[1] & 0xFF) * 256) + (long)(data[2] & 0xF0)) / 16;
  long adc_t = (((long)(data[3] & 0xFF) * 65536) + ((long)(data[4] & 0xFF) * 256) + (long)(data[5] & 0xF0)) / 16;
  // Convert the humidity data
  long adc_h = ((long)(data[6] & 0xFF) * 256 + (long)(data[7] & 0xFF));
  
  // Temperature offset calculations
  double var1 = (((double)adc_t) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
  double var2 = ((((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0) *
  (((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
  double t_fine = (long)(var1 + var2);
  double cTemp = (var1 + var2) / 5120.0;
  double fTemp = cTemp * 1.8 + 32;
  
  // Pressure offset calculations
  var1 = ((double)t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)dig_P5) * 2.0;
  var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
  var1 = (((double) dig_P3) * var1 * var1 / 524288.0 + ((double) dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
  double p = 1048576.0 - (double)adc_p;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double) dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double) dig_P8) / 32768.0;
  double pressure = (p + (var1 + var2 + ((double)dig_P7)) / 16.0) / 100;
  
  // Humidity offset calculations
  double var_H = (((double)t_fine) - 76800.0);
  var_H = (adc_h - (dig_H4 * 64.0 + dig_H5 / 16384.0 * var_H)) * (dig_H2 / 65536.0 * (1.0 + dig_H6 / 67108864.0 * var_H * (1.0 + dig_H3 / 67108864.0 * var_H)));
  double humidity = var_H * (1.0 -  dig_H1 * var_H / 524288.0);
  if(humidity > 100.0)
  {
    humidity = 100.0;
  }
  else if(humidity < 0.0)
  {
    humidity = 0.0;
  }
  
  // Output data to serial monitor
  Serial.print("Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  Serial.print("Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");
  Serial.print("Pressure : ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.print("Relative Humidity : ");
  Serial.print(humidity);
  Serial.println(" RH");

}



  void ReadSer1() {
  //static String In0="";
  char rc;
  long testl=0;
  static String In ="";
  
  while (Serial1.available() > 0) { 
     rc = Serial1.read();
     In += rc;
     if (rc == '\n') {
      ProcessSer1(In);
      In="";
    }
  }
}

/*
void ReadSoftSer() {  
  //MagC
  static String In ="";
  
  while (SoftSerial.available() > 0) { 
     char rc = SoftSerial.read();
     In += rc;
     //Serial.println(In);
     if (rc == '\n') {
      In.trim();
      ProcessMagC(In);
      In="";
    }
  }
}

*/
 
void ReadSerial2() {  
  //Payload
  static String In ="";
  
  while (Serial2.available() > 0) { 
     char rc = Serial2.read();
     In += rc;
     //Serial.println(In);
     if (rc == '\n') {
      In.trim();
      ProcessMagC(In);
      In="";
    }
  }
}



void ProcessSer1(String DataIn){
  String test3="";
  String strHH="", strMM="", strSS="";
  int intHH=0, intMM=0, intSS=0;
  long Offset1=0;

  Serial.print("A Line!!!  ");
  Serial.println(DataIn);
  test3= DataIn.substring(0,3);
  Serial.print("test3= ");
  Serial.println(test3);

  int sw= 0;
  if (test3 == "!tt") {sw =  1;}
  if (test3 == "!rt") {sw =  2;}
  if (test3 == "!33") {sw =  3;}
  
  Serial.print("sw= ");
  Serial.println(sw);

  switch (sw) {
      case 1:   //set internal clock
        if (DataIn.length() >= 9) {
          Serial.println("length good  ");
          strHH=DataIn.substring(3,5);
          intHH= strHH.toInt();    
          strMM=DataIn.substring(5,7);
          intMM= strMM.toInt();
          strSS=DataIn.substring(7,9);
          intSS= strSS.toInt();
          TimeOffset=3600* (long) intHH + 60* (long) intMM + (long) intSS;
          TimeOffset-= millis()/1000;
          Serial.print("Offset1= ");
          Serial.println(TimeOffset);
        }
        break;
      case 2: //set real time clock
        if (DataIn.length() >= 9) {
          Serial.println("length good In RTC set ");
          strHH=DataIn.substring(3,5);
          intHH= strHH.toInt();    
          strMM=DataIn.substring(5,7);
          intMM= strMM.toInt();
          strSS=DataIn.substring(7,9);
          intSS= strSS.toInt();

          int day=  rtc.day();
          int date= rtc.date();
          int month=rtc.month();
          int year= rtc.year();
          rtc.setTime(intSS, intMM, intHH, day, date, month, year);
        }
        break;
      case 3: // test subroutine call
          DoStuff();
      break;
  }

}

void DoStuff() {
  Serial.println("In the Do Stuff!!!!!!!!!!!!!!!!!");
}




void ProcessMagC(String DataIn){
  CycleData+= DataIn;
  CycleData+= "\r\n";
  //Serial.println(DataIn);
}


String MakeTimeStr(long SecsIn) {
      long SecsOut;
      long MinsOut;
      long MinsAOut;
      long HoursOut;
      String M = "";
      String S = "";
      String H = "";
      String Result;
      
      MinsAOut= SecsIn / 60;
      SecsOut= SecsIn % 60;
      
      MinsOut= MinsAOut % 60;
      //MinsOut= MinsOut % 60;
      HoursOut= MinsAOut / 60;
      
      S= String(SecsOut);
      if (SecsOut <= 9)  {
         S= "0" + S;
      }
      //S= sprintf("%2d",SecsOut);
      M= String(MinsOut);
      if (MinsOut <= 9)  {
         M= "0" + M;
      }
      H= String(HoursOut);
      if (HoursOut <= 9)  {
         H= "0" + H;
      }      
      
      Result=H + ":" + M + ":" + S;
      return Result;
      }

String MakeRTCstring() {
  char charB[4];
  String Tstring="";
  
  
  rtc.update();
  
  sprintf(charB,"%02d", rtc.hour());
  Tstring= String(charB)+":";
  
  sprintf(charB,"%02d", rtc.minute());
  Tstring+= String(charB)+":";

  sprintf(charB,"%02d", rtc.second());
  Tstring+= String(charB);
    
  //String strAA= "RTC TimeString= " + Tstring;
  //Serial.println(strAA);
  //printTime();
  return Tstring;
}
  

String FindNewFileName(File dir) {
  String Fname;
  String strIdx;
  long idx=0;
  long i;
  char charA[4];
  String strB;
  String NewName;
    
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    Fname=entry.name();
    //Serial.println(Fname);
    if (Fname.substring(0,3) == "UAS") {
      //Serial.println(Fname.substring(3,5)); 
      strIdx= Fname.substring(3,5);
      //Serial.println(strIdx);
      i= strIdx.toInt();
      //Serial.println(i);
      if (i > idx) { idx=i;}
    }
    
    entry.close();
  }
  idx++;
  //idx= idx + 12;
  //Serial.print("Final= ");
  //Serial.println(idx);
  sprintf(charA,"%02d", idx);
  strB= String(charA);
  //strA= sprintf('%02d', idx);
  //strA= "Here-> ";
  //Serial.println(strB);
  NewName= "UAS" + strB + ".DAT";
  //Serial.println(NewName);
  return NewName;
}


