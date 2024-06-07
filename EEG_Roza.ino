//popravi loop, hall pa mindwave zadeve lp


//#include <Mindwave.h>
#include <SoftwareSerial.h>

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

#define BAUDRATE 57600

#define SAFETY_CAKANJE 2000 //čas po odložitvi v ms
#define DEMO_CAKANJE 1000   //čas pavze pri demo načinu

//za odpiranje DEBUG izpisov na serijcu spremeni v 1:
#define DEBUG_GENERIC 1
#define DEBUG_MOTOR 0
#define DEBUG_MERITVE 0
#define DEBUG_EEG 0

SoftwareSerial bluetooth(A2,8);
//Mindwave mindwave;

const byte ST_POVPRECIJ=30;
uint16_t meritve[ST_POVPRECIJ];
byte umirjenost=0;
byte *kazalec=&umirjenost;
byte nastavitve;
//uint16_t povprecje=0;

byte mask_demo=0b100;
byte mask_log=0b010;
byte mask_roza=0b001;


// checksum variables
byte generatedChecksum = 0;
byte checksum = 0; 
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
//byte attention = 0;
byte meditation = 0;

// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_STEP_ENABLE,OUTPUT);
  pinMode(PIN_STEP1_DIRECTION,OUTPUT);
  pinMode(PIN_STEP1_STEP,OUTPUT);
  
  pinMode(START_GUMB,INPUT_PULLUP);
  pinMode(ROZA_STIKALO,INPUT_PULLUP);
  pinMode(CSVLOG_STIKALO,INPUT_PULLUP);
  pinMode(DEMO_STIKALO,INPUT_PULLUP);
  pinMode(START_LED,OUTPUT);
  pinMode(ROZA_LED,OUTPUT);
  pinMode(CSVLOG_LED,OUTPUT);
  pinMode(DEMO_LED,OUTPUT);

  pinMode(HALL_SENSOR,INPUT);
  pinMode(END_SWITCH,INPUT_PULLUP);
  
  bluetooth.begin(BAUDRATE);
  //mindwave.setupe();
  Serial.begin(BAUDRATE);

  //home-anje roze:
  zapiranje_roze();
}

void loop() {

static uint16_t HALL_SENSOR_AVRG=analogRead(HALL_SENSOR); //hall senzor nastavi mirovno vrednost
static uint16_t HALL_SENSOR_WIGGLE_ROOM=70;
static bool first_nastavitve=1;

if(first_nastavitve){nastavitve=ui_config();first_nastavitve=!first_nastavitve;}

while(analogRead(HALL_SENSOR)<=(HALL_SENSOR_AVRG-HALL_SENSOR_WIGGLE_ROOM)){
  #if DEBUG_GENERIC
  Serial.print("CAKAM DA ODLOZIS UI PLATO | HALL_SENOZOR:");
  Serial.print(analogRead(HALL_SENSOR));
  Serial.print("Povprečje HALL: ");
  Serial.print(HALL_SENSOR_AVRG);
  Serial.print(" +/- ");
  Serial.println(HALL_SENSOR_WIGGLE_ROOM);
  #endif
  } //dokler je UI plošča odmaknjena naprava čaka

delay(SAFETY_CAKANJE); // po potrditvi konfiguracije in odložitvi ploščice ima uporabnik 10 sekund da umakne roko

while((analogRead(HALL_SENSOR)>=(HALL_SENSOR_AVRG-HALL_SENSOR_WIGGLE_ROOM)) ){
#if DEBUG_GENERIC
Serial.print("GARBAM  parametri:");
Serial.print((nastavitve&mask_demo)==4);
Serial.print((nastavitve&mask_log)==2);
Serial.println((nastavitve&mask_roza)==1); 
#endif
ObdelavaPodatkov((nastavitve&mask_demo)==4,(nastavitve&mask_log)==2,(nastavitve&mask_roza)==1);
}
#if DEBUG_GENERIC
Serial.println("NE GARBAM");
#endif
if((nastavitve&mask_log)==2)Serial.println("stop"); // če smo merili pošjemo stop da python neha beležiti podatke
for(int a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
zapiranje_roze();
nastavitve=ui_config();

}
//-----------------------------------------------------------------------------------------------
// funkcija za mindwave mobile in preostanek:
int onMindwaveData()
{
//Serial.print("umirjenost loop: ");
//umirjenost=mindwave.meditation();
//Serial.println(umirjenost);
//Serial.println("bruh");
//*kazalec=mindwave.meditation();
}
//---------------------------------------------------------------------------------------------
byte ReadOneByte() {
  int ByteRead;

  while(!bluetooth.available());
  ByteRead = bluetooth.read();

#if DEBUG_EEG
  Serial.println(ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}
void EEG()
{
  

  // Look for sync bytes
  if(ReadOneByte() == 170) {
    if(ReadOneByte() == 170) {

      payloadLength = ReadOneByte();
      if(payloadLength > 169)                      //Payload length can not be greater than 169
          return;

      generatedChecksum = 0;        
      for(int i = 0; i < payloadLength; i++) {  
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }   

      checksum = ReadOneByte();                      //Read checksum byte from stream      
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

        if(checksum == generatedChecksum) {    

        poorQuality = 200;
       // attention = 0;
        //meditation = 0;
        *kazalec=0;
        for(int i = 0; i < payloadLength; i++) {    // Parse the payload
          switch (payloadData[i]) {
          case 2:
            i++;            
            poorQuality = payloadData[i];
            bigPacket = true;            
            break;
          case 4:
            i++;
            //attention = payloadData[i];                        
            break;
          case 5:
            i++;
            *kazalec = payloadData[i];
            break;
          case 0x80:
            i = i + 3;
            break;
          case 0x83:
            i = i + 25;      
            break;
          default:
            break;
          } // switch
        } // for loop

#if DEBUG_EEG

        // *** Add your code here ***

        if(bigPacket) {
          Serial.print("PoorQuality: ");
          Serial.println(poorQuality, DEC);
          Serial.print(" Med: ");
          Serial.println(*kazalec, DEC);
          //Serial.print(" Time since last packet: ");
          //Serial.print(millis() - lastReceivedPacket, DEC);
          //lastReceivedPacket = millis();
          Serial.print("\n");
                    
        }
#endif        
        bigPacket = false;        
      }
      else {
        // Checksum Error
      }  // end if else for checksum
    } // end if read 0xAA byte
  } // end if read 0xAA byte
}
//----------------------------------------------------------------------------
// funkcija za obdelavo podatkov:
void ObdelavaPodatkov(bool demo,bool ali_merim, bool odpiranje) {
  //demo mode spremenljivki:
  static bool demo_flag=0; //če sem prišel do zgornje ali spodnje meje
  static bool smer=1;      // da lahko spreminjam smer vrtejna v demo mode
 
 // static uint8_t meritve[ST_POVPRECIJ];
  static byte stevec_meritev=0;
  static bool nastavitev_meritev=1;

  static int HITROST_MOTORJA=2000;
  static int KORAKI_MOTORJA=10000;

  //spremenljivki za povprecje:

  //prvic po inicializaciji arraya meritve ga zafilamo z ničlami:
  if(nastavitev_meritev)
  {
    for(byte a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
    nastavitev_meritev=!nastavitev_meritev;
  }

  #if DEBUG_GENERIC
  Serial.print("Demo argument: ");Serial.println(demo);
  Serial.print("ali_merim argument: ");Serial.println(ali_merim);
  Serial.print("Odpiranje argument: ");Serial.println(odpiranje);
  #endif

  
  // demo mode:
  if(demo){demo_flag=vrtenje(HITROST_MOTORJA,smer,KORAKI_MOTORJA);}
  if(demo_flag){smer=!smer;
  #if DEBUG_MOTOR
  Serial.println("smer swap");
  #endif
  demo_flag=0;
  delay(DEMO_CAKANJE);
  }
  

  else if(ali_merim||odpiranje) //z else if damo prednost demo-tu
  {

    
  //za izpis na zaslonu računalnika:
  if(ali_merim){

  #if DEBUG_MERITVE
  Serial.print("Umirjenost: ");
  Serial.println(umirjenost);
  #endif


  #if DEBUG_MERITVE
  Serial.print("Aktivno povprečje ali povprečje zadnjih 30 meritev? ");
  Serial.println(meritve[ST_POVPRECIJ]!=0); //s tem ve če PC izpise "aktivno povprečje" ali "povprečje zadnjih 30 meritev"
  #endif
  
  
  //izracun povprecja:
  
  if(stevec_meritev==ST_POVPRECIJ){stevec_meritev=0;
  #if DEBUG_MERITVE
  Serial.println("stevec_meritev reset");
  #endif
  }
  //mindwave.update(bluetooth,onMindwaveData);
  //umirjenost=mindwave.meditation();
  EEG();
  //umirjenost=meditation;
  #if DEBUG_MERITVE
  Serial.print("Umirjenost2: ");
  #endif
  Serial.println(*kazalec);
  meritve[stevec_meritev]=umirjenost;
  stevec_meritev++;
  for(uint16_t a=0;a<ST_POVPRECIJ;a++)
  {uint16_t povprecje=0;
    #if DEBUG_MERITVE
    Serial.print("meritve[a] = ");
    Serial.println(meritve[a]);
    #endif
    if(meritve[a]!=0){povprecje+=meritve[a];}
    else {povprecje/=((a+1)*(a==0)+a*(a>0));
          #if DEBUG_MERITVE
          Serial.print("Povprečje: ");
          Serial.println(povprecje);
          #endif
          break;}
    if(a==(ST_POVPRECIJ-1))
    {
      povprecje/=((a+1)*(a==0)+a*(a>0));
      #if DEBUG_MERITVE
      Serial.print("Povprečje: ");
      Serial.println(povprecje);
      #endif
    }
  }
  }
  //regulacija ne naredi ničesar če je odpiranje=0
  //regulacija(meditacija,odpiranje); //mora bit vse po branju EEG v funkciji ker sicer nastanejo težave
  }
}
//-------------------------------------------------------------------------------------------------------
//funkcija za regulacijo:
void regulacija(uint8_t vrednost,bool mode){}

//----------------------------------------------------------------------------------------------------------
void zapiranje_roze()
{//ta funkcija home-a napravo, torej zapre celo rožo
  bool utrip=0;
  while(!(vrtenje(2000,0,1000))){
    digitalWrite(START_LED,utrip);
    utrip=!utrip;}
}
//----------------------------------------------------------------------------------------------
//funkcija za vrtenje motorja:
bool vrtenje(int hitrost, bool smer, int stevilo_korakov){
  //stevilo_korakov stela rezolucijo? vpliv na čas v for loopu
static int MAX_POZICIJA_MOTORJA=5000; //vpliva na število obratov v poz smer
static int stepsPerSecond;
static int pozicija=1;
static uint32_t micros_prej=0;

#if DEBUG_MOTOR
Serial.println("vrtim");
Serial.print("hitrost: ");Serial.print(hitrost);
Serial.print(" | smer: ");Serial.print(smer);
Serial.print(" | st_korakov: ");Serial.println(stevilo_korakov);
#endif

// spremenljivka in predznak hitrosti določata če se spremenljivka pozicija povečuje ali zmanjšuje
if(smer) stepsPerSecond = hitrost;
else stepsPerSecond = -hitrost;

//for zanka za v specifičnem obsegu:  
for(int i=stevilo_korakov;i>0;--i){
   if (((stepsPerSecond > 0) && (pozicija < MAX_POZICIJA_MOTORJA))||((stepsPerSecond < 0) && digitalRead(END_SWITCH))) 
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

                if ((stepsPerSecond > 0) && (pozicija<32348)){pozicija++;}
                else if ((stepsPerSecond < 0) && digitalRead(END_SWITCH)&&(pozicija>-32348)){pozicija--;}
            }
            else{currentState = LOW;nextChange = micros() + (1000 * abs(1000.0f / stepsPerSecond)) - 30;}

            //Set direction based on the sign of stepsPerSecond
            if (stepsPerSecond > 0){digitalWrite(PIN_STEP1_DIRECTION, LOW);}
            else{digitalWrite(PIN_STEP1_DIRECTION, HIGH);}

            //Write out the step pin
            digitalWrite(PIN_STEP1_STEP, currentState);}
    }}}    
        digitalWrite(PIN_STEP1_STEP, LOW); //dodatna vrstica, preventivno da drži motor pri miru  
if((!digitalRead(END_SWITCH))&&(stepsPerSecond<0)&&((micros()-micros_prej)>(2000000+DEMO_CAKANJE*1000)))
{pozicija=0;micros_prej=micros();}
#if DEBUG_MOTOR
Serial.print("pogoj1: ");Serial.print(pozicija>MAX_POZICIJA_MOTORJA && smer>0);
Serial.print(" | pogoj2: ");Serial.print(pozicija==0 && smer<0);
Serial.print(" | pogoj3: ");Serial.println(!digitalRead(END_SWITCH));
Serial.print("pozicija: ");Serial.print(pozicija);
Serial.print(" | hitr: ");Serial.println(stepsPerSecond);
#endif
return((pozicija>=MAX_POZICIJA_MOTORJA && stepsPerSecond>0) || (pozicija==0 && stepsPerSecond<0));
}
//------------------------------------------------------------------------------------------------
// funkcija za konfiguracijo glede na nastavitve na UI plošči

byte ui_config(void){

static bool roza_stikalo;
static bool csvlog_stikalo;
static bool demo_stikalo;
static bool start_gumb=0;

while(!start_gumb)
{
start_gumb=!digitalRead(START_GUMB);
roza_stikalo=!digitalRead(ROZA_STIKALO);
csvlog_stikalo=!digitalRead(CSVLOG_STIKALO);
demo_stikalo=!digitalRead(DEMO_STIKALO);
digitalWrite(START_LED,start_gumb);
digitalWrite(ROZA_LED,roza_stikalo);
digitalWrite(CSVLOG_LED,csvlog_stikalo);
digitalWrite(DEMO_LED,demo_stikalo);
}
start_gumb=0;

Serial.println((csvlog_stikalo<<1)&(0b010));
Serial.println((roza_stikalo<<0)&(0b001));
return (demo_stikalo<<2 | csvlog_stikalo<<1 | roza_stikalo<<0);
}
