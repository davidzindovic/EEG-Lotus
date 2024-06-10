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

#include <TimerOne.h>
#include <Mindwave.h>
//Knjižnica za komuniciranje s HC-05 bluetooth modulom
//(da lahko uporabljamo serijsko komunikacijo s PC-om)
#include <SoftwareSerial.h>

//VEZAVA ZA GONILNIK ZA KORAČNI MOTOR:
#define PIN_STEP_ENABLE 2
#define PIN_STEP1_DIRECTION 3
#define PIN_STEP1_STEP 4

//VEZAVA GUMBOV,STIKAL,LUČK:
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

#define BAUDRATE 57600      //BAUDRATE ZA MINDWAVE MOBILE

#define MLOOP 10000

#define SAFETY_CAKANJE 2000 //čas po odložitvi v ms
#define DEMO_CAKANJE 1000   //čas pavze pri demo načinu
#define MODE_CHANGE_WAIT 5000

#define EEG_POSKUSI 10

//za odpiranje DEBUG izpisov na serijcu spremeni v 1:
#define DEBUG_GENERIC 1
#define DEBUG_MOTOR 0
#define DEBUG_MERITVE 0
#define DEBUG_EEG 0

//inicializiramo še en serial stream:
SoftwareSerial bluetooth(A2,8);
Mindwave mindwave;

//spremenljivke in array za umirjenost
const byte ST_POVPRECIJ=30;
uint16_t meritve[ST_POVPRECIJ];
byte umirjenost=0;
byte *umirjenost_pointer=&umirjenost;
byte nastavitve;

uint8_t umirjenost_prej=0;

//Maske za UI ploščo:
byte mask_demo=0b100;
byte mask_log=0b010;
byte mask_roza=0b001;

void setup() {
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

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(EEG);

  //zaženemo software serial:
  bluetooth.begin(MINDWAVE_BAUDRATE);
  //mindwave.setupe();
  
  Serial.begin(MINDWAVE_BAUDRATE);
  
  //mindwave.update(bluetooth);
  //delay(10000);
  //mindwave.update(bluetooth,onMindwaveData);
  //Serial.println(umirjenost);
  //delay(10000);
  //while(1){mindwave.update(bluetooth,onMindwaveData);}
  //for(int b=0;b<MLOOP;b++)mindwave.update(bluetooth,onMindwaveData);
  //home-anje roze:
  zapiranje_roze();
}

void loop() {

static uint16_t HALL_SENSOR_AVRG=analogRead(HALL_SENSOR); //preberemo mirovno vrednost hall senzorja
static uint16_t HALL_SENSOR_WIGGLE_ROOM=70;               //določimo možno odstopanje (je večje zaradi močnejšega magneta
static bool first_nastavitve=1;                           //spremenljivka za prvotno nastavitev

// prvič, ko zaženemo program, pridemo v ui_config in spremenimo flag, da ne pridemo več v ta del programa
if(first_nastavitve){nastavitve=ui_config();first_nastavitve=!first_nastavitve;}
//while(1){mindwave.update(bluetooth,onMindwaveData);}
//dokler je 
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

delay(SAFETY_CAKANJE); // po potrditvi konfiguracije in odložitvi ploščice ima uporabnik SAFETY_CAKANJE/1000 sekund da umakne roko

while((analogRead(HALL_SENSOR)>=(HALL_SENSOR_AVRG-HALL_SENSOR_WIGGLE_ROOM)) ){
#if DEBUG_GENERIC
Serial.print("GARBAM\n  parametri:");
Serial.print((nastavitve&mask_demo)==4);
Serial.print((nastavitve&mask_log)==2);
Serial.println((nastavitve&mask_roza)==1);
Serial.print("HALL: ");Serial.println(Serial.print(analogRead(HALL_SENSOR)));
#endif
ObdelavaPodatkov((nastavitve&mask_demo)==4,(nastavitve&mask_log)==2,(nastavitve&mask_roza)==1);
}
#if DEBUG_GENERIC
Serial.println("NE GARBAM");
#endif
if((nastavitve&mask_log)==2)Serial.println("stop"); // če smo merili pošjemo stop da python neha beležiti podatke
for(int a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
nastavitve=ui_config();
delay(MODE_CHANGE_WAIT);
zapiranje_roze();


}
//-----------------------------------------------------------------------------------------------

// funkcija za mindwave mobile in preostanek (vendar posebna knjižnica:):
void onMindwaveData(){
//Serial.print("umirjenost loop: ");
//umirjenost=mindwave.meditation();
//Serial.println(umirjenost);
Serial.println("bruh");
umirjenost=mindwave.meditation();
Serial.println(umirjenost);
}

//---------------------------------------------------------------------------------------------
//Funkcija za branje enega byte-a iz payload stream-a (od software serial, lahko je tudi navaden serial)
//VRNE prebran byte
byte ReadOneByte() {
  int ByteRead;

  //while(!bluetooth.available())Serial.println("nema"); //počakamo, dokler ne dobimo nečesa za prebrat
  ByteRead = bluetooth.read();   // preberemo bajt

#if DEBUG_EEG
  Serial.println(ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}
//--------------------------------------------------------------------------------------------
//Funkcija za branje EEG oz. obdelavo payloada od Neurosky:
void EEG()
{
  /*
  Serial.println("EEG");
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
  if(ReadOneByte() == 170) {
    if(ReadOneByte() == 170) {

      payloadLength = ReadOneByte();
      if(payloadLength > 169)                      //Payload length can not be greater than 169
          bruh=0;
      else
          bruh=1;
   if(bruh==1){       
      generatedChecksum = 0;        
      for(int i = 0; i < payloadLength; i++) {  
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }   

      checksum = ReadOneByte();                      //Read checksum byte from stream      
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

        if(checksum == generatedChecksum) {    
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!");
        poorQuality = 200;
       // attention = 0;
        //meditation = 0;
        *umirjenost_pointer=0;
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
            *umirjenost_pointer = payloadData[i];
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
          Serial.println(*umirjenost_pointer, DEC);
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
*/
for(int i=0;i<EEG_POSKUSI;i++)
{if(umirjenost_prej==umirjenost)mindwave.update(bluetooth,onMindwaveData);
else break;
}

}
//----------------------------------------------------------------------------
// Funkcija za obdelavo podatkov:
// VHODNI parametri: nastavitev stikal na UI plošči (DEMO, CSVLOG, ODPIRANJE ROŽE)
void ObdelavaPodatkov(bool demo,bool ali_merim, bool odpiranje) {
  
  //demo mode spremenljivki:
  static bool demo_flag=0; //če sem prišel do zgornje ali spodnje meje
  static bool smer=1;      // da lahko spreminjam smer vrtejna v demo mode
 
  // spremenljivki za meritve:
  static byte stevec_meritev=0;
  static bool nastavitev_meritev=1;

  //NASTAVITVI za motor (lahko spreminjaš):
  static int HITROST_MOTORJA=2000;
  static int KORAKI_MOTORJA=10000;

  //static uint8_t umirjenost_prej=umirjenost;

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
    
  #if DEBUG_MERITVE
  Serial.print("Umirjenost: ");
  Serial.println(umirjenost);
  Serial.print("Aktivno povprečje ali povprečje zadnjih 30 meritev? ");
  Serial.println(meritve[ST_POVPRECIJ]!=0); //s tem ve če PC izpise "aktivno povprečje" ali "povprečje zadnjih 30 meritev"
  #endif
  
  // za ponastavitev kazalca v arrayu
  if(stevec_meritev==ST_POVPRECIJ){stevec_meritev=0;
  #if DEBUG_MERITVE
  Serial.println("stevec_meritev reset");
  #endif
  }
  
 
  #if DEBUG_MERITVE
  Serial.print("Umirjenost2: ");
  #endif

  //izpišemo le v primeru, da izberemo CSVLOG:
  //if(ali_merim)Serial.println(*umirjenost_pointer);

  //posodobimo spremeljivo umirjenost(imamo umirjenost_pointer)
  //uint8_t umirjenost_prej=umirjenost;
  //while(umirjenost_prej==umirjenost)
  //{
  //če je samo dela meritve, vendar sekvenčni ukaz za motor ne dewa. Poglej!!
  //verjetn lahko v onMindwaveData spraviš vse meritve(array) in povprečejnje!!!!!!!
  //mindwave.update(bluetooth,onMindwaveData);
  //}
  //vrtenje(2000,1,10000); 
  //Izmerjen nivo umirjenosti shranimo v array 
  //in premaknemo umirjenost_pointer na naslednje mesto:
  if(umirjenost_prej!=umirjenost){
    meritve[stevec_meritev]=umirjenost;
    stevec_meritev++;
    umirjenost_prej=umirjenost;    
  


  //izračunamo povprečje in izpišemo glede na spremenljivko ali_merim:
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
          #endif
         //if(ali_merim)Serial.println(povprecje);
          break; //če srečamo element z vrednostjo 0 sklepamo, da so vsi preostali 0, povprečimo in zaključimo zanko
          }
    if(a==(ST_POVPRECIJ-1))
    {
      //izračun je prilagojen, da ne delimo z 0:
      povprecje/=((a+1)*(a==0)+a*(a>0));
      #if DEBUG_MERITVE
      Serial.print("Povprečje: ");
      #endif
      //if(ali_merim)Serial.println(povprecje);
    }
  }}
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
{//Funkcija, ki home-a napravo, torej zapre celo rožo
  
  bool utrip=0; //za heartbeat START_LED (sočasno brlita zelena in rdeča štart LED)
  //zanka se izhvaja dokler funkcija vrtenje ne vrne "1" (torej se vrti dokler ne zadane END_SWITCH
  while(!(vrtenje(2000,0,1000))){ 
    digitalWrite(START_LED,utrip);
    utrip=!utrip;}
}
//----------------------------------------------------------------------------------------------
//funkcija za vrtenje motorja:
//VHODNI parametri: hitrost vrtenja, smer vrtenja (predznak hitrosti), število korako(ločljivost "korakov")
//-> manjše število korakov pomeni hitrejša izvedba for zanke in vrnitev ven iz funkcije
//VRNE: informacijo, če se je roža do konca zaprla (beremo stikalo END_SWITCH), ali pa do konca odprla (če je pozicija na definiranem maksimumu)
bool vrtenje(int hitrost, bool smer, int stevilo_korakov){

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
 
for(int i=stevilo_korakov;i>0;--i){
   if (((stepsPerSecond > 0) && (pozicija < MAX_POZICIJA_MOTORJA))||((stepsPerSecond < 0) && digitalRead(END_SWITCH))) 
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

                if ((stepsPerSecond > 0) && (pozicija<32348)){pozicija++;}
                else if ((stepsPerSecond < 0) && digitalRead(END_SWITCH)&&(pozicija>-32348)){pozicija--;}
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
// vrne masko glede na pritisnjena stikala: DEMO,LOG,ROŽA
byte ui_config(void){

//spremeljivke za stikala:
static bool roza_stikalo;
static bool csvlog_stikalo;
static bool demo_stikalo;
static bool start_gumb=0;

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
if((csvlog_stikalo<<1)&(0b010)){
Serial.println((csvlog_stikalo<<1)&(0b010));
Serial.println((roza_stikalo<<0)&(0b001));}

//vrne masko sestavljeno iz zamaknjenih bitov spremeljivk
return (demo_stikalo<<2 | csvlog_stikalo<<1 | roza_stikalo<<0);
}
