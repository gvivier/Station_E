/*
 * Version OK
 */
#include <stdio.h>
#include <VirtualWire.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP183.h>
#include <JeeLib.h>

//#include <Servo.h>

int  Vitesse=0;
float N_maxi=0;
int V_maxi=0;
float T_mini=99, T_maxi=99;

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

Donnees mesdonnees;
uint8_t rcvdSize = sizeof(mesdonnees);
char Message[VW_MAX_MESSAGE_LEN];


/*
	Variables pour le Barometre
 */
#define BMP183_CLK  9
#define BMP183_SDO  8  // AKA MISO
#define BMP183_SDI  6  // AKA MOSI

#define BMP183_CS   5
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);

//Servo myServo;
//int const Lumiere = A1;
//int const Pin_Servo = 12;
//int Val_Lumiere;
//int angle;

/*
 Variables pour la Girouette
 */

int const pinGirouette = A0;
int ValpinGirouette;


#define NUMDIRS 8
//float adc[NUMDIRS + 1] = {
 //   0.0, 0.365, 0.43, 0.535, 0.76, 1.045, 1.295, 1.69, 2.115, 2.59, 3.005, 3.255, 3.635, 3.94, 4.33, 4.7, 5.0
//  };
int adc[NUMDIRS]={26,45,77,118,161,197,220,256};
//char *strVals[NUMDIRS]={"W","NW","N","SW","NE","S","SE","E"};
//char *strVals[NUMDIRS] = {"ESE","ENE", "E","SSE", "SE","SSW", "S", "NNE","NE","WSW","SW","NNW","N","WNW", "W", "NW"};

const int Led_Emission = 7;
const int Led_Fonction = 11;
/*
  Variables pour le Pluviometre
 */

const int Pluviometre = 3;

volatile unsigned long Cpt_Pluv = 0;

/*
  Variables pour l' Anemometre
 */
const int Anemometre = 2;

volatile unsigned long Cpt_Anem = 0;
volatile boolean f_vent;
unsigned int Etat_Anemometre = 0;
float Rayon = 0.07;

unsigned long time;
unsigned long Calcul_Speed_Suivant;

//ISR(WDT_vect) {
//  Sleepy::watchdogEvent();
//}

int fct_Calcul_Girouette() {


  int dirOffset = 0;
  unsigned int val;
  float voltage;
  //int x = 0;
byte x,reading;
  

  val = analogRead(pinGirouette);
  val >>= 2;
  reading=val;
//  voltage = (float)val / 1024.0 * 5.0;
//Serial.println(reading);
//  x = 0;
//  while (x <= NUMDIRS + 1 )
//  {
//    if ( voltage >= adc[x] && voltage < adc[(x + 1)] )
//    {
//      break;
//    }
//    x++;
//  }
//for (x=0;x=adc[x]) && (voltage < adc[(x+1)])) { break; }
//  x = (x + dirOffset) % NUMDIRS;
  for (x=0;x<NUMDIRS;x++) {
    if (adc[x] >= reading )
      break;
  }
  x=(x+dirOffset)%8;
//    Serial.println(x);
//    Serial.println(strVals[x]);
  return (x);

}

void setup()
{
  
  vw_set_tx_pin(4);
  vw_setup(2000);     // Bits par seconde
  
  Serial.begin(9600);
  //Serial.println(F("reboot"));
  pinMode(Anemometre, INPUT);
  digitalWrite(Anemometre, HIGH);
  attachInterrupt(0, cpt_Anemometre, FALLING);

  pinMode(Pluviometre, INPUT);
  digitalWrite(Pluviometre, HIGH);
  attachInterrupt(1, cpt_Pluviometre, FALLING);

  pinMode(Led_Emission,OUTPUT);
  pinMode(Led_Fonction,OUTPUT);

//  myServo.attach(Pin_Servo);
}

void loop() {


  f_vent = false;
  digitalWrite(Led_Fonction,HIGH);
  delay(2000);
//  Val_Lumiere = analogRead(Lumiere);

//  angle = map (Val_Lumiere,0,1023,0,179);
//  Serial.print(F("Angle: "));
//  Serial.println(angle);
//  myServo.write(angle);
  
  //	Calcul Girouette
  mesdonnees.Index_V = fct_Calcul_Girouette();
  digitalWrite(Led_Fonction,LOW);

  //  Calcul Anemometre

  time = millis();
  Calcul_Speed_Suivant = time + 30000;
  f_vent = true;
  while (time < Calcul_Speed_Suivant)
  {
    time = millis();
  }
  digitalWrite(Led_Fonction,HIGH);
  mesdonnees.Vitesse = fct_Calcul_Vitesse_Vent();
  delay(2000);
  digitalWrite(Led_Fonction,LOW);
//  Serial.println(Vitesse);
  f_vent = false;

  //	Calcul Pluviometre
  digitalWrite(Led_Fonction,HIGH);
  mesdonnees.Pluie =  fct_Calcul_Pluviometre();
  digitalWrite(Led_Fonction,LOW);

  //	Calcul Temperature

  if (!bmp.begin())
  {
    Serial.println(F("Erreur init baro"));
    //while (1);
  }
  mesdonnees.Temp = bmp.getTemperature();

  //	Calcul Pression atmosphérique

  mesdonnees.Pression = bmp.getPressure() / 100;

  if ( mesdonnees.Vitesse > V_maxi )
  {
    mesdonnees.Vit_maxi = mesdonnees.Vitesse;
    V_maxi=mesdonnees.Vitesse;
  }

  if ( mesdonnees.Pluie > N_maxi )
  {
    mesdonnees.Niv_maxi = mesdonnees.Pluie;
    N_maxi=mesdonnees.Pluie;
//    Serial.println(Niv_maxi);
  }

  if ( mesdonnees.Temp < T_mini || T_mini == 99 )
  {
    mesdonnees.Temp_mini = mesdonnees.Temp;
  }
  if ( mesdonnees.Temp > T_maxi || T_maxi == 99 )
  {
    mesdonnees.Temp_maxi = mesdonnees.Temp;
  }
  
  digitalWrite(Led_Emission,HIGH);
  vw_send((uint8_t *) &mesdonnees, rcvdSize);
//  Serial.println(mesdonnees);
  vw_wait_tx();
  delay(2000);
  digitalWrite(Led_Fonction,LOW);
  digitalWrite(Led_Emission,LOW);
/*  for (byte i = 0; i < 3; ++i)
  Sleepy::loseSomeTime(60000); */

}


void cpt_Anemometre()
{
  if (f_vent)
  {
    Cpt_Anem++;
  }
}
void cpt_Pluviometre()
{
  Cpt_Pluv++;
}
int fct_Calcul_Vitesse_Vent()
{

  int Vitesse = 0;

  /*
  Formule de calcul de Vitesse
   V(m/mn)= N(t/mn) * Pi * D(mm) / 1000
   V(km/h) = V(m/mn) * 0.06
   */

  //Nb_tour = Cpt_Anem / 10;
//  Vitesse = Cpt_Anem * 12 * 3.14 * 140 * 0.06 / 1000;
  Vitesse = Cpt_Anem * 2* 3.14 * 140 * 0.06 / 1000;
//Serial.println(Cpt_Anem);
  Cpt_Anem = 0;
  return (Vitesse);
}

float fct_Calcul_Pluviometre()
{
//  Serial.println(Cpt_Pluv);

//  Niveau = Cpt_Pluv * 0.2794 / 2;
  float Niv = Cpt_Pluv * 0.2794;
//Cpt_Pluv
  return (Niv);
}








