#include <Mindwave.h>

#define PIN_STEP_ENABLE 2
#define PIN_STEP1_DIRECTION 3
#define PIN_STEP1_STEP 4

#define START_GUMB 5
#define ROZA_STIKALO 6
#define CSVLOG_STIKALO 7
#define DEMO_STIKALO 9
#define START_LED 10
#define ROZA_LED 11
#define CSVLOG_LED A1
#define DEMO_LED 12
#define END_SWITCH 13
#define HALL_SENSOR A0

#define HITROST_MOTORJA 2000
#define KORAKI_MOTORJA 100 //?preveri!
#define MAX_KORAKI_MOTORJA 10000 //popravi!

#define BAUDRATE 57600

#define ST_POVPRECIJ 30

#define HALL_SENSOR_WIGGLE_ROOM 5

Mindwave mindwave;

uint8_t meritve[ST_POVPRECIJ];

void setup() {
  // put your setup code here, to run once:
  pinMode(START_GUMB,INPUT_PULLUP);
  pinMode(ROZA_STIKALO,INPUT_PULLUP);
  pinMode(CSVLOG_STIKALO,INPUT_PULLUP);
  pinMode(DEMO_STIKALO,INPUT_PULLUP);
  pinMode(START_LED,OUTPUT);
  pinMode(ROZA_LED,OUTPUT);
  pinMode(CSVLOG_LED,OUTPUT);
  pinMode(DEMO_LED,OUTPUT);
  mindwave.setupe();
  //Serial.begin(BAUDRATE);

  //home-anje roze:
  while(!digitalRead(END_SWITCH))vrtenje(HITROST_MOTORJA,0,100);
}

void loop() {
static uint16_t HALL_SENSOR_AVRG=analogRead(HALL_SENSOR); //hall senzor nastavi mirovno vrednost
static uint8_t nastavitve=ui_config();                    // nastavitve določimo in potrdimo z UI ploščo
//po stabilizaciji hall senzorja odkomentiraj:

while(analogRead(HALL_SENSOR)<(HALL_SENSOR_AVRG-HALL_SENSOR_WIGGLE_ROOM)){} //dokler je UI plošča odmaknjena naprava čaka
delay(10000); // po potrditvi konfiguracije in odložitvi ploščice ima uporabnik 10 sekund da umakne roko

while((analogRead(HALL_SENSOR)<(HALL_SENSOR_AVRG+HALL_SENSOR_WIGGLE_ROOM)) || (analogRead(HALL_SENSOR)>(HALL_SENSOR_AVRG-HALL_SENSOR_WIGGLE_ROOM))){mindwave.updatee(); ObdelavaPodatkov(nastavitve&0b100,nastavitve&0b010,nastavitve&0b001);}
if(nastavitve&0b010)Serial.println("stop"); // če smo merili pošjemo stop da python neha beležiti podatke
for(int a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
//dokler ni hall senzor še stabilen:
//mindwave.update(Serial, onMindwaveData(nastavitve&0b100,nastavitve&0b010,nastavitve&0b001));

}
//-------------------------------------------------------------------------------------------------
// funkcija za mindwave mobile in preostanek:
void ObdelavaPodatkov(bool demo,bool ali_merim, bool odpiranje) {
  //demo mode spremenljivki:
  static bool demo_flag=0; //če sem prišel do zgornje ali spodnje meje
  static bool smer=0;      // da lahko spreminjam smer vrtejna v demo mode

  //spremenljivki za povprecje:
  static bool elementi_povprecja=0; //ce jih je vec kot 30
  uint16_t povprecje=0;
 // static uint8_t meritve[ST_POVPRECIJ];
  static bool nastavitev_meritev=1;
  static uint8_t stevec_meritev=0;
  static uint8_t meditacija;

  //prvic po inicializaciji arraya meritve ga zafilamo z ničlami:
  /*if(nastavitev_meritev)
  {
    for(int a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
    nastavitev_meritev=!nastavitev_meritev;
  }*/
  
  // demo mode:
  //if(demo){demo_flag=vrtenje(HITROST_MOTORJA,1,KORAKI-MOTORJA);if(demo_flag)smer!=smer;}  
  if(demo){vrtenje(HITROST_MOTORJA,1,100);}
  else if(ali_merim||odpiranje)
  {
  //dejanska meritev:
  meditacija = mindwave.getMeditation();
    
  //za izpis na zaslonu računalnika:
  if(ali_merim){
  while(!Serial.available()){}
  Serial.println(meditacija);
  Serial.println(meritve[ST_POVPRECIJ]!=0); //s tem ve če PC izpise "aktivno povprečje" ali "povprečje zadnjih 30 meritev"
  //izracun povprecja:
  if(stevec_meritev>ST_POVPRECIJ)stevec_meritev=0;
  meritve[stevec_meritev]=meditacija;
  stevec_meritev++;
  for(int a=0;a<ST_POVPRECIJ;a++)
  {
    if(meritve[a]!=0)povprecje+=meritve[a];
    else {povprecje/=a; break;}
  }
  Serial.println(povprecje);
  }
  //regulacija ne naredi ničesar če je odpiranje=0
  //regulacija(meditacija,odpiranje); //mora bit vse po branju EEG v funkciji ker sicer nastanejo težave
  }
}
//-------------------------------------------------------------------------------------------------------
//funkcija za vrtenje motorja:
bool vrtenje(int hitrost, bool smer, int stevilo_korakov)
{
static int stepsPerSecond;
static int pozicija=0;

// spremenljivka in predznak hitrosti določata če se spremenljivka pozicija povečuje ali zmanjšuje
if(smer) stepsPerSecond = hitrost;
else stepsPerSecond = -hitrost;

//for zanka za v specifičnem obsegu:  
for(int i=stevilo_korakov;i>0;--i)
{
   if (((stepsPerSecond > 0) && (pozicija < 9534))||((stepsPerSecond < 0) && (pozicija>0))) //pogoj 9534 je zgornja meja korakov, ki jih naredi motor pri cca 6 obratih
   {
    static unsigned long nextChange = 0;
    static uint8_t currentState = LOW;

    if (stepsPerSecond == 0)
    { //if speed is 0, set the step pin to LOW to keep current position
        currentState = LOW;
        digitalWrite(PIN_STEP1_STEP, LOW);}
    else
    {
        //if stepsPerSecond is not 0, then we need to calculate the next time to change the state of the driver
        if (micros() > nextChange)
        {   //Generate steps
            if (currentState == LOW)
            {
                currentState = HIGH;
                nextChange = micros() + 30;

                if ((stepsPerSecond > 0) && (pozicija < 65535)){pozicija++;}
                else if ((stepsPerSecond < 0) && (pozicija>0)){pozicija--;}
            }
            else{currentState = LOW;nextChange = micros() + (1000 * abs(1000.0f / stepsPerSecond)) - 30;}

            //Set direction based on the sign of stepsPerSecond
            if (stepsPerSecond > 0){digitalWrite(PIN_STEP1_DIRECTION, LOW);}
            else{digitalWrite(PIN_STEP1_DIRECTION, HIGH);}

            //Write out the step pin
            digitalWrite(PIN_STEP1_STEP, currentState);}
    }}}    
        digitalWrite(PIN_STEP1_STEP, LOW); //dodatna vrstica, preventivno da drži motor pri miru  
return(pozicija>9534 || digitalRead(END_SWITCH));
        }
//------------------------------------------------------------------------------------------------
// funkcija za konfiguracijo glede na nastavitve na UI plošči
uint8_t ui_config(void)
{
static bool roza_stikalo;
static bool csvlog_stikalo;
static bool demo_stikalo;
static bool start_gumb=0;
static uint32_t millis_prej=0;

while(!start_gumb)
{
if((millis()-millis_prej)>1000)start_gumb=!(digitalRead(START_GUMB));
roza_stikalo=!digitalRead(ROZA_STIKALO);
csvlog_stikalo=!digitalRead(CSVLOG_STIKALO);
demo_stikalo=!digitalRead(DEMO_STIKALO);
digitalWrite(START_LED,start_gumb);
digitalWrite(ROZA_LED,roza_stikalo);
digitalWrite(CSVLOG_LED,csvlog_stikalo);
digitalWrite(DEMO_LED,demo_stikalo);
}
start_gumb=0;
millis_prej=millis();
Serial.println((csvlog_stikalo<<1)&(0b010));
Serial.println((roza_stikalo<<0)&(0b001));
return (demo_stikalo<<2 | csvlog_stikalo<<1 | roza_stikalo<<0);
}
