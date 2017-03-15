#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "MFRC522.h"
#include "stdlib.h"
#define RST_PIN 5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  4 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 
#define REDLED 0
#define GREENLED 15

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance


//WIFI ssid and password in include file
#include <passwords.h>


char* CMDCTRLaddr;
char* CMDCTRLhost;

String cardroll;



int strikeRLY=10;
String last1="1";
String last2="2";
String last3="3";
String last4="4";
String last5="5";


//init WiFi propoerties
WiFiClient client;
WiFiServer server(80);

//timer
int MCLKmsec, MCLKsec, MCLKminutes, MCLKhours;
int TMRmsec, TMRsec, TMRminutes, TMRhours;

int defaultDELAY;
bool waitDELAY;
bool secondFLAG;
bool forceUpdate;
int currentDELAY;
int CMDCTRLposition;

bool overrideFLAG;

void LEDred(){
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, HIGH);
}
void LEDgreen(){
  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, LOW);
}
void LEDoff(){
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);
}
void LEDtoggle(){
  digitalWrite(GREENLED, !digitalRead(GREENLED));
  digitalWrite(REDLED, !digitalRead(GREENLED));
}




//connect to wifi
void startWIFI() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //connect to network using cons credentials
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { //wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}



void getCMDCTRL(char* url) {
  Serial.println("localtion is" + String(CMDCTRLaddr));
  char* host = CMDCTRLhost;



  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  cardroll = "";

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    //    Serial.print("asciR");
    String line = client.readStringUntil('\r');
    Serial.print(line);
    cardroll += line;
  }

  Serial.println();
  Serial.println("closing connection");


}




void doorOPENtx(String ID) {
  
  char* host = CMDCTRLhost;



  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("SENDING DOOROPEN");

  // This will send the request to the server
  client.print(String("GET ") + "/cgi-bin/IOS/access.py?mode=doorOPENtx&ID="+ID+ " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  
  Serial.println();
  Serial.println("closing connection");


}








void MCLKreset() {
  MCLKmsec = 0;
  MCLKsec = 0;
  MCLKminutes = 0;
  MCLKhours = 0;
}

void TMRreset() {
  TMRmsec = 0;
  TMRsec = 0;
  TMRminutes = 0;
  TMRhours = 0;
}

void ticker() {
  //  Serial.println("delay length");
  //  delay(incr);
  MCLKmsec += 100;
  TMRmsec += 100;
  //  Serial.println("TICK" + TMRmsec)

  if (MCLKmsec > 1000) {
    MCLKmsec = 0;
    MCLKsec++;
    secondFLAG=false;
    //Serial.println("Second++");
  }
  if (MCLKsec > 60) {
    MCLKsec = 0;
    MCLKminutes++;
  }
  if (MCLKminutes > 60) {
    MCLKminutes = 0;
    MCLKhours++;
  }


}



bool checkAUTH(String IDtocheck){
  Serial.println("CHECKING INPUT");
  Serial.print(IDtocheck);
  bool AUTHED=false;
  int rollposition=0;
  int le = 1;
  String line ="";
  String ID="";
  while (le>0)
  {
     //find end of line
     le = cardroll.indexOf('\n',rollposition);
     //Serial.println(le);
     if(le<0)
      break;
     
     //pull line out
     line = cardroll.substring(rollposition, le);
     //Serial.println(line);
     //pull ID out (before first ,)
     //check if there even is a ,
     if(line.indexOf(',')>0)
      ID=line.substring(0,line.indexOf(','));
     //otherwise just assume only ID is on line
     else
      ID=line;
 
     Serial.println("CHECKING AGAINST");
     Serial.print(ID);
     LEDtoggle();
      
     if(IDtocheck == ID){
       Serial.println("!!!MATCH FOUND...AUTHED!!!");
       AUTHED=true;
       break;
     }

     rollposition=le+1;
  }
  last5=last4;
  last4=last3;
  last3=last2;
  last2=last1;
  last1=IDtocheck;
  
  
  return AUTHED;
}




void unlockdoor(){
  digitalWrite(strikeRLY, 0);
  return;
}

void lockdoor(){
  digitalWrite(strikeRLY, 1);
  return;
}



// Helper routine to dump a byte array as hex values to Serial
void processRFIDcard(byte *buffer, byte bufferSize) {
//  char arr[128]={0};
//  for (byte i = 0; i < bufferSize; i++) {
//    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//    Serial.print(buffer[i], HEX);
//    //Serial.print(buffer[i], DEC);
//    sprintf((arr + (i * 2)), "%2x", (buffer[i] & 0xff));
//  }
  unsigned long decValue = buffer[0];
  decValue <<= 8; decValue |= buffer[1];
  decValue <<= 8; decValue |= buffer[2];
  decValue <<= 8; decValue |= buffer[3];
  Serial.println(decValue);


  
//  Serial.printf("arr is %s\n", arr);
//  long decValue = strtol(arr, NULL, 32);
  //Serial.println(decValue);
  if(checkAUTH(String(decValue))){
    LEDgreen();
    unlockdoor();
    doorOPENtx(String(decValue));
    delay(5000);
    lockdoor();
    
  }
  else{
    Serial.println("NOT AUTHORIZED");
    LEDred();
  }
  //wait to prevent clash
  delay(500);
  
}




void setup() {
  pinMode(strikeRLY, OUTPUT);
  digitalWrite(strikeRLY, 1);
  //pinMode(9, INPUT_PULLUP);
  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);


  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  LEDred();
  delay(1000);
  LEDgreen();
  delay(1000);
  LEDred();
  delay(1000);
  LEDoff();

  
  WiFi.hostname("frontdoor");

  
  startWIFI();//Connect to WIFI network and start server

  CMDCTRLhost = "192.168.0.31";
  CMDCTRLaddr = "/espserve/CMDCTRL/cardroll.dat";
  cardroll = "";
  defaultDELAY = 3000;
  waitDELAY = true;

  MCLKmsec = 0;
  MCLKsec = 0;
  MCLKminutes = 0;
  MCLKhours = 0;

  TMRmsec = 0;
  TMRsec = 0;
  TMRminutes = 0;
  TMRhours = 0;

  Serial.println("INIT COMPLETED");
  LEDoff();
  delay(3000);
}


void loop() {
  //  Serial.println("LOOP");
  delay(100);
  ticker();
  //if wifi connection has been lost, try to reconnect
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    startWIFI();
    return;
  }
 

  if (forceUpdate||(MCLKsec % 45 == 0 && secondFLAG==false)){
    LEDred();
    getCMDCTRL(CMDCTRLaddr);
    secondFLAG=true;
    forceUpdate=false;
    LEDgreen();
    
  }

  if (MCLKsec % 2)
    LEDgreen();
  else
    LEDoff();
 

    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    

    //check to see if a card has been swiped
    if (!mfrc522.PICC_IsNewCardPresent()) {
      delay(20);
      return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
      delay(20);
      return;
    }
    // Show some details of the PICC (that is: the tag/card)
    Serial.println("CARD DETECTED");
    processRFIDcard(mfrc522.uid.uidByte, mfrc522.uid.size);
    LEDred();
    Serial.println();
    
    return;
  
  }
  Serial.println("CLIENT CONNECTION");
  
  String request = client.readStringUntil('\r');
  Serial.println(request); //serial debug output
  client.flush(); //trash data
  
 
  if (request.indexOf("UNLOCK") != -1) {
    unlockdoor();
  }


     // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    if(digitalRead(9))
      client.println("<BR><BR><h2>DOOR IS OPEN</h2>");
    else
      client.println("<BR><BR><h2>DOOR IS CLOSED</h2>");
    
    client.println("<a href='/UNLOCK'>CLICK TO UNLOCK</a>");
    client.println("<BR><BR><h2>LAST IDS</h2>");
    client.println(last1);
    client.println("<BR>");
    client.println(last2);
    client.println("<BR>");
    client.println(last3);
    client.println("<BR>");
    client.println(last4);
    client.println("<BR>");
    client.println(last5);    

    client.println("</body>");
    //client.println("<script type='text/javascript' src='http://192.168.0.31/espserve/scripts/IOSlocal.js'></script>");

    //
    //  client.println("<br><br>");
    //  client.println("<a href=\"/LED=ON\">Click here to turn the lights on</a>");
    //  client.println("<BR><BR><BR><BR><BR><BR><BR>");
    //  client.println("<a href=\"/LED=OFF\">Click to turn lights off</a>");

    client.println("</html>");

  

}
