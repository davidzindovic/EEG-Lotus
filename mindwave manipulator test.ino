#include <Mindwave.h>
#include <SoftwareSerial.h>

Mindwave mindwave;
SoftwareSerial blu(A2,8);

int count=0;

void setup() {
  blu.begin(MINDWAVE_BAUDRATE);
  Serial.begin(MINDWAVE_BAUDRATE);
}

void onMindwaveData(){
  Serial.print("\tquality: ");
  Serial.print(mindwave.quality());
  Serial.print("\tattention: ");
  Serial.print(mindwave.attention());
  Serial.print("\tmeditation: ");
  Serial.print(mindwave.meditation());
  Serial.print("\tlast update: ");
  Serial.print(mindwave.lastUpdate());
  Serial.print("ms ago");
  uint8_t a=mindwave.meditation();
  Serial.println(a);
  Serial.println();
  
}
void loop() {
  static bool ena=0;
  if(!ena){
  delay(10000);ena=!ena;}
  mindwave.update(blu,onMindwaveData);
  //uint8_t a=mindwave.meditation();
  //Serial.println(a);
}
