
#include <SPI.h>                // for SD card
#include <SD.h>                 // for SD card
#include <SparkFunDS1307RTC.h>  // for RTC clock
#include <Wire.h>               // I2C for RTC
#include <SoftwareSerial.h>
#include <Adafruit_SHT31.h>
 

SoftwareSerial SoftSerial(10, 9); // RX, TX
// 

// Configuration for this 'setup'
// Serial1 is to computer (to labview program on laptop
// Serial2 is to payload
// Serial3 is to POPS
// SoftSerial is to the HC2 temp, rh sensor

//This version also includes two T-RH sensors on an I2C multiplexer
#define TCAADDR 0x70
Adafruit_SHT31 sht31 = Adafruit_SHT31();

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
long CTdelta;
long s;

//long CTsecs;
long NewSecs;
String TString;
long TimeOffset= 0;
//String CycleData = "";
String CardData="";
String SendData="";

String inputString = "";
String inputStringPOPS="";
String LastPOPS="";
bool stringComplete = false;  // whether the string is complete
int PayBytes=0;
int POPSbytes=0;
String Disp="";
int B2=0;
int B3=0;
int B4=0;
bool I2C_T_RH=true;  // set this to false to turn off I2C T-RH




void setup() {
 Serial.begin(115200);
 Serial.println("<Arduino is ready>");
 //Serial1.begin(38400);   // to laptop
 Serial1.begin(19200);   // to laptop
 Serial2.begin(38400);   // to payload
 Serial3.begin(9600);    // POPS
 SoftSerial.begin(19200); // HC2 t,rh probe
 
 //SoftSerial.begin(9600);
 inputString.reserve(200);
 //CycleData.reserve(600);
 CardData.reserve(600);
 SendData.reserve(600);
 
 LastPOPS.reserve(120);

  pinMode(A2, OUTPUT);     
  digitalWrite(A2, HIGH);   // I2C socket power on


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
    dataFile = SD.open(DataFname, FILE_WRITE);
    Serial.println("done!");
  } else {
    Serial.println("SD failed...........");
  }

 if (I2C_T_RH) {
   tcaselect(1);
   Serial.println("SHT31 (1) test");
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      Serial.println("Couldn't find SHT31");
      //while (1) delay(1);
    }
      float testT= sht31.readTemperature();
      Serial.print("testT (again) ");
      Serial.println(testT);
      //delay(2000);
 
    tcaselect(2);
    Serial.println("SHT31 (2) test");
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      Serial.println("Couldn't find SHT31");
     // while (1) delay(1);
    }
 }

}

void tcaselect(uint8_t i) {
  if (i > 7) return;
//  Serial.print("Select ");
//  Serial.println(i);
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}


void loop() {
  long CT;
  static long LastCT=0;
  //static long CTdelta;
  //static long s;  
  long Remain;
  static long LastRemain=300;
  long Delta;
  String strA="";
  //static int8_t lastSecond = -1;
  static int8_t lastSecond;
  static int8_t lastMinute = -1;
  static String DataCard="";
  String Poll="";
  static int alter=0;
  static bool PollTime= false;
  static bool StatusTime=false;
  static bool POPSTime=false;
  static bool HC2Time=false;
  static bool PSAP_T_time=false;
  static bool PSAP_RH_time=false;
  static bool POPS_T_time=false;
  static bool POPS_RH_time=false;
  //static float PSAP_t;
  //static float PSAP_rh;
  //static float POPS_t;
  //static float POPS_rh;
 
// Aerosol Payload on Serial2 is now read inside of serialEvent2()
  
  ReadSer1();
  //ReadSer3();   Now reading the POPS with Serial Event
  ReadSoftSer();
   
  CT=millis();
  //CTdelta= CT-LastCT;
  rtc.update(); 
  s = rtc.second();
  
  if (s != lastSecond) // If the second has changed
  { 
//      uint8_t ch=1;
//      tcaselect(1);
//      float testT= sht31.readTemperature();
//      Serial.print("testT (again) ");
//      Serial.println(testT);
//      float testRH= sht31.readHumidity();
//      Serial.print("testRH (again) ");
//      Serial.println(testRH);
//
//      ch = 2;
//      tcaselect(ch);
//      testT= sht31.readTemperature();
//      Serial.print("testT (again) ");
//      Serial.println(testT);
//      testRH= sht31.readHumidity();
//      Serial.print("testRH (again) ");
//      Serial.println(testRH);

//    Serial.print("NewSec: ");
//    Serial.print(String(s) + " " + String(lastSecond));
//    Serial.print(" " + String(CT) + " " + String(LastCT));
//    Serial.println(" " + String(CTdelta));


    
    LastCT=CT;
    lastSecond = s; // Update lastSecond value
    PollTime=true;
    StatusTime=true;
    POPSTime=true;
    HC2Time=true;
    PSAP_T_time=true;
    PSAP_RH_time=true;
    POPS_T_time=true;
    POPS_RH_time=true;
    
    // change LED on output 13
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState);

    //CycleData+= "RTC= " + MakeRTCstring() + "\r\n";
    //CycleData+= MakeRTCstring() + "\r\n";
    //CycleData+= MakeRTCstring() + " ";
    CardData+= "\r\n" +  MakeRTCstring() + " ";
    SendData+= "\r\n" + MakeRTCstring() + " ";

    
    //s = rtc.second();
    long m = rtc.minute();
    long h = rtc.hour();
    long RTCsecs= s + 60*m + 3600*h;

    Serial.print("RTCsecs= ");
    Serial.println(RTCsecs);    
    //Serial.print("CT= ");
    //Serial.println(CT);
     
    float CTsecs= CT/1000.0;
    float diff= CTsecs + TimeOffset; 
    diff =diff - (float)RTCsecs;
    
    String Buffers= "B2=" + String(B2) + " B3=" + String(B3) + " B4=" + String(B4) + "\r\n";
    CardData += Buffers;
    SendData += Buffers;
    
    B2=0;
    B3=0;
    B4=0;

    if (s == 0) {  // New Miniute: add file name and size to data record
      //CycleData+= "File=" + DataFname + "\r\n";
      CardData+= "File=" + DataFname + "\r\n";
      SendData+= "File=" + DataFname + "\r\n";
      
      Serial.print("File=" + DataFname + "\r\n");
      //dataFile = SD.open(DataFname, FILE_READ);
      unsigned long Fsize= dataFile.size();
      //CycleData+= "Fsize=" + String(Fsize) + "\r\n";
      CardData+= "Fsize=" + String(Fsize) + "\r\n";
      SendData+= "Fsize=" + String(Fsize) + "\r\n";
      
      Serial.print("Fsize=" + String(Fsize) + "\r\n");
      //dataFile.close();
      char charD[4];     // Now add Date Stamp
      String Dstring="";
      sprintf(charD,"%4d-", (rtc.year()+2000));
      Dstring= String(charD);
      sprintf(charD,"%02d-", rtc.month());
      Dstring+= String(charD);
      sprintf(charD,"%02d", rtc.date());
      Dstring+= String(charD);
      //CycleData+= "Date=" + Dstring + "\r\n";
      CardData+= "Date=" + Dstring + "\r\n";
      SendData+= "Date=" + Dstring + "\r\n";

      
      Serial.print("Date=" + Dstring + "\r\n");       
    }
    
    //CycleData+= "Delta Time (secs) = " + String(diff,4) + "\r\n"; 
    //Serial1.print(CycleData);
    //Serial.print(CycleData);
    //DataCard+= CycleData;    //  for the one minute SD card write

    //Now Send data on Serial1
    Serial1.print(SendData);
    SendData="";
    
    //CycleData="";
       
    //lastSecond = s; // Update lastSecond value
   
//    //if (m != lastMinute)   // If the minute has changed, write to SD card
//    if ((s % 2) == 0 )    //now every even second
//    {
//      dataFile = SD.open(DataFname, FILE_WRITE);
//      dataFile.print(DataCard);
//      dataFile.close();
//      //lastMinute= m;
//      DataCard="";    
//    }

    //Now Print to Data Card
    //CycleData="";    // remove this when finished
     dataFile.print(CardData);
     CardData="";
     if (m != lastMinute)  {   // If the minute has changed, flush the SD card
        dataFile.flush();
        lastMinute= m;
     }

  }   // this is the end of the Start new Second If statement
 CTdelta= CT -LastCT;

   
  // the following was 400, now is 50
  
  if( PollTime && CTdelta > 50) {
    // Now Poll
    Serial.println("Polling Sample at: " + String(CTdelta));
    PollTime=false;
    Poll= "Sample";
    //Poll= "Status";
    Poll.concat("\r\n");
    Serial2.print(Poll);
  }

  // the following was 600, now is 550
  //StatusTime= false;
  if( StatusTime && CTdelta > 350) {
      // Now Poll for Status 
      Serial.println("Polling Sattus at: " + String(CTdelta));
      StatusTime=false;
      Poll= "Status";
      Poll.concat("\r\n");
      Serial2.print(Poll);
  }  

if ( HC2Time && (CT - LastCT) > 450) {
  // Now poll the HydroClip2 t,rh probe
  String PollHC2 = "{ 99RDD}\r";
  SoftSerial.print(PollHC2);
  HC2Time= false;
  
}
  
 if( POPSTime && CTdelta > 550) {
  POPSTime=false;
  //CycleData += LastPOPS;
  CardData += LastPOPS;
  if (s % 2 == 0) {          // only send every other POPS line
    SendData += LastPOPS;
  }
  
  LastPOPS=""; 
 }

//   tcaselect(1);

   if (I2C_T_RH) {
 
    
     
     if ( PSAP_T_time && CTdelta > 700){
      PSAP_T_time=false;
      tcaselect(1);
      float PSAP_t= sht31.readTemperature();
      //Serial.print( "PSAP-T=" + String(PSAP_t) + "\r\n");
      //CycleData += "PSAP-T=" + String(PSAP_t) + "\r\n";
      String i2c =  "PSAP-T=" + String(PSAP_t) + "\r\n";
      if ((s+3) % 5 == 0) {
        CardData += i2c;
        SendData += i2c;  
      } else {
        CardData += i2c;
      }      
      //Serial.print("PSAP-T (again) ");
      //Serial.println(PSAP_t);
      //String sPSAP_t= String(PSAP_t);
      //Serial.println("PSAP-T= " + sPSAP_t);
      }
  
     if ( PSAP_RH_time && (CT - LastCT) > 760){
      PSAP_RH_time=false;
      tcaselect(1);
      float PSAP_rh= sht31.readHumidity();
      //Serial.print( "PSAP-RH=" + String(PSAP_rh) +"\r\n");
      //CycleData += "PSAP-RH=" + String(PSAP_rh) +"\r\n";
      String i2c= "PSAP-RH=" + String(PSAP_rh) +"\r\n";
      if ((s+3) % 5 == 0) {
        CardData += i2c;
        SendData += i2c;  
      } else {
        CardData += i2c;
      }
      //String sPSAP_rh= String(PSAP_rh);
      //Serial.println("PSAP-RH= " + sPSAP_rh);
      }
  
  
     if ( POPS_T_time && (CT - LastCT) > 820){
      POPS_T_time=false;
      tcaselect(2);
      float POPS_t= sht31.readTemperature();
      //Serial.print( "POPS-T=" + String(POPS_t) +"\r\n");
      //CycleData +=  "POPS-T=" + String(POPS_t) +"\r\n";
      String i2c =  "POPS-T=" + String(POPS_t) +"\r\n";
        if ((s+2) % 5 == 0) {
          CardData += i2c;
          SendData += i2c;  
        } else {
          CardData += i2c;
        }
      
      
      //String sPOPS_t= String(POPS_t);
      //Serial.println("POPS-T= " + sPOPS_t);
      }
  
  
     if ( POPS_RH_time && (CT - LastCT) > 880){
      POPS_RH_time=false;
      tcaselect(2);
      float POPS_rh= sht31.readHumidity();
      //Serial.print( "POPS-RH=" + String(POPS_rh) +"\r\n");
      //CycleData += "POPS-RH=" + String(POPS_rh) +"\r\n";
      String i2c= "POPS-RH=" + String(POPS_rh) +"\r\n";
      if ((s+2) % 5 == 0) {
          CardData += i2c;
          SendData += i2c;  
        } else {
          CardData += i2c;
       }

      }

   }




}    // This is the end of the "main" loop
//}    //WHY IS THIS NEEDED   Now the end of mail loop???


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

//// Serial3 is now POPS          Now reading POPS with Serial Event
//void ReadSer3() {
//  //static String In0="";
//  char rc;
//  long testl=0;
//  static String In ="";
//  
//  while (Serial3.available() > 0) { 
//     rc = Serial3.read();
//     In += rc;
//     //Serial.println(In);
//     //if ((rc == '\r') || (rc == '\n')) {
//     if (rc == '\n') {
//      //Serial.print(In);
//      ProcessSer3(In);
//      In="";
//    }
//  }
//}
//

//void ReadSoftSer() {
//  //static String In0="";
//  char rc;
//  long testl=0;
//  static String In ="";
//  
//  while (SoftSerial.available() > 0) { 
//     rc = SoftSerial.read();
//     In += rc;
//     if (rc == '\r') {     
//      Serial.println(In);
//      ProcessHC2(In);
//      In="";
//     }  
//  }  
// }

void ReadSoftSer() {
  static uint8_t ii=0;
  static char buff[160];

    while (SoftSerial.available()) {
      int Avail=SoftSerial.available();
      if (Avail > B4) {
        B4= Avail;
      }
     
    char inChar= (char)SoftSerial.read();
    if (inChar == '\r' || ii >= 159 )  {
      buff[ii]=0;
      String HC2= String(buff);
      //PayOut.trim();
      ProcessHC2(HC2);
      //ProcessPayL(PayOut);
      ii=0;      
    } else {
      buff[ii] = inChar;
      ii++;
    }
  }
}

void ProcessHC2(String sLine) {
//Serial.println("sLine is-> " + sLine);
//String sParams[5];
int iCount, i;
//String sLine;
char aa = ';';

String AirT = getValue(sLine, aa, 5);
AirT.trim();
//Serial.println(AirT);

String RH = getValue(sLine, aa, 1);
RH.trim();
//Serial.println(RH);

String Outline="RH=" + RH + " " + "AT=" + AirT + "\r\n";
Serial.print(Outline);

 //CycleData+= Outline;
 CardData+= Outline;
 SendData+= Outline;

}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


//void serialEvent2() {
//  //Serial.println("In the Event");
//  PayBytes=Serial2.available();
//  Disp="Bytes= " + String(PayBytes);
//  //Serial.println(Disp);
//  while (Serial2.available()) {
//  //while (PayBytes) {
//    //Serial.println('Bytes= '+PayBytes);
//    // get the new byte:
//    char inChar = (char)Serial2.read();
//    // add it to the inputString:
//    inputString += inChar;
//    // if the incoming character is a newline, set a flag so the main loop can
//    // do something about it:
//    if (inChar == '\r') {
//      inputString.trim();
//      ProcessPayL(inputString);
//      inputString="";
//      stringComplete = true;
//    }
//  }
//}

// New Payload Reader with mostly char

void serialEvent2() {
  static uint8_t ii=0;
  static char buff[160];
  while (Serial2.available()) {
    int Avail=Serial2.available();
    if (Avail > B2) {
        B2= Avail;
    }
    char inChar= (char)Serial2.read();
    //if (inChar == '\r' || ii >= 159 )  {
    if (inChar == '\n' || ii >= 159 )  {
      buff[ii]=0;
      String PayOut= String(buff);
      PayOut.trim();
      //Serial.print("InReader2: " + PayOut);
      ProcessPayL(PayOut);
      ii=0;      
    } else {
      buff[ii] = inChar;
      ii++;
    }
  }
}


//POPS
void serialEvent3(){
  static uint8_t ii=0;
  static char buff[160];
  
  while (Serial3.available()) {
    int Avail=Serial3.available();
    if (Avail > B3) {
      B3= Avail;
    }
    char inChar= (char)Serial3.read();
    if (inChar == '\n' || ii >= 159 )  {
      buff[ii]=0;
      String POPSOut= String(buff);
      //POPSOut.trim();
      //Serial.println(POPSOut);
      //ProcessSer3(POPSOut + "\r\n");
      ProcessSer3(POPSOut);
      ii=0;      
    } else {
      buff[ii] = inChar;
      //Serial.print(inChar);
      ii++;
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

  //Serial.println(DataIn.substring(0,1));

  if (DataIn.substring(0,1) == "!" ) 
    {
    // This is for commands prefaced with "!" that
    // are processed by the Arduino
    
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
    }   // end of "switch" code
   }
   else 
   {
     // These are commands that are not prefaced by  "!"
     // All of these commands are passed on to the Payload
     
     DataIn.trim();
     Serial2.print(DataIn + "\r\n");  
   }
}    // end of ProcessSer1()



void ProcessSer3(String DataIn){
//Serial.print(DataIn);
//CycleData += "POP=" + DataIn;
LastPOPS= "POP=" + DataIn;
}



void DoStuff() {
  Serial.println("In the Do Stuff!!!!!!!!!!!!!!!!!");
}

void ProcessPayL(String DataIn){
   //Serial.println(String(CTdelta) +" " + DataIn);
   DataIn += "\r\n";
   CardData += DataIn;  //save all data to the SD card
   //SendData += DataIn;

  if (CTdelta > 350 && CTdelta < 450) {      // This is 'Status' data
     if (s % 5 == 0) {
      //Send only Status data at even 5 sec intervals
      SendData += DataIn;
     } else {
      // Do not send Status Data
     }
    
  }  else {
    // This is Sample Data, Send Sample data, execpt at even 5 sec intervals
    if (s % 5 == 0) {
      // Do not send 'Sample' Data    
    }  else  {
      //Send the 'Sample' Data
      SendData += DataIn;
    }
  }
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
