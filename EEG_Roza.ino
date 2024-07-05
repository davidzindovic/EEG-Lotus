//popravi loop, hall pa mindwave zadeve lp

////////////////////////////////////////////////////////////
/*                                                       
      1. Priključi napravo na napajanje in počakaj,
         da se skalibrira (odpre in zapre)
      2. Vzemi UI ploščo
      3. Nastavi željene nastavitve
      4. Pritisni gumb start
      5. Odloži pločico nazaj (glej START LED) in umakni roko

      Vsakič, ko boš odstranil UI ploščo, se bo roža odprla in
      zaprla, za tem boš lahko nadaljeval postopek od
      koraka 3. Obstoječe meritve bodo po zbrisane.
*/
/////////////////////////////////////////////////////////////

//za namen python loggerja je potrebno urediti serijske izpise

#include <Mindwave.h>

//Knjižnica za komuniciranje s HC-05 bluetooth modulom
//(da lahko uporabljamo serijsko komunikacijo s PC-om):
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

//hardcode hall vrednosti. Ponovno pomeri in spremeni v primeru spremembe pri magnetih:
#define HALL_SENSOR_AVRG 475
#define HALL_SENSOR_WIGGLE_ROOM 15

#define HITROST_MOTORJA 2000
#define KORAKI_MOTORJA 2000
#define MAX_SPREMEMBA 8 //nastavljeno s preizkušanjem
#define MAX_POZICIJA_MOTORJA 29600  //1600 obratov @ 2000speed = 1 obrat; 
//~18.5 obratov je do odprtja -> 1600*18.5=29600

#define SAFETY_CAKANJE 5000 //čas po odložitvi v ms
#define DEMO_CAKANJE 3000   //čas pavze pri demo načinu
#define MODE_CHANGE_WAIT 3000

//če se ne odpre/zapre v omejenem času naj naprava sklepa, da nekaj ni ok
//in prekine obratovanje
#define END_SWITCH_TIMEOUT 40000000
#define OPENING_TIMEOUT 40000000 //enak cas predviden za doseg obeh ekstremov

//za odpiranje DEBUG izpisov na serijcu spremeni v 1:
#define DEBUG_GENERIC 0
#define DEBUG_MOTOR 0
#define DEBUG_MERITVE 0
#define DEBUG_REGULACIJE 0
#define CSVLOG 0

AltSoftSerial bluetooth;//inicializiramo še en serial stream
Mindwave mindwave;

//globalna spremenljivka, ki hrani vrednost umirjenosti:
uint8_t umir=0;

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

  //zaženemo umetno serijsko komunikacijo:
  bluetooth.begin(MINDWAVE_BAUDRATE);
  
  Serial.begin(MINDWAVE_BAUDRATE);

  //ČAKANJE PO VKLOPU, pred home-anjem:
  delay(SAFETY_CAKANJE);
  
  //home-anje roze:
  zapiranje_roze();
}

void loop() {

static bool first_nastavitve=1;  //spremenljivka za prvotno nastavitev
static byte nastavitve;
static bool ponovna_nastavitev=0;

//maske za stikala:
byte mask_demo=0b100;
byte mask_log=0b010;
byte mask_roza=0b001;

// prvič, ko zaženemo program, pridemo v ui_config in spremenimo flag, da ne pridemo več v ta del programa
if(first_nastavitve){nastavitve=ui_config();first_nastavitve=!first_nastavitve;}

//po pritisku na tipko start skočimo ven iz ui_config in čakamo, da uporabnik odloži
//uporabniški vmesnik, pri čemer beremo hall senzor in primerjamo z vnaprej nastavljenimi vrednostmi
while(analogRead(HALL_SENSOR)>=(HALL_SENSOR_AVRG+HALL_SENSOR_WIGGLE_ROOM)){
  //če smo izbrali merjenje si damo headstart da se komunikacija čim prej začne: 
  if((nastavitve&mask_log)==2){for(int c=0;c<100;c++)mindwave.update(bluetooth,onMindwaveData);}
  
  #if DEBUG_GENERIC
  Serial.print("CAKAM DA ODLOZIS UI PLATO | HALL_SENOZOR:");
  Serial.print(analogRead(HALL_SENSOR));
  Serial.print("  Povprečje HALL: ");
  Serial.print(HALL_SENSOR_AVRG);
  Serial.print(" +/- ");
  Serial.println(HALL_SENSOR_WIGGLE_ROOM);
  #endif
  }

delay(SAFETY_CAKANJE); // po potrditvi konfiguracije in odložitvi ploščice ima uporabnik SAFETY_CAKANJE/1000 sekund da umakne roko

//prvic po prizigu preskočimo, ker smo home-ali že v setupu,
//sicer pa home-amo vsakič med po preklopu načina oz. zagonu:
if(ponovna_nastavitev)zapiranje_roze();

//pokažemo izbran način obratovanja preko LED na UI:
digitalWrite(ROZA_LED,(nastavitve&mask_roza)==1);
digitalWrite(CSVLOG_LED,(nastavitve&mask_log)==2);
digitalWrite(DEMO_LED,(nastavitve&mask_demo)==4);
digitalWrite(START_LED,1);

delay(MODE_CHANGE_WAIT);

if(!ponovna_nastavitev)ponovna_nastavitev=1;

//dokler je plošča odložena obratuje:
while((analogRead(HALL_SENSOR)<=(HALL_SENSOR_AVRG+HALL_SENSOR_WIGGLE_ROOM)) ){
  
#if DEBUG_GENERIC
Serial.print("GARBAM\n  parametri:");
Serial.print((nastavitve&mask_demo)==4);
Serial.print((nastavitve&mask_log)==2);
Serial.println((nastavitve&mask_roza)==1);
Serial.print("HALL: ");Serial.println(Serial.print(analogRead(HALL_SENSOR)));
#endif

//Funkcija za obratovanje:
ObdelavaPodatkov((nastavitve&mask_demo)==4,(nastavitve&mask_log)==2,(nastavitve&mask_roza)==1,ponovna_nastavitev);

if(ponovna_nastavitev)ponovna_nastavitev=0;
}

#if DEBUG_GENERIC
Serial.println("NE GARBAM");
#endif

// če smo merili pošjemo stop da python neha beležiti podatke
#if CSVLOG
if((nastavitve&mask_log)<=3&&(nastavitve&mask_log)>=1)Serial.println("stop");
#endif

//po izhodu preverimo nov izbor:
nastavitve=ui_config();
ponovna_nastavitev=1;

}

//################################# FUNKCIJE #######################################

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
//Funkcija, ki nameni procesorski čas zgolj sinhronizaciji in prejemu podatkov o umirjenosti uporabnika

//Kot vhodno spremenljivko prejme podatek, če naj kliče regulacijo po prejetju meritev

void merjenja(bool ali_reguliram){
  
  static bool ledica=0;
  uint8_t eeg_prej=umir;

  if(bluetooth.available()>0){
  digitalWrite(CSVLOG_LED,ledica);
  ledica=!ledica;
  }

  //dokler smo v loopou gorita rdeča in rumena lučka CSVLOG
  //v loopu smo dokler se ne spremeni vrednost umirjenosti
  while(umir==eeg_prej){
  mindwave.update(bluetooth,onMindwaveData);
  digitalWrite(CSVLOG_LED,ledica);
  ledica=!ledica;
  }

  //na podlagi vhodnega parametra kličemo funkcijo za regulacijo:
  if(ali_reguliram)regulacija(umir);
  
  }
//----------------------------------------------------------------------------
// Funkcija za obdelavo podatkov:
// VHODNI parametri: nastavitev stikal na UI plošči (DEMO, CSVLOG, ODPIRANJE ROŽE)
//                   podatek o ponastavitvi nastavitev/meritev

void ObdelavaPodatkov(bool demo,bool ali_merim, bool odpiranje,bool reset_settings) {
  
  //demo mode spremenljivki:
  static bool demo_flag=0; //če sem prišel do zgornje ali spodnje meje
  static bool smer=0;      // da lahko spreminjam smer vrtejna v demo mode

  static byte stevec_meritev=0;
  const byte ST_POVPRECIJ=30;
  static uint16_t meritve[ST_POVPRECIJ];
  static unsigned long prejle=0;

  static uint8_t umirjenost_prej=0;
  
  //prvic po inicializaciji arraya meritev ga zafilamo z ničlami
  //oz. resetiramo ob ponovnem zagonu/spremembi mode-a:
  if(reset_settings)
  {
    for(byte a=0;a<ST_POVPRECIJ;a++)meritve[a]=0;
  }
  
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
  delay(DEMO_CAKANJE); //počakamo, da se lahko nagledamo odprte/zaprte rože
  }
  
  //če se odločimo za meritve ali odpiranje ROŽE glede na umirjenost
  else if(ali_merim||odpiranje) //z else if damo prednost demo-tu
  {  
    //vsako sekundo namenimo procesorski čas prejemanju izmerjenih podatkov:
    if(abs(micros()-prejle)>1000000){merjenja(odpiranje);prejle=micros();}
    
  //za izpis na zaslonu računalnika (CSVLOG):
  if(ali_merim){ //merjenje je samo za beleženje in povprečenje

  #if DEBUG_MERITVE
  static bool print_flag=1;
  if(print_flag){
  Serial.print("Aktivno povprečje(0) ali povprečje zadnjih 30 meritev(1)? ");
  Serial.println(meritve[ST_POVPRECIJ-1]!=0); //s tem ve če PC izpise "aktivno povprečje" ali "povprečje zadnjih 30 meritev"
  print_flag=0;
  }
  #endif
  #if CSVLOG
  Serial.println(meritve[ST_POVPRECIJ]!=0);
  #endif
  
  // za ponastavitev kazalca v arrayu:
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

  //izračunamo povprečje in izpišemo glede na spremenljivko ali_merim (python log):
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
  }
}
//-------------------------------------------------------------------------------------------------------
//Funkcija za regulacijo:
//Kot vhodni parameter prejme trenutno stanje umirjenosti

void regulacija(int trenutno_stanje)
{
static bool first_reg=false;
static int pozicija_cilj=1;
static int motor=0;
static int delta_motor=0;
int regulacijsko_povprecje;
static int meditacija_prej;

if(first_reg!=false || trenutno_stanje!=0) //preventiva za prvo izvedbo
{ // upoštevamo novo vrednost meditacije oz. umirjenosti in pomnimo le zadnjo prejšnjo:
 if (first_reg==false)
  {
  if(trenutno_stanje!=meditacija_prej)meditacija_prej=trenutno_stanje;
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
Serial.print(" | Pozicija cilj: ");
Serial.println(pozicija_cilj);
#endif


// zančno primerjamo trenutno in ciljno pozicijo ter obračamo motor
// vmesni pogoj definira smer vrtenja
while(!(vrtenje(HITROST_MOTORJA,0,KORAKI_MOTORJA,MAX_POZICIJA_MOTORJA-pozicija_cilj))) 
{//manjsa cifra je bolj odprta roza, zato rabis max-trenutno futrat kot cilj
}    
                                                               
}
}

//----------------------------------------------------------------------------------------------------------
void zapiranje_roze()
{//Funkcija, ki home-a napravo, torej prvo zapre in nato odpre celo rožo

//z vsemi rdečimi LED na UI plošči naznani home-anje:
digitalWrite(ROZA_LED,0);
digitalWrite(CSVLOG_LED,0);
digitalWrite(DEMO_LED,0);
digitalWrite(START_LED,0);
  
  bool utrip=0; //za heartbeat START_LED (sočasno brlita zelena in rdeča štart LED)
  
  //zanka se izhvaja dokler funkcija vrtenje ne vrne "1" (torej se vrti dokler ne zadane END_SWITCH)
  //ODPIRANJE je prvo ZARAD END SWITCHA
  uint32_t zacetni_cas_homanja=micros();
  while(!(vrtenje(2000,1,1000,-1))){
    if((micros()-zacetni_cas_homanja)>END_SWITCH_TIMEOUT)break;//ce se homea predolg prekini homeanje
    digitalWrite(START_LED,utrip);
    utrip=!utrip;}

  delay(1000);

  //ko se home-a (odpre) se potem še zapre:
  zacetni_cas_homanja=micros();
  while(!(vrtenje(2000,0,1000,-1))){
    if((micros()-zacetni_cas_homanja)>OPENING_TIMEOUT)break;//ce se zapira predolg prekini proces
    digitalWrite(START_LED,utrip);
    utrip=!utrip;}
    
}
//----------------------------------------------------------------------------------------------
//funkcija za vrtenje motorja:
//VHODNI parametri: hitrost vrtenja, smer vrtenja (predznak hitrosti), število korako(ločljivost "korakov")
//in tudi ciljna pozicija (če je -1 se ignorira parameter)
//-> manjše število korakov pomeni hitrejša izvedba for zanke in vrnitev ven iz funkcije
//VRNE: informacijo, če se je roža do konca zaprla (beremo stikalo END_SWITCH),do konca odprla (če je pozicija na definiranem maksimumu)
//      ali pa če je dosegla ciljno pozicijo

bool vrtenje(int hitrost, bool smer, int stevilo_korakov,int poz_cilj)
{ //če ne želimo uporabljati regulacij nastavimo cilj na -1
// +/TRUE smer je odpiranje, -/FALSE smer je zapiranje
static int stepsPerSecond;
static int pozicija=MAX_POZICIJA_MOTORJA; //na startu misli da je cisto zaprt
static uint32_t micros_prej=micros();
bool knof=0;

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
else if (pozicija<poz_cilj) {stepsPerSecond = -hitrost;}
else stepsPerSecond = hitrost;

for(int i=stevilo_korakov;(i>0)&&(pozicija!=poz_cilj)&&(!knof);--i){
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
    }}

if((!digitalRead(END_SWITCH))&&(stepsPerSecond>0))
{ knof=1; //spremenljivka, ki naznani doseg ekstrema
  pozicija=0;
}
}    
digitalWrite(PIN_STEP1_STEP, LOW); //dodatna vrstica, preventivno da drži motor pri miru  

#if DEBUG_MOTOR
Serial.print("pogoj1: ");Serial.print(pozicija>MAX_POZICIJA_MOTORJA && smer>0);
Serial.print(" | pogoj2: ");Serial.print(pozicija==0 && smer<0);
Serial.print(" | pogoj3: ");Serial.println(!digitalRead(END_SWITCH));
Serial.print("pozicija: ");Serial.print(pozicija);
Serial.print(" | hitr: ");Serial.println(stepsPerSecond);
#endif

//vrne "1" če je roža prišla do keregakol ekstrema ali pa do cilja
return((pozicija>=MAX_POZICIJA_MOTORJA && stepsPerSecond<0) || (knof) || (pozicija==poz_cilj));
}
//------------------------------------------------------------------------------------------------
// funkcija za konfiguracijo glede na nastavitve na UI plošči
// vrne masko glede na pritisnjena stikala: DEMO,LOG,ROŽA
byte ui_config(void){

// maski za beleženje meritev:
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

start_gumb=0;//ponastavimo spremenljivko

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
