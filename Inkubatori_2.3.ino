
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
#include <FirebaseArduino.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //lista e librarive
#include <DoubleResetDetector.h> // Open Arduino IDE and go to Sketch > Include Library > Manage Libraries. 
#include <TimeLib.h> 
#include "Adafruit_SHT31.h" //https://github.com/adafruit/Adafruit_SHT31/archive/master.zip
#include "RTClib.h" //Search for RTCLib and install RTCLib by Adafruit.
#include <Ticker.h>
Ticker ticker;
#include <Wire.h>
#include <EEPROM.h>
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

SSD1306Wire display(0x3c, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h
RTC_DS1307 RTC;

const String FirmwareVer = {"1.3"}; 

#define URL_fw_Version "/dmeqa/FM/main/Inkubatori.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/dmeqa/FM/main/Inkubatori.bin"
const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j
ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL
MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3
LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug
RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm
+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW
PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM
xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB
Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3
hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg
EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF
MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA
FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec
nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z
eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF
hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2
Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe
vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep
+OkuE6N36B9K
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);


extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

// Set these to run example.
//#define FIREBASE_HOST "espinkeu-default-rtdb.europe-west1.firebasedatabase.app"                       // database URL edhe per finger print
//#define FIREBASE_AUTH "eYkYmvbRMevyqhXTj4tj9PQEhrvrCCaSXDcaINqy"                  // secret key
#define FIREBASE_HOST "espink-ed327-default-rtdb.firebaseio.com"                       // database URL edhe per finger print
#define FIREBASE_AUTH "BOgAEAZSMinUud0sFYhZOZm0mOjDI7OwaDFMK0sJ"                  // secret key


//Reset Double Click reset
#define DRD_TIMEOUT 2
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

#define led_pin 2  // D4 drita LED ne ESP
#define motorri_pin 14      // D5
#define nxemesi_pin 12      // D6
#define lageshtia_pin 13      // D7



//String Koha,Data;
char Data[16];
char Koha[16];
char DataP[16];
char KohaP[16];
//char DataM[16];
//char KohaM[16];

int countdown;
int ora;
int selekto = 2;
float t, h;
int konstante = 0;
String eepromValW; //per Eeprom
String eepromValR;
int eepromValK;
int unixVal;
int eepromValInt;
String ID = WiFi.macAddress(); //  tring ID = WiFi.softAPmacAddress().c_str();   Identifikatori i paisjes -ne kete rast MAC - String ID = "A0:20:A6:05:E9:D9";

//----------------------Timer
int period = 1000; //millis
unsigned long kohaMillis = 0; //millis



//--------------------Koha nga Iterneti--------------------------------
void  setClock() {   // Set time via NTP, as required for x.509 validation
      drd.stop();
      ticker.attach(0.3, tick);
 
   configTime(3 * 1200, 3, "at.pool.ntp.org", "time.nist.gov"); 
   setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 3);    //Timezone
   tzset();                                         //Timezone
   
      Serial.print("Waiting for NTP time sync: ");
      time_t now = time(nullptr);
  
  while (now < 8 * 3600 * 2) {
            delay(500);
            Serial.print("*");
            now = time(nullptr); 
            
                 if (konstante == 8){ // nese nuk ka internet 
                   // digitalWrite(led_pin, HIGH); //ndalet drita LED
                    break;
                 }
        
                 if (millis() >= 30000){  //time out
                    konstante = 8; // kyqet me senzor te drites
                    break;
                 }
            }
  // Serial.println(ctime(&now));
 
 
  
  if (konstante != 8){
      //vendoset koha ne RTC
          struct tm * timeinfo;
          time(&now);
          timeinfo = localtime(&now);
          RTC.adjust(DateTime((timeinfo->tm_year + 1900), (timeinfo->tm_mon+1), (timeinfo->tm_mday), (timeinfo->tm_hour), (timeinfo->tm_min), (timeinfo->tm_sec)));

        


   FirmwareUpdate();
   }
  }
  
//--------------------Firmware update nga Iterneti--------------------------------
void FirmwareUpdate(){ 

 konstante = 0;
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
        break;
    }
  }
  String payload = client.readStringUntil('\n');
  payload.trim();
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
     //ticker.detach();
      digitalWrite(led_pin, LOW); //ndezet drita LED
  }
  else
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(led_pin, LOW); 
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    } 
  }
 }  

//--------------------------------------------------------------
void tick()
{
  //toggle state
  digitalWrite(led_pin, !digitalRead(led_pin));     // set pin to the opposite state
}

//-----------------------------------------------------------------
void OraData(){
DateTime now = RTC.now(); 

sprintf(Data, "%02d.%02d.%02d", (now.day()),(now.month()),(now.year()));
sprintf(Koha, "%02d:%02d:%02d", (now.hour()),(now.minute()),(now.second()));

//Serial.println (Data);
//Serial.println (Koha);

//Serial.println (now.unixtime());
//Serial.println ((now.unixtime()+1814400));//now + 21dite

  
 unixVal = (now.unixtime());//koha unix tani --per krahasim me kohen ne eprom
 
     if ((now.year()) != 2000){
          digitalWrite(led_pin, LOW); //kur te kyqet ne internet ndezet drita LED
         }

  displayOLED();
}

//-------------------------------------------------------------------------------------------------------------------------------
void lexongaFirebase(){

OraData(); //kete e ka qite
         Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
          
              Firebase.setFloat(ID + "/ZG", 0);
          
              Firebase.setString(ID + "/ES/1_IP", (WiFi.localIP().toString()));
              Firebase.setString(ID + "/ES/2_MAC", WiFi.macAddress());
              Firebase.setString(ID + "/ES/3_WiFi", "\"" + WiFi.SSID() + "\"");
              Firebase.set(ID + "/ES/4_WiFi_F", WiFi.RSSI());
              Firebase.setString(ID + "/ES/5_On_Ora",(Koha));
              Firebase.setString(ID + "/ES/6_On_Data",(Data));
              Firebase.setString(ID + "/ES/7_Firm_Ver",FirmwareVer);
              Firebase.setString(ID + "/ES/8_P_Ora",(KohaP));
              Firebase.setString(ID + "/ES/9_P_Data",(DataP));
              Firebase.setFloat(ID + "/ES/10_Ditet",(countdown));
              
          
             // Firebase.set(ID + "/ND1",0);                     // E ndale nderprsat nese ristartohet - nese ndalet rryma
              //Firebase.set(ID + "/ND2",0);                      // E ndale nderprsat nese ristartohet - nese ndalet rryma
             Firebase.setFloat(ID + "/SE/1_Tem", (sht31.readTemperature())+6);
             Firebase.setFloat(ID + "/SE/2_Lag", (sht31.readHumidity()));
            // Firebase.setFloat(ID + "/SE/3_Ndr", (countdown));
          
              // lexo se cili rast eshte zgjedhur
              selekto = (Firebase.getString(ID + "/T/3_S")).toInt();



             Serial.println("lexo selekto");
              Serial.println(selekto);
         
 //------------------------------------------------------------Rasti1 - me buton ON/OFF
          if (( selekto == 1 )||( selekto == 5 )){ 
              // konstante = 1;
              Serial.println("Rasti 1");
              }
         
 // -------------------------------------------------------- Rasti 2 - me sensor te drites
          else if ( selekto == 2 ){ 
                 //konstante = 8;
                 Serial.println("Rasti 2");                 
          }
          
// -------------------------------------------------------------Rasti 3 - me kohe
         else if ( selekto == 3 ){
                 // konstante = 9; 
                 Serial.println("Rasti 3");
          }
   
}

/*
//-------------------*******************Shkrimi nga eeprom    --------------------------
void shkruaj_eeprom(){

  
      
           for (int i = 0; i < 10; ++i)
         {
           EEPROM.write(i, eepromValW[i]);
          }
        EEPROM.commit();  
        delay(1000);  
}
*/
//-------------------*******************Leximi nga eeprom    --------------------------
void lexo_eeprom(){
   eepromValK = EEPROM.read(15);
  //eepromValK = Lexo0.toInt();;
  Serial.println (eepromValK); 
  delay(100);
    
    DateTime now = RTC.now();
   
    for (int i = 0; i < 10; ++i)
      {
        eepromValR += char(EEPROM.read(i));
      }
   

    eepromValInt = eepromValR.toInt();
    Serial.println(eepromValInt);

DateTime next = eepromValInt; 

sprintf(DataP, "%02d.%02d.%02d", (next.day()),(next.month()),(next.year()));
sprintf(KohaP, "%02d:%02d:%02d", (next.hour()),(next.minute()),(next.second()));

Serial.println(DataP);
Serial.println(KohaP);
countdown = (((eepromValInt-(now.unixtime()))/86400)+1); //ditet e mbetura der sa te qelin ------ 1 dite =86400   

Serial.println (countdown);  


}

//-------------SETUP--------------------------------------------------------------
void setup() {
      Serial.begin(9600);
      EEPROM.begin(512);  //Initialize EEPROM
      
   
      
      ID.remove(14, 1); ID.remove(11, 1); ID.remove(8, 1); ID.remove(5, 1); ID.remove(2, 1); // largimi i ":" nga MAC adresa
      ticker.attach(0.5, tick);
      sht31.begin(0x44);//starton njesimin e temperatures

      pinMode(led_pin, OUTPUT);
      pinMode(motorri_pin, OUTPUT);
      pinMode(nxemesi_pin, OUTPUT);
      pinMode(lageshtia_pin, OUTPUT);
      digitalWrite(nxemesi_pin, HIGH);
      digitalWrite(lageshtia_pin, HIGH);
          
//-----Koha
      RTC.begin();
      
      
   //----OLED------
      display.init();
      display.flipScreenVertically();
      
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 1, "Inkubatori v1.0");
      display.display(); // write the buffer to the display
      delay(10);

//------konfigurimi fillestar i WiFii---------------------------------------
//-----------------FILLON - Resetimi i ESP kekesa per kyqje ne AP ---------
    
if ( drd.detectDoubleReset()){ 
   ticker.attach(0.1, tick);
   //digitalWrite(led_pin, LOW); //ndezet drita LED
   
   WiFiManager wifiManager;
        if (!wifiManager.startConfigPortal(ID.c_str(),"password")) //kthimi i MAC adreses si String 
        {
            Serial.println("failed to connect and hit timeout");
            Serial.println("kyqu ne SSID: WiFi");
            delay(3000);
            //reset and try again, or maybe put it to deep sleep
            ESP.reset();
            delay(5000);
          } //if you get here you have connected to the WiFi
            Serial.println("connected...yeey :)");
            //ticker.attach(0.5, tick);
            ESP.reset(); //kontrolloje se e kam qite me u resetu pas konfigurimit te SSID
        }
        
    else{
   
//------Pret deri sa te kyqjet ne WiFi---------------------------------------
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)// exit(0); 
    {
   
      Serial.print(".");
      delay(100);
     
        if (millis() >= 10000){
            //WiFi.disconnect();
            drd.stop();
            //konstante = 8; // kyqet me senzor te drites
            //digitalWrite(led_pin, LOW);
            break;
         }
    }
}

WiFi.mode(WIFI_STA); //e ndal AP MODE

  Serial.println("u ndez");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.RSSI());
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.macAddress());

  display.clear();  // clear the display
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 1, "Inkubatori v1.0");
  display.drawString(0,21,"SSID: "+ String(WiFi.SSID()));
  display.drawString(0,37,"Sig: "+ String(WiFi.RSSI())+ "db");
  display.drawString(0,53,"IP: " + (WiFi.localIP().toString()));
 
  display.display();
  
  setClock(); //thirret koha nga interneti
  
  ticker.detach();
  
 
  
// OraData(); kete e kam ndal tani

   drd.stop(); //ketu ndalet double reset click

     //Serial.println (now.unixtime());
     //Serial.println ((now.unixtime()+1814400));//now + 21dite

  DateTime now = RTC.now(); 
  eepromValW = String(now.unixtime()+1814400);//now + 21dite

 lexo_eeprom();

 //digitalWrite(motorri_pin, HIGH);
// delay(3000);
// digitalWrite(motorri_pin, LOW);
  

delay(500);
}

//---------OLED Display-------------------------------------------------------------
void displayOLED() {
    DateTime now = RTC.now(); 

  display.clear();  // clear the display
  //display.setTextAlignment(TEXT_ALIGN_CENTER); // The coordinates define the center of the screen!
  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 1, "Inkubatori v1.0        Ditë: "+ String(countdown));
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,21,"Temp:");
  display.setFont(ArialMT_Plain_16);
  display.drawString(35,15,String(t,1)+" °C");

  display.setFont(ArialMT_Plain_10);
  display.drawString(0,37,"Lag:");
  display.setFont(ArialMT_Plain_16);
  display.drawString(35,33,String(h,1)+" %");

  display.setFont(ArialMT_Plain_10);
  display.drawString(0,53, String(Koha)+ "           " + String(Data));
    
  display.display();


 

}
//-------------Firebase connect-----------------------
void firebasereconnect()
{
  Serial.println("Trying to reconnect");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream(ID); 

  lexongaFirebase();
  }

//----------------Loop---------------------------
void loop() {
  t = sht31.readTemperature()+6;

  //Serial.println(sht31.readTemperature());
  
  h = sht31.readHumidity()+4; 
  
  OraData();
  //DateTime now = RTC.now(); 
  if (Firebase.failed())
      {
      if (WiFi.status() == WL_CONNECTED){
      
      Serial.print("setting number failed:");
      Serial.println(Firebase.error());
      firebasereconnect();
      return;
      }
}
  



//-----------------------------------------------
   if (Firebase.available()) 
      {
      FirebaseObject event = Firebase.readEvent();
      String eventType = event.getString("type");
      eventType.toLowerCase();
          
     // -----Pjesa e qelsave pa hagju bandwith  -----------------------------
          if (eventType == "put") {
          String path = event.getString("path");
          String data = event.getString("data");
            
          if (path == "/")  
             {
             JsonVariant payload = event.getJsonVariant("data");
             }
          
          
           if (path == "/ZG")
               {
                int payload = event.getString("data").toInt();
                    Serial.println(payload);
                if (payload == 1)
                  {
                  
                  //OraData();
          
                  Firebase.setFloat(ID + "/ZG", 5);
                  Firebase.setFloat(ID + "/SE/1_Tem", (t));
                  Firebase.setFloat(ID + "/SE/2_Lag", (h));
                  
              Firebase.setString(ID + "/ES/5_On_Ora",(Koha));
              Firebase.setString(ID + "/ES/6_On_Data",(Data));
              Firebase.setString(ID + "/ES/8_P_Ora",(KohaP));
              Firebase.setString(ID + "/ES/9_P_Data",(DataP));
              Firebase.setFloat(ID + "/ES/10_Ditet",(countdown));

               
                  Serial.print("Lexo te dhenat ZG: ");
                  Serial.println(payload);   
                  }
               }
          
             if (path == "/ES/11_Reboot")
                   {
                    int payload = event.getString("data").toInt();
                  
                      if (payload == 1)
                            {
                              Firebase.set(ID + "/ES/11_Reboot","0");
                                delay(50);
                                ESP.reset();
                             }
                   }
                   if (path == "/ND1/5_R1")
                   {
                    int payload = event.getString("data").toInt();
                  
                      if (payload == 1)
                            {
                              Firebase.set(ID + "/ND1/5_R1","0");
                                delay(50);
           digitalWrite(motorri_pin, HIGH);
          delay(1800);
           
                    digitalWrite(motorri_pin, LOW);
                                
                             }
                   }
                
            if (path == "/ND1/1_N1")
               {
                int payload = event.getString("data").toInt();
                // int payload = event.getInt("data");  
                
                if (payload == 0){
                      Serial.println("Deaktivizo");
                        /*
                         eepromValW = String(0000000000);     
                         //shkruaj_eeprom();
                          for (int i = 1; i < 11; ++i)
                           {
                             EEPROM.write(1, eepromValW[0]);
                            }
                            */
                           EEPROM.write(15, 0);//EEPROM.write(addr, val);
                          EEPROM.commit();  
                          delay(1000);  


                         
                                       
                      Firebase.set(ID + "/ND1/4_Kon1","0"); 
                      konstante = 0;     
                  
                       delay(1000);
                       //lexo_eeprom();
                       ESP.reset();                  
                                           
                  }
                   if (payload == 1){
                     Serial.println("Aktivizo");
                
                   EEPROM.write(15, 1);//EEPROM.write(addr, val);
                   EEPROM.commit();  
                    delay(1000);
                     
                      DateTime now = RTC.now(); 
                      eepromValW = String(now.unixtime()+1814400);//now + 21dite
                      Serial.println(eepromValW);
                      for (int i = 0; i < 10; ++i)
                           {
                             EEPROM.write(i, eepromValW[i]);
                            }
                          EEPROM.commit();  
                          delay(1000);  

                                            
                      Firebase.set(ID + "/ND1/4_Kon1","1");
                      konstante = 1;
                     
                      delay(1000);
                      // lexo_eeprom();
                       ESP.reset(); 
   
                   
                
                  }
              }
          
            /*      
            if (path == "/T/4_SNdr")
                {
                    int payload = event.getInt("data");           //ka qene keshtu
                    // int payload = event.getString("data").toInt();
                    Serial.print("Data ndriqimi: ");
                    Serial.println(payload);
                   // ndriqimi =(payload);
                }
          
                   
            if (path == "/T/3_S")
                  {
                  int payload = event.getString("data").toInt();
                  
                  if (payload == 1){
                      Serial.println("Rasti 1-vepro");
                           
                   }
                          
                  if (payload == 2){
                      Serial.println("Rasti 2-vepro");  
                
                  }
          
                 if (payload ==3){
                   Serial.println("Rasti 3");
                    
              }
          
            }*/
           }
      } 
//---------


//Serial.println (countdown);

//Serial.println (eepromValInt - 259200 - unixVal);
//------1-17 dite ------------------------------------------------
    //Temp=37.5 (min 37 - max38), Lag=50%, Rrotullimi cdo x here



  

   
 
 if ((eepromValInt-259200 > unixVal)&&(eepromValK==1))
{
    //Serial.println("1-17 dite");
    //---temperatura
      if (t < 37.3){
        //Serial.println("Nxemesi ON");
        digitalWrite(nxemesi_pin, LOW);
        digitalWrite(lageshtia_pin, LOW);
        }
          
      if (t > 37.6){
       // Serial.println("Nxemsi OFF");
        digitalWrite(nxemesi_pin, HIGH);
        digitalWrite(lageshtia_pin, HIGH);
          }
      /*    
    //---lageshtia
      if (h <50)  {//49
        Serial.println("Lageshtia ON");
        digitalWrite(lageshtia_pin, LOW);
        delay (500);
         digitalWrite(lageshtia_pin, HIGH);
         delay (5000);
          }
          /*
      if (h >49.1)  {//53
        Serial.println("Lageshtia OFF");
        digitalWrite(lageshtia_pin, HIGH);
          }
          */
                /*  if (millis() >= 30000){  //time out
                    konstante = 8; // kyqet me senzor te drites
                    break;
                 }*/

        // Serial.println (millis() - kohaMillis);        
    //-----rrotullimi-----
     if((millis() - kohaMillis) >=(3600000)){ //360000keshtu perdoret per te mos shkaktuar probleme me rastin arritjes se limitit te numrimit 36000000
      Serial.println (millis() - kohaMillis);
              kohaMillis = millis();

              
              Serial.println("Motorri ON");
              
          Firebase.setString(ID + "/SE/3_Moto_Ora",(Koha));
          delay(100);
          Firebase.setString(ID + "/SE/4_Moto_Data",(Data));
        digitalWrite(motorri_pin, HIGH);
        
delay(1800);
        Serial.println("Motorri OFF");
                    digitalWrite(motorri_pin, LOW);
                
     }
     /*
       if((millis() - kohaMillis1) >=(7000)){ //keshtu perdoret per te mos shkaktuar probleme me rastin arritjes se limitit te numrimit 36000
              kohaMillis1 = millis();
            //delay(3000);
              
              Serial.println("Motorri OFF");
                    digitalWrite(motorri_pin, LOW);
                
    
     } 
  */

}

//------------------18-21 dite ------------------------------------------------
    //Temp=37.5 ,  //Lag=70% 
    //Rrotullimi cdo x here

else if (eepromValInt < unixVal )
{
 // Serial.println("18-21 dite");

      //---temperatura
        if (t < 37.3){
            //Serial.println("Nxemsja ON(18-21)");
            digitalWrite(nxemesi_pin, HIGH);
            }
          else if (t > 37.6){
            //Serial.println("Nxemsja OFF");
            digitalWrite(nxemesi_pin, LOW);
            }
/*      
//---lageshtia
  if (h <51)  {
   // Serial.println("Lageshtia ON");
    digitalWrite(lageshtia_pin, HIGH);
      }
  if (h >54)  {
    //Serial.println("Lageshtia OFF");
    digitalWrite(lageshtia_pin, LOW);
      }

//-----rrotullimi-----
 Serial.println("Te largohet mekanizmi i rrotullimit");
*/
 
  }



delay(500);
}
