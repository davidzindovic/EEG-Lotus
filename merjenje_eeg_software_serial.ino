#include <Mindwave.h>
#include <SoftwareSerial.h>

Mindwave mindwave;
SoftwareSerial blu(A2,8);

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
  Serial.println();
}
void loop() {
  mindwave.update(blu,onMindwaveData);
}
