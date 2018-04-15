#include <Bounce2.h>
#include <MIDI.h>

//Rotary Stuff
#define numKnobs 8

const uint8_t outA[] =  {4,   2,    8,    9,    20,   19,   30,   27};
const uint8_t outB[] =  {5,   3,    10,   23,   11,   32,   12,   28};
const uint8_t RECCNum[] = {74,  71,   74,   71,   74,   71,   74,   71};
const uint8_t REChNum[] = {10,  10,   1,    1,    2,    2,    3,    3};
int state[numKnobs] = {0};
int lasts[numKnobs] = {0};
uint8_t values[numKnobs] = {100};

//Slider Stuff
#define numSliders 4

int slidermin = 270;
int slidermax = 1024;

const uint8_t  sliderPins[] = {A0, A3, A2, A1};
const uint8_t SLCCNum[] = {7,  7,   7,   7};
const uint8_t SLChNum[] = {10,  1,   2,  3};
int lastslider[numSliders] = {0};

//Button Stuff
#define numButtons 8

const uint8_t buttonNumbers[] = {0, 1,  2,  3,  4,  5,  6,  7};
const uint8_t buttonPins[] =    {6, 7,  21, 22, 26, 25, 31, 29};
const uint8_t BTCCNum[] =       {73, 70, 73, 70, 73, 70, 73, 70};
const uint8_t BTChNum[] =       {10, 10, 1,  1,  2,  2,  3,  3};

Bounce * buttons = new Bounce[numButtons];

//Midi Stuff
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {

  for (int i = 0; i < numKnobs; i++)
  {
    pinMode (outA[i], INPUT_PULLUP);
    pinMode (outB[i], INPUT_PULLUP);
    lasts[i] = digitalRead(outA[i]);
  }

  for (int i = 0; i < numButtons; i++)
  {
    //pinMode(buttonPins[i], INPUT_PULLUP);
    buttons[i].attach( buttonPins[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(25); // interval in ms

  }

  Serial.begin (250000);
  usbMIDI.begin();
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  for (int i = 0; i < numSliders; i++)
    doSlider(i);

  for (int i = 0; i < numKnobs; i++)
    doKnobs(i);

  for (int i = 0; i < numButtons; i++)
    doButton(buttonNumbers[i], digitalRead(buttonPins[i]), i);


  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}

void doSlider(uint8_t sliderNum) {
  int sliderVal = analogRead(sliderPins[sliderNum]);
  sliderVal = (floor(127 * (1 - max(0, min(1, ((float)sliderVal - (float)slidermin) / ((float)slidermax - (float)slidermin))))));

  if (sliderVal >= lastslider[sliderNum] + 4 || sliderVal <= lastslider[sliderNum] - 4) {
    lastslider[sliderNum] = sliderVal;
    SendMidi(SLCCNum[sliderNum], lastslider[sliderNum], SLChNum[sliderNum]);
  }
}

void doKnobs(uint8_t knobNum)
{
  int lastVal = values[knobNum];
  state[knobNum] = digitalRead(outA[knobNum]);
  if (state[knobNum] != lasts[knobNum]) {

    if (digitalRead(outB[knobNum]) != state[knobNum]) {
      values[knobNum] = min(127, values[knobNum] + 3);
    } else {
      values[knobNum] = max(0, values[knobNum] - 3);
    }

    if (lastVal != values[knobNum])
    {
      SendMidi(RECCNum[knobNum], values[knobNum], REChNum[knobNum]);
    }
  }
  lasts[knobNum] = state[knobNum];
}

void doButton(uint8_t buttid, bool down, uint8_t index)
{
  buttons[index].update();

  if ( buttons[index].fell() ) {
    SendMidi(BTCCNum[index], 0, BTChNum[index]);
  }

  else if (buttons[index].rose() ) {
    SendMidi(BTCCNum[index], 127, BTChNum[index]);
  }
}

void SendMidi(uint8_t CCNum, uint8_t Val, uint8_t Channel){
    usbMIDI.sendControlChange(CCNum, Val, Channel);
    MIDI.sendControlChange(CCNum, Val, Channel);
}

