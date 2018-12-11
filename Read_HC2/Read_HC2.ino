//To test the HD2 HydroClip T and RH sensor


void setup() {

  Serial.begin(9600);
  Serial1.begin(19200);
  
  // put your setup code here, to run once:

}

void loop() {
  //First Poll the HD2
  long CT;
  static long LastCT=0;
  long Remain;
  String PollHD2 = "{ 99RDD}\r";

  CT=millis();

  ReadSer1();
  if ( (CT - LastCT) >= 1000)  {
    LastCT = CT;
    Serial.print("CT = ");
    Serial.println(CT);
    Serial1.print(PollHD2);
    // add stuff here
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
     if (rc == '\r') {     
      //Serial.println(In);
      ProcessHC2(In);
      In="";
     }
  
  }
  
}
  


void ProcessHC2(String sLine) {
//Serial.println("sLine is-> " + sLine);
String sParams[5];
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



 
   
 


