/*###############################################################
 * 
 *     This is a DEMO version of the original code designed
 *     by Elia Lazzari for EliusLab. If you want some help
 *     please contact me directly at elia.lazzari@gmail.com
 * 
 * ##############################################################
 */

#include <EEPROM.h>

int i = 0;

int ccSequence[16] = {127,0,127,0,127,0,127,0,127,0,127,0,127,0,127,0};
int pcSequence[16] = {5,5,4,4,4,4,5,5,4,4,4,4,5,5,4,4};

int bpm = 120;
int channel = 0;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  delay(4000);
}

// the loop routine runs over and over again forever:
void loop() {
  pulse();
  //60'000/4 = 15'000 --> 60'000 it's a minute and 4 is the 1/16 note
  //15 seconds/beat per minuts give you the period of one 1/16 note
  delay(15000/bpm);
}

void pulse() {
  if(i>15) i=0;
  midiSendPC(pcSequence[i],channel);
  midiSendCC(13,ccSequence[i],channel);
  i++;
}

void midiSendCC(byte controller, byte value, byte channel) {
  byte a = 0xb;
  byte b = a << 4;
  byte ch = b | channel;
  Serial.write(ch);
  Serial.write(controller);
  Serial.write(value);
}

void midiSendPC(byte program, byte channel) {
  byte a = 0xc;
  byte b = a << 4;
  byte ch = b | channel;
  Serial.write(ch);
  Serial.write(program);
}
