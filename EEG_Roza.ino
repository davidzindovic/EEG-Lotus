//popravi loop, hall pa mindwave zadeve lp

////////////////////////////////////////////////////////////
/*                                                       
      1. Priključi napravo na napajanje
      2. Vzemi UI ploščo
      3. Nastavi željene nastavitve
      4. Pritisni gumb start
      5. Odloži pločico nazaj (glej START LED) in umakni roko

      Vsakič, ko boš odstranil UI ploščo, se bo roža zaprla
      in boš lahko nadaljeval postopek od koraka 3. Obstoječe
      meritve bodo po zbrisane.
*/
/////////////////////////////////////////////////////////////

//mindwave.h, objekti, funkcija se bo mogoče odstranila

#include <Mindwave.h>

//Knjižnica za komuniciranje s HC-05 bluetooth modulom
//(da lahko uporabljamo serijsko komunikacijo s PC-om)
#include <AltSoftSerial.h>

//VEZAVA ZA GONILNIK ZA KORAČNI MOTOR:
#define PIN_STEP_ENABLE 2
#define PIN_STEP1_DIRECTION 3
#define PIN_STEP1_STEP 4

//VEZAVA GUMBOV,STIKAL,LUČK:
#define START_GUMB 5
#define ROZA_STIKALO 6
#define CSVLOG_STIKALO A1
#define DEMO_STIKALO 12
#define START_LED 7
#define ROZA_LED 9
#define CSVLOG_LED 10
#define DEMO_LED 11
#define END_SWITCH 13
#define HALL_SENSOR A0

#define BAUDRATE 57600      //BAUDRATE ZA MINDWAVE MOBILE

//hardcore hall vrednosti. Ponovno pomeri in spremeni v primeru spremembe pri magnetih:
#define HALL_SENSOR_AVRG 475
#define HALL_SENSOR_WIGGLE_ROOM 15

//#define MLOOP 10000
#define HITROST_MOTORJA 2000
#define KORAKI_MOTORJA 2000
#define MAX_SPREMEMBA 8 //se da spreminjat pomojm
#define MAX_POZICIJA_MOTORJA 29600  //1600 obratov @ 2000speed = 1 obrat; 
//~18.5 obratov je do odprtja -> 1600*18.5=29600

#define SAFETY_CAKANJE 5000 //čas po odložitvi v ms
#define DEMO_CAKANJE 3000   //čas pavze pri demo načinu
#define MODE_CHANGE_WAIT 3000

#define END_SWITCH_TIMEOUT 20000000

#define EEG_POSKUSI 10000

//za odpiranje DEBUG izpisov na serijcu spremeni v 1:
#define DEBUG_GENERIC 0
#define DEBUG_MOTOR 0
#define DEBUG_MERITVE 0
#define DEBUG_REGULACIJE 1
#define CSVLOG 0

//#define MINDWAVE_BAUDRATE 57600

//inicializiramo še en serial stream:
//SoftwareSerial bluetooth(A2,8);
AltSoftSerial bluetooth;
Mindwave mindwave;

uint8_t umir=0;

void setup() {

 // Timer1.initialize(1000000);
 //Timer1.attachInterrupt(test);
  //Timer1.stop();
//  delay(10);
  
  //inicializiramo pine za gonilnik;
  pinMode(PIN_STEP_ENABLE,OUTPUT);
  pinMode(PIN_STEP1_DIRECTION,OUTPUT);
  pinMode(PIN_STEP1_STEP,OUTPUT);

  //tipki in stikala so INPUT_PULLUP
  pinMode(START_GUMB,INPUT_PULLUP);
  pinMode(ROZA_STIKALO,INPUT_PULLUP);
  pinMode(CSVLOG_STIKALO,INPUT_PULLUP);
  pinMode(DEMO_STIKALO,INPUT_PULLUP);
  pinMode(END_SWITCH,INPUT_PULLUP);
    
  pinMode(START_LED,OUTPUT);
  pinMode(ROZA_LED,OUTPUT);
  pinMode(CSVLOG_LED,OUTPUT);
  pinMode(DEMO_LED,OUTPUT);

  pinMode(HALL_SENSOR,INPUT);


  //zaženemo software serial:
  bluetooth.begin(MINDWAVE_BAUDRATE);
  
  Serial.begin(MINDWAVE_BAUDRATE);

  //ČAKANJE PO VKLOPU, pred home-anjem:
  delay(SAFETY_CAKANJE);
  
  //home-anje roze:
  zapiranje_roze();
}

void loop() {

//pazi orientacijo močnejšega magneta:
//static uint16_t HALL_SENSOR_AVRG=analogRead(HALL_SENSOR); //preberemo mirovno vrednost hall senzorja
//static uint16_t HALL_SENSOR_WIGGLE_ROOM=40;              //določimo možno odstopanje (je večje zaradi močnejšega magneta
static bool first_nastavitve=1;                           //spremenljivka za prvotno nastavitev
static byte nastavitve;
static bool ponovna_nastavitev=1;
//static bool toggle=0;

byte mask_demo=0b100;
byte mask_log=0b010;
byte mask_roza=0b001;

// prvič, ko zaženemo program, pridemo v ui_config in spremenimo flag, da ne pridemo več v ta del programa
if(first_nastavitve){nastavitve=ui_config();first_nastavitve=!first_nastavitve;}
//while(1){mindwave.update(bluetooth,onMindwaveData);}

//spremenjen pogoj zaradi zamenjave orientacije senzorja:
while(analogRead(HALL_SENSOR)>=(HALL_SENSOR_AVRG+HALL_SENSOR_WIGGLE_ROOM)){
  if((nastavitve&mask_log)==2){for(int c=0;c<100;c++)mindwave.update(bluetooth,onMindwaveData);}
  #if DEBUG_GENERIC
  Serial.print("CAKAM DA ODLOZIS UI PLATO | HALL_SENOZOR:");
  Serial.print(analogRead(HALL_SENSOR));
  Serial.print("Povprečje HALL: ");
  Serial.print(HALL_SENSOR_AVRG);
  Serial.print(" +/- ");
  Serial.println(HALL_SENSOR_WIGGLE_ROOM);
  #endif
  } //dokler je UI plošča odmaknjena naprava čaka

delay(SAFETY_CAKANJE); // po potrditvi konfiguracije in odložitvi ploščice ima uporabnik SAFETY_CAKANJE/1000 sekund da umakne roko
zapiranje_roze();
digitalWrite(ROZA_LED,(nastavitve&mask_roza)==1);
digitalWrite(CSVLOG_LED,(nastavitve&mask_log)==2);
digitalWrite(DEMO_LED,(nastavitve&mask_demo)==4);
digitalWrite(START_LED,1);
delay(MODE_CHANGE_WAIT);

while((analogRead(HALL_SENSOR)<=(HALL_SENSOR_AVRG+HALL_SENSOR_WIGGLE_ROOM)) ){
#if DEBUG_GENERIC
Serial.print("GARBAM\n  parametri:");
Serial.print((nastavitve&mask_demo)==4);
Serial.print((nastavitve&mask_log)==2);
Serial.println((nastavitve&mask_roza)==1);
Serial.print("HALL: ");Serial.println(Serial.print(analogRead(HALL_SENSOR)));
#endif
//digitalWrite(DEMO_LED,toggle);toggle=!toggle;
ObdelavaPodatkov((nastavitve&mask_demo)==4,(nastavitve&mask_log)==2,(nastavitve&mask_roza)==1,ponovna_nastavitev);
if(ponovna_nastavitev)ponovna_nastavitev=0;
}
#if DEBUG_GENERIC
Serial.println("NE GARBAM");
#endif

#if CSVLOG
if((nastavitve&mask_log)<=3&&(nastavitve&mask_log)>=1)Serial.println("stop"); // če smo merili pošjemo stop da python neha beležiti podatke
#endif
nastavitve=ui_config();
ponovna_nastavitev=1;
//delay(MODE_CHANGE_WAIT);
//zapiranje_roze();


}
//-----------------------------------------------------------------------------------------------

// funkcija za mindwave mobile in preostanek (vendar posebna knjižnica:):
void onMindwaveData(){

umir=mindwave.meditation();
#if DEBUG_MERITVE
Serial.print("umirjenost data: ");
Serial.println(umir);
#endif
}

//---------------------------------------------------------------------------------------------
//Funkcija za branje enega byte-a iz payload stream-a (od software serial, lahko je tudi navaden serial)
//VRNE prebran byte
/*int ReadOneByte() {
  int ByteRead;

  //while(!bluetooth.available())Serial.println("nema"); //počakamo, dokler ne dobimo nečesa za prebrat
  ByteRead = bluetooth.read();   // preberemo bajt

#if DEBUG_EEG
  Serial.println("prebral");
  //Serial.println((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}*/
//--------------------------------------------------------------------------------------------
//Funkcija za branje EEG oz. obdelavo payloada od Neurosky:
/*
void EEG()
{
  
  //Serial.println("EEG");
// checksum variables
//byte generatedChecksum = 0;
uint16_t generatedChecksum = 0;
byte checksum = 0; 
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
//byte attention = 0; //če želimo zvedeti fokus, odkomentiramo
//byte meditation = 0;

// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;
static bool bruh=0;
  
  // Look for sync bytes
  if(bluetooth.read() == 170) {
    if(bluetooth.read() == 170) {

      payloadLength = bluetooth.read();
 
   if(payloadLength > 169)return;       
      generatedChecksum = 0;        
      for(int i = 0; i < payloadLength; i++) {  
        payloadData[i] = bluetooth.read();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }   

      checksum = bluetooth.read();                      //Read checksum byte from stream      
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

        if(checksum == generatedChecksum) {    
    //Serial.println("!!!!!!!!!!!!!!!!!!!!!!");
        poorQuality = 200;
       // attention = 0;
        //meditation = 0;
        umir=0;
        for(int i = 0; i < payloadLength; i++) {    // Parse the payload
          switch (payloadData[i]) {
          case 2:
            i++;            
            poorQuality = payloadData[i];
            bigPacket = true;            
            break;
          case 4:
            i++;
            //če bi potrebovali fokus, odkomentiramo:
            //attention = payloadData[i];                        
            break;
          case 5:
            i++;
            //shranimo v umirjenost_pointer za umirjenost
            umir= payloadData[i];
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
        //erial.println("EEG");
        if(bigPacket) {
          Serial.print("PoorQuality: ");
          Serial.println(poorQuality, DEC);
          Serial.print(" Umirjenost: ");
          Serial.println(umir, DEC);
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

Serial.println("o");
//noInterrupts();
//uint8_t prej=umir;
//uint32_t cnt=0;
//for(int i=0;i<(EEG_POSKUSI*10);i++)
//while((umir==prej)/*&&(cnt<1000))*/
//{
//mindwave.update(bluetooth,onMindwaveData);
//cnt++;
//}
//interrupts();
//Serial.println(mindwave.meditation());
//Serial.println("a");
//}
//------------------------------------------------------------------------------- 

void meritvice(bool ali_reguliram){
  //Serial.println("a");
  
  static bool ledica=0;
  uint8_t eeg_prej=umir;

  //načeloma za toglanje, ampak ga while preglasi:
  if(bluetooth.available()>0){
  digitalWrite(CSVLOG_LED,ledica);
  ledica=!ledica;
  }

  //dokler smo v loopou gorita rdeča in rumena lučka CSVLOG
  while(umir==eeg_prej){
  mindwave.update(bluetooth,onMindwaveData);
  digitalWrite(CSVLOG_LED,ledica);
  ledica=!ledica;
  }
  if(ali_reguliram)regulacija(umir);
  
  }
//----------------------------------------------------------------------------
// Funkcija za obdelavo podatkov:
// VHODNI parametri: nastavitev stikal na UI plošči (DEMO, CSVLOG, ODPIRANJE ROŽE)
void ObdelavaPodatkov(bool demo,bool ali_merim, bool odpiranje,bool reset_settings) {
  
  //demo mode spremenljivki:
  static bool demo_flag=0; //če sem prišel do zgornje ali spodnje meje
  static bool smer=0;      // da lahko spreminjam smer vrtejna v demo mode

  static bool print_flag=1;
  // spremenljivki za meritve:
  static byte stevec_meritev=0;
  //static bool nastavitev_meritev=1;
  const byte ST_POVPRECIJ=30;
  static uint16_t meritve[ST_POVPRECIJ];
  //static bool eeg_kalibracija=1;
  static unsigned long prejle=0;

  //static uint8_t umirjenost=0;
  static uint8_t umirjenost_prej=0;
  
  //Je v #define
  //static int HITROST_MOTORJA=2000;
  //static int KORAKI_MOTORJA=10000;


  
  //prvic po inicializaciji arraya meritev ga zafilamo z ničlami
  //oz. resetiramo ob ponovnem zagonu/spremembi mode-a:
  if(reset_settings)
  {
    for(byte a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
  }
  
//a bo potrebno preden kličeš motorje shranit vrednost od micros()??

//ZA STESTIRAT:!!!!!!
//vrtenje(2000,1,100); 
  
  #if DEBUG_GENERIC
  Serial.print("Demo argument: ");Serial.println(demo);
  Serial.print("ali_merim argument: ");Serial.println(ali_merim);
  Serial.print("Odpiranje argument: ");Serial.println(odpiranje);
  #endif

  // demo mode:
  if(demo){demo_flag=vrtenje(HITROST_MOTORJA,smer,KORAKI_MOTORJA,-1);}
  //ko roža pride do software maksimuma oz. do end switcha, flaga demo_flag na "1":
  if(demo_flag){
  smer=!smer; //spremenimo smer
  #if DEBUG_MOTOR
  Serial.println("smer swap");
  #endif
  demo_flag=0;//ponastavimo demo_flag
  delay(DEMO_CAKANJE); //počakamo, da se lahko nagledamo odpre/zaprte rože
  }
  
  //če se odločimo za CSVLOG ali odpiranje ROŽE
  else if(ali_merim||odpiranje) //z else if damo prednost demo-tu
  {  
  
  //za izpis na zaslonu računalnika (CSVLOG):
  if(ali_merim){
  if(abs(micros()-prejle)>1000000){meritvice(odpiranje);prejle=micros();}
  #if DEBUG_MERITVE
  if(print_flag){
  Serial.print("Aktivno povprečje(0) ali povprečje zadnjih 30 meritev(1)? ");
  Serial.println(meritve[ST_POVPRECIJ-1]!=0); //s tem ve če PC izpise "aktivno povprečje" ali "povprečje zadnjih 30 meritev"
  print_flag=0;
  }
  #endif
  #if CSVLOG
  Serial.println(meritve[ST_POVPRECIJ]!=0);
  #endif
  
  
  // za ponastavitev kazalca v arrayu
  if(stevec_meritev==ST_POVPRECIJ){stevec_meritev=0;
  #if DEBUG_MERITVE
  Serial.println("stevec_meritev reset!");
  #endif
  }
  



  //Izmerjen nivo umirjenosti shranimo v array 
  //in premaknemo umirjenost_pointer na naslednje mesto:
  if(umirjenost_prej!=umir){
    meritve[stevec_meritev]=umir;
    stevec_meritev++;
    umirjenost_prej=umir;
    print_flag=1;
    
    #if DEBUG_MERITVE
    Serial.print("Umirjenost: ");
    Serial.println(umir);
    #endif
          
    #if CSVLOG
    Serial.println(umir);
    #endif


  //izračunamo povprečje in izpišemo glede na spremenljivko ali_merim:
  uint16_t povprecje=0;
  for(uint16_t a=0;a<ST_POVPRECIJ;a++)
  {
    #if DEBUG_MERITVE
    Serial.print(a);
    Serial.print(" meritve[a] = ");
    Serial.print(meritve[a]);
    #endif
    #if DEBUG_MERITVE
    Serial.print(" |  Povprečje VMES: ");
    Serial.println(povprecje);
    #endif
    if(meritve[a]!=0){povprecje+=meritve[a];}
    else {povprecje/=((a+1)*(a==0)+a*(a>0));
          #if DEBUG_MERITVE
          Serial.print("Povprečje: ");
          Serial.println(povprecje);
          #endif
          #if CSVLOG
          if(ali_merim)Serial.println(povprecje);
          #endif
          break; //če srečamo element z vrednostjo 0 sklepamo, da so vsi preostali 0, povprečimo in zaključimo zanko
          }
    if(a==(ST_POVPRECIJ-1))
    {
      //izračun je prilagojen, da ne delimo z 0:
      povprecje/=ST_POVPRECIJ;
      #if DEBUG_MERITVE
      Serial.print("Povprečje: ");
      Serial.println(povprecje);
      #endif
      #if CSVLOG
      if(ali_merim)Serial.println(povprecje);
      #endif
    }
  }}
  }
  //regulacija ne naredi ničesar če je odpiranje=0
  //regulacija(meditacija,odpiranje); //mora bit vse po branju EEG v funkciji ker sicer nastanejo težave
  }
}
//-------------------------------------------------------------------------------------------------------
//funkcija za regulacijo:
void regulacija(int trenutno_stanje)
{static bool first_reg=false;
static int pozicija_cilj=1;
static int motor=0;
static int delta_motor=0;
int regulacijsko_povprecje;
static int meditacija_prej;
static uint8_t prevent;

if(first_reg!=false || trenutno_stanje!=0) //preventiva za prvo izvedbo
{ // upoštevamo novo vrednost meditacije oz. umirjenosti in pomnimo le zadnjo prejšnjo:
 if (first_reg==false)
  {
  if(trenutno_stanje!=meditacija_prej)meditacija_prej=trenutno_stanje;//regulacija(meditacija_prej,trenutno_stanje);
  first_reg=!first_reg;
  }
 else
  { // računamo regulacijsko povprečje z aritmetično sredino (in prilagajamo glede na vrednosti meditacij zdej in prej)
   if(trenutno_stanje>meditacija_prej)regulacijsko_povprecje=(trenutno_stanje-(trenutno_stanje-meditacija_prej)/2);
   else regulacijsko_povprecje=(trenutno_stanje+(-trenutno_stanje+meditacija_prej)/2);
   meditacija_prej=trenutno_stanje;
  }
}
 // uporabimo regulacijske vrednosti za določitev premika motorja, pazimo na overflow:
 if(abs(regulacijsko_povprecje-motor)>MAX_SPREMEMBA) delta_motor=MAX_SPREMEMBA*(regulacijsko_povprecje-motor)/abs(regulacijsko_povprecje-motor); //zadnji del je za predznak
 else delta_motor=regulacijsko_povprecje-motor; 

// premik shranimo v spremenljivko:
motor+=delta_motor;

#if DEBUG_REGULACIJE
Serial.print("Meditacija: ");
Serial.print(umir);
Serial.print(" | Trenutno stanje: ");
Serial.print(trenutno_stanje);
Serial.print(" | Regulacijsko povprečje: ");
Serial.print(regulacijsko_povprecje);
Serial.print(" | Motor: ");
Serial.println(motor);
#endif

// Preventiva, da ne bo težav pri mapiranju:
if (motor>100) motor=100;
else if (motor<0) motor=0;

// mappamo od 0 do max obratov
pozicija_cilj=map(motor,0,100,0,MAX_POZICIJA_MOTORJA);

#if DEBUG_REGULACIJE
//Serial.print("Pozicija: ");
//Serial.print(pozicija);
Serial.print(" | Pozicija cilj: ");
Serial.println(pozicija_cilj);
#endif
prevent=0;
// zančno primerjamo trenutno in ciljno pozicijo ter obračamo motor
while(!(vrtenje(HITROST_MOTORJA,0,KORAKI_MOTORJA,pozicija_cilj)))
// vmesni pogoj definira smer vrtenja
{

//  vrtenje(HITROST_MOTORJA,pozicija<pozicija_cilj,KORAKI_MOTORJA,pozicija_cilj); // vmesni pogoj definira smer vrtenja
//if(prevent>=10)break;
//prevent++;
}                                                                 
}

//----------------------------------------------------------------------------------------------------------
void zapiranje_roze()
{//Funkcija, ki home-a napravo, torej zapre celo rožo

digitalWrite(ROZA_LED,0);
digitalWrite(CSVLOG_LED,0);
digitalWrite(DEMO_LED,0);
digitalWrite(START_LED,0);
  
  bool utrip=0; //za heartbeat START_LED (sočasno brlita zelena in rdeča štart LED)
  //zanka se izhvaja dokler funkcija vrtenje ne vrne "1" (torej se vrti dokler ne zadane END_SWITCH
  //ODPIRANJE HOME ZARAD END SWITCHA
  uint32_t zacetni_cas_homanja=micros();
  while(!(vrtenje(2000,1,1000,-1))){
    if((micros()-zacetni_cas_homanja)>END_SWITCH_TIMEOUT)break;//ce se homea predolg prekini homeanje
    digitalWrite(START_LED,utrip);
    utrip=!utrip;}
}
//----------------------------------------------------------------------------------------------
//funkcija za vrtenje motorja:
//VHODNI parametri: hitrost vrtenja, smer vrtenja (predznak hitrosti), število korako(ločljivost "korakov")
//-> manjše število korakov pomeni hitrejša izvedba for zanke in vrnitev ven iz funkcije
//VRNE: informacijo, če se je roža do konca zaprla (beremo stikalo END_SWITCH), ali pa do konca odprla (če je pozicija na definiranem maksimumu)

bool vrtenje(int hitrost, bool smer, int stevilo_korakov,int poz_cilj){ //če ne želimo uporabljati regulacij nastavimo cilj na -1
// +/TRUE smer je odpiranje, -/FALSE smer je zapiranje
static int stepsPerSecond;
static int pozicija=32300;
static uint32_t micros_prej=micros();
//ZAKOMENTIRANI DELI KER TESTIRAM ČE JE KUL
#if DEBUG_MOTOR
Serial.println("vrtim");
Serial.print("hitrost: ");Serial.print(hitrost);
Serial.print(" | smer: ");Serial.print(smer);
Serial.print(" | st_korakov: ");Serial.println(stevilo_korakov);
#endif

// spremenljivka in predznak hitrosti določata če se spremenljivka pozicija povečuje ali zmanjšuje
if(poz_cilj==-1){
if(smer) stepsPerSecond = hitrost;
else stepsPerSecond = -hitrost;}
else if (pozicija<poz_cilj) {stepsPerSecond = -hitrost;} //PREVERI PREDZNAKE!!!!
else stepsPerSecond = hitrost;
 //če poz>0 ni ok daj nazaj >-32348
for(int i=stevilo_korakov;(i>0)&&(pozicija!=poz_cilj);--i){
   if (((stepsPerSecond > 0) && (pozicija > 0))||((stepsPerSecond < 0) && (pozicija < MAX_POZICIJA_MOTORJA))) 
   {
    static unsigned long nextChange = 0;
    static uint8_t currentState = LOW;

    if (stepsPerSecond == 0)
    { //če je stepsPerSecond enak 0 nastavimo pin za korak na 0, da se motor ne premika
        currentState = LOW;
        digitalWrite(PIN_STEP1_STEP, LOW);}
    else
    {
        //če stepsPerSecond ni enak 0, potem je potrebno preračunati naslednji čas za spremembo stanja gonilnika TMC2209
        if (micros() > nextChange)
        {   //Generira korake
            if (currentState == LOW)
            {
                currentState = HIGH;
                nextChange = micros() + 30;

                if ((stepsPerSecond > 0) && digitalRead(END_SWITCH)&&(pozicija>0)){pozicija--;}
                else if ((stepsPerSecond < 0) &&(pozicija<MAX_POZICIJA_MOTORJA)){pozicija++;}
            }
            else{currentState = LOW;nextChange = micros() + (1000 * abs(1000.0f / stepsPerSecond)) - 30;}

            //Nastavi smer gelde ne predznak hitrosti oz. spremenljivke stepsPerSecond
            if (stepsPerSecond > 0){digitalWrite(PIN_STEP1_DIRECTION, LOW);}
            else{digitalWrite(PIN_STEP1_DIRECTION, HIGH);}

            //Zapiše izbrano stanje na pin za korak
            digitalWrite(PIN_STEP1_STEP, currentState);}
    }}}    
        digitalWrite(PIN_STEP1_STEP, LOW); //dodatna vrstica, preventivno da drži motor pri miru  
//ponastavimo pozicijo, če roža zadane end switch. Vmes od zadnjega proženja morata miniti vsaj 2 sekundi + DEMO_CAKANJE
if((!digitalRead(END_SWITCH))&&(stepsPerSecond>0))
{pozicija=0;
//micros_prej=micros();
}

#if DEBUG_MOTOR
Serial.print("pogoj1: ");Serial.print(pozicija>MAX_POZICIJA_MOTORJA && smer>0);
Serial.print(" | pogoj2: ");Serial.print(pozicija==0 && smer<0);
Serial.print(" | pogoj3: ");Serial.println(!digitalRead(END_SWITCH));
Serial.print("pozicija: ");Serial.print(pozicija);
Serial.print(" | hitr: ");Serial.println(stepsPerSecond);
#endif
//vrne če je roža prišla do keregakol ekstrema ali pa do cilja
return((pozicija>=MAX_POZICIJA_MOTORJA && stepsPerSecond<0) || (pozicija==0 && stepsPerSecond>0) || (pozicija==poz_cilj));
}
//------------------------------------------------------------------------------------------------
// funkcija za konfiguracijo glede na nastavitve na UI plošči
// vrne masko glede na pritisnjena stikala: DEMO,LOG,ROŽA
byte ui_config(void){

byte mask_log=0b010;
byte mask_roza=0b001;

//spremeljivke za stikala:
bool roza_stikalo;
bool csvlog_stikalo;
bool demo_stikalo;
bool start_gumb=0;

//dokler ni pritisnjena start tipka, čaka:
while(!start_gumb)
{
//vrednosti gumbov (PULLUP orientacija):
start_gumb=!digitalRead(START_GUMB);
roza_stikalo=!digitalRead(ROZA_STIKALO);
csvlog_stikalo=!digitalRead(CSVLOG_STIKALO);
demo_stikalo=!digitalRead(DEMO_STIKALO);

//takoj prilagodimo prižgano lučko glede na stanje stikala:
digitalWrite(START_LED,start_gumb);
digitalWrite(ROZA_LED,roza_stikalo);
digitalWrite(CSVLOG_LED,csvlog_stikalo);
digitalWrite(DEMO_LED,demo_stikalo);
}

start_gumb=0;//resetiramo spremenljivko

//izpis "mode"-ov za python interface,
//če je pritisnjeno stikalo za CSVLOG:
#if CSVLOG
if((csvlog_stikalo<<1)&(mask_log)){
Serial.println((csvlog_stikalo<<1)&(mask_log));
Serial.println((roza_stikalo<<0)&(mask_roza));}
#endif

//vrne masko sestavljeno iz zamaknjenih bitov spremeljivk
return (demo_stikalo<<2 | csvlog_stikalo<<1 | roza_stikalo<<0);
}
