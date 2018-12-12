
#include <SPI.h>                // for SD card
#include <SD.h>                 // for SD card
#include <SparkFunDS1307RTC.h>  // for RTC clock
#include <Wire.h>               // I2C for RTC
#include <SoftwareSerial.h>

// for "Office testing" of Arduino system

SoftwareSerial SoftSerial(10, 9); // RX, TX
File root;
String DataFname;
bool GoodSD= true;
File dataFile;

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
 Serial.begin(9600);      // Serial 0 or USB
 Serial.println("<Arduino is ready>");
 Serial1.begin(38400);    // Terminal to send commands
 SoftSerial.begin(9600);  // MagCompass or similar unpolled instrument
 
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
  
  //ReadSer0(); 
  ReadSer1();
  ReadSoftSer();
   
  // End of the Second is here
  CT=millis();
  rtc.update(); 

  if (rtc.second() != lastSecond) // If the second has changed
  {
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


void ProcessSer1(String DataIn){
  String test3="";
  String strHH="", strMM="", strSS="";
  String strDOM="", strDay="", strMonth="", strYYYY="";
  
  int intHH=0, intMM=0, intSS=0;
  int intY=0, intDOM=0, intDay=0, intMonth=0;
  long Offset1=0;

  Serial.print("A Line!!!  ");
  Serial.println(DataIn);
  test3= DataIn.substring(0,3);
  Serial.print("test3= ");
  Serial.println(test3);

  int sw= 0;
  if (test3 == "!tt") {sw =  1;}
  if (test3 == "!rt") {sw =  2;}
  if (test3 == "!rd") {sw =  3;}
  
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
//          Serial.print("year= ");
//          Serial.println(year);
//          
          rtc.setTime(intSS, intMM, intHH, day, date, month, year);
        }
        break;
      case 3: // Set Date
          //DoStuff();
          //Command is !rd201812113  Last digit is day of week 3=Tuesday
          //  The above date is Dec 11, 2018  (Tuesday)
          
         if (DataIn.length() >= 9) {
          Serial.println("length good In RTC set ");
          strYYYY=DataIn.substring(3,7);
          intY= strYYYY.toInt() - 2000;    
          strMonth=DataIn.substring(7,9);
          
          intMonth= strMonth.toInt();
          strDOM=DataIn.substring(9,11);
          intDOM= strDOM.toInt();
          strDay=DataIn.substring(11,12);
          intDay= strDay.toInt();
 
          int s= rtc.second();
          int m=rtc.minute();
          int h= rtc.hour();

//          Serial.print("Day= ");
//          Serial.println(intDay);
//          Serial.print("DOM= ");
//          Serial.println(intDOM);
//          Serial.print("month= ");
//          Serial.println(intMonth);
//          Serial.print("year= ");
//          Serial.println(intY);
//          
                             
          rtc.setTime(s, m, h, intDay, intDOM, intMonth, intY);
        }
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


