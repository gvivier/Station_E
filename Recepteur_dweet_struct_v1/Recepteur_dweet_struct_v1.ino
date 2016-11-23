// Version avec DEBUG sur sortie série et avec LED

// Libraries
#include <SPI.h>

#include <avr/wdt.h>
#include <stdio.h>
#include <Wire.h>
#include <VirtualWire.h>

#include <WiFi.h>

char ssid[] = "Bbox-E4F52C58";          //  your network SSID (name)
char pass[] = "676C1166C2D4A45CEC433341E1A2D2";   // your network password

int status = WL_IDLE_STATUS;
char servername[] = "dweet.io"; // remote server we will connect to

#define thing_name "Station_allouis"

struct Donnees {
  float Temp;
  int Pression;
  float Pluie;
  int Vitesse;
  int Index_V;
  int Niv_maxi;
  float Temp_mini;
  float Temp_maxi;
  int Vit_maxi;
};

struct Donnees mesdonnees;
uint8_t rcvdSize = sizeof(mesdonnees);

const int Led_Erreur = 2;
const int Led_Get = 3;

//byte SD_init_OK;

int k = 0;

#define NUMDIRS 8
char *strVals[NUMDIRS] = {"Ouest", "Nord Ouest", "Nord", "Sud Ouest", "Nord Est", "Sud", "Sud Est", "Est"};

char Message[50];

WiFiClient client;

void setup(void)
{
  boolean DEBUG = true;
  pinMode(Led_Erreur,OUTPUT);
  pinMode(Led_Get,OUTPUT);
  if ( DEBUG ) {
    // Initialize
    Serial.begin(115200);
    Serial.println("Attempting to connect to WPA network...");
    Serial.print("SSID: ");
    Serial.println(ssid);
  }
digitalWrite(Led_Erreur, LOW);
  status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED ) {
    if ( DEBUG) {
      Serial.println("Couldn't get a wifi connection");

    } else {
      while (true)
        {
        digitalWrite(Led_Erreur, HIGH);
        delay(2000);
        digitalWrite(Led_Erreur, LOW);
       }
      }
    }
  if ( DEBUG ) {
    Serial.println(F("Connected to wifi"));
  }
  else {
    digitalWrite(Led_Get, HIGH);
/*    delay(2000);
    digitalWrite(Led_Get, LOW);*/
  }

  if (! client.connect(servername, 80) )
  {
    /* led2 connected failed*/
    digitalWrite(Led_Erreur, HIGH);
    delay(2000);
    digitalWrite(Led_Erreur, LOW);
    delay(1000);
    digitalWrite(Led_Erreur, HIGH);
    delay(2000);
    digitalWrite(Led_Erreur, LOW);
    if (DEBUG) {
      Serial.println(F("Connection failed"));
      return;
    }
  }
  else {
    /* led1 connected to wifi*/
      digitalWrite(Led_Get,HIGH);
      delay(2000);
      digitalWrite(Led_Get,LOW);
    if (DEBUG ) {
      Serial.println("connected");
    }
  }
  vw_set_rx_pin(8);
  vw_setup(2000);   // Bits par seconde

  vw_rx_start();
}

void loop(void)
{
  boolean DEBUG = true;
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  char *val;
  int n;


  // Start watchdog
  wdt_enable(WDTO_8S);


  // Reset watchdog
  wdt_reset();

  if ( vw_have_message() )
  {
    if (vw_get_message((uint8_t *) &mesdonnees, &rcvdSize))
    {
      client.stop();
      client.connect(servername, 80);
      /* led 3 Loop Connected */
      digitalWrite(Led_Get,HIGH);
      delay(2000);
      digitalWrite(Led_Get,LOW);
      if ( DEBUG ) {
        Serial.println("connected");
      }
      client.print(F("GET /dweet/for/"));
      client.print(thing_name);

      client.print(F("?Temperature="));
      client.print(mesdonnees.Temp);
      //Serial.println(mesdonnees.Temp,BIN);
      client.print(F("&Pression="));
      client.print(mesdonnees.Pression); 
      client.print(F("&Pluie="));
      client.print(mesdonnees.Pluie);
      client.print(F("&Vitesse_vent="));
      client.print(mesdonnees.Vitesse);
      client.print(F("&Direction_vent="));
      client.print(strVals[mesdonnees.Index_V]);
      client.print(F("&Pluie_maxi="));
      client.print(mesdonnees.Niv_maxi);
      client.print(F("&Temp_mini="));
      client.print(mesdonnees.Temp_mini);
      client.print(F("&Temp_maxi="));
      client.print(mesdonnees.Temp_maxi);
      client.print(F("&Vitesse_vent_maxi="));
      client.print(mesdonnees.Vit_maxi);
    }

    client.println(F(" HTTP/1.1"));

    client.println(F("Host: dweet.io"));
    client.println(F("Connection: close"));
    client.println(F(""));
    
    // Reset watchdog
    wdt_reset();
    
    if ( DEBUG ) {
      Serial.println(F("done."));
    }

    // Reset watchdog
    wdt_reset();

    // led 4 Close connection and disconnect
/*    digitalWrite(Led_Get, HIGH);
    delay(2000);
    digitalWrite(Led_Get, LOW);*/
    if ( DEBUG ) {
      Serial.println(F("Disconnecting"));
    }

    // Reset watchdog & disable
    wdt_reset();
    wdt_disable();


  }
  // Wait 10 seconds until next update
  delay(2000);
}
