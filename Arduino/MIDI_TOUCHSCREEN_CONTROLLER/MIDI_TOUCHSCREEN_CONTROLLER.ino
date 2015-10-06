/*###############################################################
#                                                               #
#          ARDUINO MIDI XY CONTROLLER FOR KORG KP2&KP3          #
#              Version: 2.0 with "BlinkM" Support               #
#  Designed by: Elia Lazzari on 06/10/2015 at 02:12 in Bologna  #
#     License: GNU GPL License, Copyright 2015 Elia Lazzari     #
#               http://eliuslab.com/projects/code               #
#                                                               #
################################################################*/

#include <Wire.h>
#include <EEPROM.h>

#define ADDRESS_XO    12
#define ADDRESS_YO    13
#define ADDRESS_XM    14
#define ADDRESS_YM    15

#define ENCODER_1     2
#define ENCODER_2     3
#define ENCODER_B     5
#define HOLD_BUTTON   4

#define MIDI_CH       0
#define X_MIDI        12
#define Y_MIDI        13

#define HOLD_LED      6

int holdstate = 0;
boolean hold = false;
boolean protocol = false;

int encoder0Pos = 0;
int encoder0PinALast = LOW;
int n = LOW;
int i = 0;

int y1 = A0;
int x2 = A1;
int y2 = A2;
int x1 = A3;

int xOffset = 0;
int yOffset = 0;
int xMax = 0;
int yMax = 0;

int x = 0;
int y = 0;
int touchMIDI = 92;

int xMap = 0;
int yMap = 0;
int xMapLED = 0;
int yMapLED = 0;

bool control = true;

void setup() {
  Wire.begin(); // set up I2C
  Wire.beginTransmission(0x09);// join I2C, talk to BlinkM 0x09
  Wire.write('p'); // ‘c’ == fade to color
  Wire.write(1); // value for red channel
  Wire.write(2); // value for blue channel
  Wire.write(0); // value for green channel
  Wire.endTransmission(); // leave I2C bus
  delay(4000);
  pinMode(ENCODER_1, INPUT);
  pinMode(ENCODER_2, INPUT);
  pinMode(ENCODER_B, INPUT);
  digitalWrite(ENCODER_2, HIGH);
  digitalWrite(ENCODER_1, HIGH);

  if (digitalRead(ENCODER_B)) {
    delay(1000);
    if (digitalRead(ENCODER_B)) {
      calibrate();
    }
  }

  xOffset = EEPROM.read(ADDRESS_XO);
  yOffset = EEPROM.read(ADDRESS_YO);
  xMax = EEPROM.read(ADDRESS_XM);
  yMax = EEPROM.read(ADDRESS_YM);

  pinMode(HOLD_BUTTON, INPUT);
  pinMode(HOLD_LED, OUTPUT);

  attachInterrupt(0, doEncoderA, CHANGE);
  attachInterrupt(1, doEncoderB, CHANGE);

  Serial.begin (9600);
  Serial.begin(31250);
  //Serial.begin(9600);
}

void doEncoderA() {
  // look for a low-to-high on channel A
  if (digitalRead(ENCODER_1) == HIGH) {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_2) == LOW) {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }
  else   // must be a high-to-low edge on channel A
  {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_2) == HIGH) {
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  midiSendPC(encoder0Pos, MIDI_CH);
  // use for debugging - remember to comment out
}

void doEncoderB() {
  // look for a low-to-high on channel B
  if (digitalRead(ENCODER_2) == HIGH) {
    // check channel A to see which way encoder is turning
    if (digitalRead(ENCODER_1) == HIGH) {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }
  // Look for a high-to-low on channel B
  else {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_1) == LOW) {
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  midiSendPC(encoder0Pos, MIDI_CH);
}

int readX() {
  int xVal;
  pinMode(y1, INPUT);
  pinMode(x2, OUTPUT);
  pinMode(y2, INPUT);
  pinMode(x1, OUTPUT);

  digitalWrite(x2, LOW);
  digitalWrite(x1, HIGH);

  delay(5); //pause to allow lines to power up
  xVal = analogRead(y1) - xOffset;
  return xVal;
}

int readY() {
  int yVal;
  pinMode(y1, OUTPUT);
  pinMode(x2, INPUT);
  pinMode(y2, OUTPUT);
  pinMode(x1, INPUT);

  digitalWrite(y1, HIGH);
  digitalWrite(y2, LOW);

  delay(5); //pause to allow lines to power up
  yVal = analogRead(x2) - yOffset;
  return yVal;
}

void holdFunction() {
  if (digitalRead(HOLD_BUTTON)) {
    hold = !hold;
    if (hold) midiSendCC(touchMIDI, 127, MIDI_CH);
    else midiSendCC(touchMIDI, 0, MIDI_CH);
    digitalWrite(HOLD_LED, hold);
    delay(250);
  }
}

void changeProtocolFunction() {
  if (digitalRead(ENCODER_B)) {
    protocol = !protocol;
    if (protocol) touchMIDI = 92;
    else touchMIDI = 95;
    delay(250);
  }
}

void calibrate() {
  //led Red
  digitalWrite(HOLD_LED, HIGH);
  delay(200);
  digitalWrite(HOLD_LED, LOW);
  delay(200);
  digitalWrite(HOLD_LED, HIGH);
  delay(200);
  digitalWrite(HOLD_LED, LOW);
  //PREMERE IL PUNTO X0;Y0 SUL DISPLAY
  while (!digitalRead(ENCODER_B)) {
    xOffset = readX();
    yOffset = readY();
    delay(50);
  }
  EEPROM.update(ADDRESS_XO, xOffset);
  EEPROM.update(ADDRESS_YO, yOffset);
  delay(2000);

  //led Red
  digitalWrite(HOLD_LED, HIGH);
  delay(200);
  digitalWrite(HOLD_LED, LOW);
  delay(200);
  digitalWrite(HOLD_LED, HIGH);
  delay(200);
  digitalWrite(HOLD_LED, LOW);
  //PREMERE IL PUNTO XMAX;YMAX SUL DISPLAY
  while (!digitalRead(ENCODER_B)) {
    xMax = readX();
    yMax = readY();
    delay(50);
  }
  EEPROM.update(ADDRESS_XM, xMax);
  EEPROM.update(ADDRESS_YM, yMax);
  //Led Flashing
  for (i = 0; i < 12; i++) {
    digitalWrite(HOLD_LED, HIGH);
    delay(50);
    digitalWrite(HOLD_LED, LOW);
    delay(50);
  }
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

void loop()
{
  if (control) {
    control = false;
    Wire.begin(); // set up I2C
    Wire.beginTransmission(0x09);// join I2C, talk to BlinkM 0x09
    Wire.write('p'); // ‘c’ == fade to color
    Wire.write(10); // value for red channel
    Wire.write(0); // value for blue channel
    Wire.write(0); // value for green channel
    Wire.endTransmission(); // leave I2C bus
  }
  holdFunction();
  changeProtocolFunction();
  x = readX();
  y = readY();

  if (x < 1000 & y < 1000) {
    control = true;
    //    Serial.print("x: ");
    //    Serial.print(x-100);
    //    Serial.print(" - y: ");
    //    Serial.println(y- 130);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > xMax) x = xMax;
    if (y > yMax) y = yMax;
    xMap = map(x, 0, xMax, 0, 127);
    yMap = map(y, 0, yMax, 0, 127);
    xMapLED = map(x, 0, xMax, 0, 255);
    yMapLED = map(y, 0, yMax, 0, 255);
    if (!hold) midiSendCC(touchMIDI, 127, MIDI_CH);
    midiSendCC(X_MIDI, xMap, MIDI_CH);
    midiSendCC(Y_MIDI, yMap, MIDI_CH);
    Wire.begin(); // set up I2C
    Wire.beginTransmission(0x09);// join I2C, talk to BlinkM 0x09
    Wire.write('h'); // ‘c’ == fade to color
    Wire.write(xMapLED); // value for red channel
    Wire.write(yMapLED); // value for blue channel
    Wire.write(255); // value for green channel
    Wire.endTransmission(); // leave I2C bus
  }
  else {
    if (!hold) midiSendCC(touchMIDI, 0, MIDI_CH);
  }
  //delay(100); //just to slow this down so it is earier to read in the terminal - Remove if wanted
}
