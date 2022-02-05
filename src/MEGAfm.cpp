/*
cd /Users/a/Documents/bootloaderT cd /Users/a/Documents/bootloaderT&&cp -f /private/var/hex/MEGAfm.ino.hex /Users/a/Documents/bootloaderT&&python tools/hex2sysex/hex2sysex.py --syx -o firmware.syx MEGAfm.ino.hex

LOG Pickup optimized
Anti MIDI feedback


Algorithm knob doesn't do parameter pickup, and Vibrato depth doesn't display its value on screen

J’ai testé le 2.4 aujourd’hui :

BUG :
tout les faders sauf detune n’affichent plus leurs valeurs ?
( NB: le parameter pick up etait OFF quand j’ai vu ca - en l’activant, il ne marche pas sur tout les faders sauf detune )


J’ai principalement fait des presets utilisant le fat mode depuis que j’ai cette machine, et surtout en mode 1 octave,
en utilisant les premiers crans pour colorer le son et le reste de la course du portard pour le faire muter completement quand c’est pertinent

FAT MODE :
le nouveau fat mode est interessant mais du coup ca ne marche vraiment pas a tout les coups…
Serait il possible que les 2 “modes” soient switchables ? ( ancient / nouveau )
Perso j’aurais plus d’usage de l’ancien.

Serait il aussi possible en mode 1 octave, que la courbe de valeur soit (plus) exponentielle ?
pour avoir plus de finesse sur les premiers pas
d’abord parce qu’il y a beaucoup de nuances possibles entre les valeurs 1 et 4 ( actuelles ) et que pour le jeu en live c’est super touchy

Legato :
Je ne comprend pas la necessité d’avoir a activer l’ARP pour que le pitch soit updaté a chaque nouvelle note
(vu que c’est le mode naturel de jeu ? ), a moins que cela soit une contrainte technique ?

Le glide ON seulement quand jeu legato serait top…
ca evite d’avoir des notes seule qui ont pas un pitch steady

Autre chose que je remarque est qui manque, c’est ca :
je presse une note N
je presse une 2e note N+1, le pitch change, la note N est toujours enfoncée.
lorsque je lache la note N+1, le pitch est toujours a N+1 et ne revient pas a N, alors que la note N est toujours enfoncée.

ca me semble indispensable pour jouer en mode solo / lead / basslines

ca marche si l’ARP est ON & rate > 0, mais lorsque rate = 0, ca ne marche pas


Parameter pickup :
c’est tres bien pour le live, mais pour la prog de presets, on peut se louper, il faut aller vraiment doucement.

Je me permet de suggerer quelques chose de pratique ( qui pourrait etre caduc si j’arrivais a faire marcher le plugin sous Cubase ) :

ca serait un bouton qui quand il est appuyé, permet d’afficher la valeur du pot / fader manipulé
( sans envoyer les valeurs modifiées aux chips of course )

du coup quand on veut obtenir une valeur, on appuie & on tourne et on sait ;)
quand on reprend un preset pour le modifier ou en cours de programmation c’est MEGA utile haha


LFO DEPTH :
Super fan du fait qu’on puisse assigner la velocité / mod wheel / aftertouch a la depth !!

Ca serait vraiment le luxe, si les potards de Depth avaient encore de l’influence comme attenuateurs ?
Exemple : quand il y 100% d'influence de la velocité sur par exemple le volume d’un Operateur qui est modulateur,
c’est beaucoup trop, souvent on a besoin de moduler en +/- 5 ou +/- 10, pas FULL.

SETUP MODE :
Les options choisies dans le setup pourraient etre stockées par presets ?


unison legato "follow legato” ou “permanent”
choose between fat mode down left up right or mixed
display preset number after program change

Poly12 sustain pedal bug
I’ve found that when playing with a sustain pedal in Poly12 mode, pressing a key twice does not cause the sustained note to retrigger. Interestingly, it does do this in Wide6 mode. Would it be possible to implement this in a future firmware update, or is there a way to turn this on for Poly12 that I just completely missed?


Hey again! So when i was in setup mode (SE), after activating ‘midi thru’ on LFO 1 link button ..
it seems that i couldn’t exit SE mode by pushing voice 1 time, and i could not flip thru different voice modes…
and i believe the screen was blank… perhaps im not doing something right. because it took me about 4 tries to successfully activate midi thru… i didn’t want to try and replicate the problem again because it’s working now… but like i said i may have pushed a button or hit another knob, or did something wrong.


DONE
Fix PA function (not updating all faders/pots)

TODO

Steal closest note so it glides logically
figure out issue with detun3

check tuning?!

Was playing around with the dual ch3 mode and found some odd behaviour, that I'm guessing is not intentional?
Firstly, seems like the detune sliders are affecting the wrong operators. OP1 detune slider detunes OP4, OP2 slider detunes OP1, OP3 slider detunes OP3 and OP4 slider detunes OP1.
Secondly, detuning an operator only has effect while playing the same note. Playing different notes seems to reset the detune again.
And lastly, just wondering if it's as intended that only OP4 pitch is tracking the keyboard, whereas the other three are fixed?

- pitchbend cc
When im sequencing the pitchbend cc from my sequencer after a few playback the mega starts sounding weird. Rebooting fixes it for a while. Maybe its an lfo acting up. Cant figure it out. I have cleared out all 3 lfos first but maybe its these that dont reset properly.

- programchange. When this is used sometimes a completely wrong program is selected. Rebooting fixes it temporarily.

*/

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <TimerOne.h>

#include "megafm.h"
#include "lfo.h"
#include "mux.h"
#include "FM.h"
#include "leds.h"
#include "isr.h"
#include "preset.h"

// Whether one second has elapsed since device boot.
// Used in: loop.cpp, pots.cpp
bool secPast = false;
byte lastSentCC[2];
byte lastSentMega[2];
byte lastSentYm[2];
//0=chip1 down chip2 up 1=both chips go up and down (mixed)
bool fatSpreadMode;
// Whether the knobs/sliders should use 'pickup' behavior
// Used in: buttons.cpp, megafm.cpp, pots.cpp
bool pickupMode = true;
// Whether each knob/slider has been picked up.
// Used in: pickup.cpp, pots.cpp, preset.cpp
byte pickup[49];
// Similar to pickup? Ignore volume knob immediately after loading a preset
// Used in: buttons.cpp, megafm.cpp, pots.cpp, preset.cpp
bool ignoreVolume;
// Some kind of defer countdown for pitch bend
// Used in: loop.cpp, midi.cpp
int bendyCounter;
// True for each MIDI note included in arpeggiation
// Used in: arp.cpp, voice.cpp
bool arpNotes[128];
// Note priority: lowest (0), highest (1), or last (2).
// Used in: buttons.cpp, megafm.cpp, midi.cpp, voice.cpp
byte notePriority = 2;
// Track the last note for glide.
// Distance to go from last note to future.
// Used in: midi.cpp, voice.cpp
byte lastNotey[40];
/**
 * When the arp needs to be resynced; MIDI clock changes or arp mode change
 * Used in: buttons.cpp, midi.cpp
*/
bool resyncArp = false;
bool stereoCh3;
/**
 * We have 100 presets arranged in 6 banks. Change the bank by pressing
 * an LFO waveform during the preset loading processes (blinking display).
 * FYI presets 0-50 are in the internal EEPROM, presets 50-99 of bank 1
 * and all presets of banks 2-6 are in the external SPI EEPROM.
 */
byte bank;
int lfoLedCounter;
byte seq[16];
byte seqStep;
byte seqLength;
bool seqRec;
bool flasher;
int flashCounter, flashCounter2, bankCounter;
/**
 * When set to true, megaFM boots in test mode (hold reset sat startup 
 * to enter test mode after a factory reset) : (plays some chords at 
 * various volumes and fires a note on every channel in a loop). This 
 * is used for me to check that the chips and volume control circuits 
 * are ok.
 */
bool test;
/**
 * This mode is used for sending and receiving preset sysex dumps.
 * Hold preset up or down at startup to boot in send or receive modes.
 */
byte sendReceive;
byte rootNote, rootNote1;
bool thru = true;
int setupCounter;
bool justQuitSetup;
bool invertedSquare[3];
bool invertedSaw[3];

byte muxChannel;
bool voiceSlots[12];
byte noteOfVoice[12];

// Velocity affects lfo1
bool lfoVel;
// Mod affects lfo2
bool lfoMod;
// Aftertouch affects lfo3
bool lfoAt;
float mpeBend[12];
byte bendUp = 12;
byte bendDown = 12;
int bendRoot = -1;
float destiFreq[12];
float freq[12];
// freqTotal = freq + vib * bend
float freqTotal[12];
float freqLast[12];
float glideIncrement[12];
int volumeCounter;
bool changeVol;
bool dotOn;
bool arpClockEnable;
bool lfoClockEnable[3];
bool vibratoClockEnable;
byte glide = 0;
byte lastVol, vol;
bool arpJustWentOff;
bool setupMode;
bool fineChanged;
byte arpMidiSpeedPending, arpMidiSpeed;
byte fine;
byte presetCounts;
bool presetTargetMode;
byte presetTargetModeChanged;
int targetPresetModeTimer;
bool targetPresetFlasher;
int scrollDelay, scrollCounter;
byte noiseTableLength[3];
// Buffer for sysex preset dumps.
byte mem[3950];
int notey[12];
byte arpMode, arpModeLast;
bool looping[3];
int arpIndex;
bool resetHeld;
bool shuffled;
bool voiceHeld;
int lastNumber = 255;
byte lfoClockSpeed[3];
// To keep the lfo beat in sync with the MIDI master clock, 
// this is used to store a new lfo rate until the incoming MIDI clock counter resets, 
// whereas changing immediately would make it go out of sync.
byte lfoClockSpeedPending[3];
byte absoluteClockCounter;
YM2612 ym;
int heldKeys;
byte lastNote, dotTimer;
int arpCounter, arpStep, preset;
float lfoStepF[3];
bool arpModeHeld;
byte inputChannel = 1;
bool changedChannel;
/**
 * Whether to turn off the voice slot when the pedal is lifted.
 */
bool pedalOff[12];
bool pedal;
/**
 * Used to track if any notes are stored in the arpeggiator stack/array (true=empty).
 * Notes are added or removed from the stack when MIDI notes on/off are parsed.
 */
bool emptyStack;
bool fatMode;
int arpButtCounter;

/**
 * Used to adjust the global tuning of MEGAfm (when voice mode button is held
 * and volume knob is turned) by multiplying it by a range of 1 to 2 (1 octave)
 */
float finey = 1;
int ledNumberTimeOut;
LedControl mydisplay = LedControl(13, 29, 12, 1);
int presetLast;
int secCounter;
int vibIndex, vibIndexLast;
int arpDivider;
VoiceMode voiceMode = kVoicingPoly12;
bool sync;
/**
 * Banks of 100 presets are sent and received in 2 halves (A=presets 0-49. B=presets 50-99).
 * I'm unsure why I chose to do this, from memory the serial USART had trouble buffering
 * more than 50 presets at a time.
 */
bool ab;
int potLast[64];
int lfoDepth[3];
bool buttLast[19];
int algoLast;
/**
 * These are all the parameters before they get modulated by LFO
 * (they are loaded by the preset and changed by MIDI CC or knob/fader movements).
 * Basically it's all the stuff we can modulate to change things.
 */
byte fmBase[51], fmBaseLast[51], fmBaseLastNumber[51];
/**
 * These are the same values as fmBase but after LFO modulation,
 * before being shifted and sent to the FM chips.
 */
int fmData[51], fmDataLast[51];
bool linked[3][51];
bool dontShow[50];
byte lfoRandom[3][32];
byte randomIndex[3];
bool pressedUp, pressedDown;
bool saved;
int presetFlasher;
byte updatePitchCounter;
int shuffleCounter2;
bool shuffleAlternator;
int shuffleCounter;
byte linkCounter;
byte selectedLfo, selectedLfoLast;
bool cleared;
/** Which LFO chain button is being pressed. 1, 2, or 3; 0 = none. */
byte chainPressed;
byte targetPot, targetPotLast;
byte masterChannelOut = 1;
float ch3x[4];
byte lfoShape[3];
byte lfo[3], lfoLast[3];
int lfoStep[3];
int lfoStepLast[3];
/**
 * incoming MIDI aftertouch (the at can be assigned to one of the LFO)
 */
int at;
/**
 * These are aftertouch values. I interpolate between last (current) and
 * dest (next) values with some gliding so the aftertouch response isn't too choppy.
 * FYI aftertouch can override one of the LFOs.
 */
int atDest, atLast, atGlideCounter;
bool lfoNewRand[3];
int lfoCounter[3], lfoSpeed[3];
bool retrig[3];
int fatLast;
float fat;
float bendy = 0;
bool setupChanged;
bool lastLfoSetting[3];
int vibPitch;
byte presetTp;
int vibCounter;
int shuffleTimer;
byte seed;
byte potClock;
bool mpe;

void enterSetup() {
  digit(0, 5);
  digit(1, 18);
  setupMode = true;
  ledSet(13, thru);
  ledSet(14, pickupMode);
  ledSet(19, fatSpreadMode);
}

void setup() {
  lastSentCC[0] = 255;

  fillAllLfoTables();

  Wire.begin();

  potClock = 19;

  //dPot
  digitalWrite(13, LOW);
  digitalWrite(29, LOW);
  digitalWrite(21, LOW);

  //dPot
  pinMode(potClock, OUTPUT); //clock
  pinMode(18, OUTPUT); //data
  pinMode(21, OUTPUT); //latch

  // LOAD SETTINGS

  //3950 = bit 0 thru
  //3950 = bit 1 ignore preset volume
  //3950 = bit 2-7 noisetableLength

  //3951 = midi channel
  //3952 = last preset
  //3953 bit0  =  midi clock lfo1
  // 3953 bit1    midi clock lfo2
  // 3953 bit2 =  midi clock lfo3
  // 3953 bit3 =  midi clock vibra
  // 3953 bit4 =  midi clock  arp
  // 3953 bit5 =  fatMode

  //3954 = pickup mode (0=on)

  //3958 =bendDown
  //3959 =bendUp
  //3960 =mpe mode

  //3961 =vel >lfo1 depth
  //3962 =mod >lfo2 depth
  //3963 =at >lfo3 depth

  //3964 lastbank

  //3965 brightness.

  //3966 = bit 0 inv saw1
  //3966 = bit 1 inv saw2
  //3966 = bit 2 inv saw3
  //3966 = bit 3 inv square1
  //3966 = bit 4 inv square2
  //3966 = bit 5 inv square3
  //3966 = bit 6 stereoCh3

  //3967 = note priority 0=low 1=high 2=last

  //3968 = bit 0 fatSpreadMode

  byte input = EEPROM.read(3968);
  fatSpreadMode = bitRead(input, 0);

  notePriority = EEPROM.read(3967);
  if (notePriority > 2)notePriority = 0;

  input = EEPROM.read(3966);

  invertedSaw[0] = bitRead(input, 0);
  invertedSaw[1] = bitRead(input, 1);
  invertedSaw[2] = bitRead(input, 2);
  invertedSquare[0] = bitRead(input, 3);
  invertedSquare[1] = bitRead(input, 4);
  invertedSquare[2] = bitRead(input, 5);

  stereoCh3 = bitRead(input, 6);

  noiseTableLength[0] = 0;
  bitWrite(noiseTableLength[0], 0, bitRead(EEPROM.read(3950), 2));
  bitWrite(noiseTableLength[0], 1, bitRead(EEPROM.read(3950), 3));
  noiseTableLength[0] += 2;

  noiseTableLength[1] = 0;
  bitWrite(noiseTableLength[1], 0, bitRead(EEPROM.read(3950), 4));
  bitWrite(noiseTableLength[1], 1, bitRead(EEPROM.read(3950), 5));
  noiseTableLength[1] += 2;

  noiseTableLength[2] = 0;
  bitWrite(noiseTableLength[2], 0, bitRead(EEPROM.read(3950), 6));
  bitWrite(noiseTableLength[2], 1, bitRead(EEPROM.read(3950), 7));
  noiseTableLength[2] += 2;

  if (bitRead(EEPROM.read(3950), 0)) {
    thru = 0;
  } else {
    thru = 1;
  }

  if (EEPROM.read(3961)) {
    lfoVel = 1;
  } else {
    lfoVel = 0;
  }
  if (EEPROM.read(3962)) {
    lfoMod = 1;
  } else {
    lfoMod = 0;
  }
  if (EEPROM.read(3963)) {
    lfoAt = 1;
  } else {
    lfoAt = 0;
  }

  if (EEPROM.read(3954)) { pickupMode = true; } else { pickupMode = false; }

  lfoClockEnable[0] = bitRead(EEPROM.read(3953), 0);
  lfoClockEnable[1] = bitRead(EEPROM.read(3953), 1);
  lfoClockEnable[2] = bitRead(EEPROM.read(3953), 2);
  vibratoClockEnable = bitRead(EEPROM.read(3953), 3);
  arpClockEnable = bitRead(EEPROM.read(3953), 4);
  fatMode = bitRead(EEPROM.read(3953), 5);

  ignoreVolume = bitRead(EEPROM.read(3950), 1);

  bank = EEPROM.read(3964);
  if (bank > 5)bank = 0;
  preset = EEPROM.read(3952);
  if (preset > 99) {
    preset = 0;
  }

  arpModeLast = 6;

  for (int i = 0; i < 16; i++) {
    readMux();
  }

  ym.setup(30, 10, 11, 11, 20, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23);

  setupFM();

  pinMode(A1, INPUT);
  digitalWrite(A1, HIGH);

  pinMode(22, INPUT);
  digitalWrite(22, HIGH); //pc6 retrig

  mydisplay.shutdown(0, false); // turns on display

  byte brightness = EEPROM.read(3965);
  if (brightness > 15)brightness = 10;//default;
  mydisplay.setIntensity(0, brightness); // 15 = brightest

  //show version
  digit(0, version0);
  digit(1, version1);
  //show dot
  mydisplay.setLed(0, 7, 6, 1);
  delay(500);
  mydisplay.setLed(0, 7, 6, 0);
  Timer1.initialize(150); //
  Timer1.attachInterrupt(isr); // attach the service routine here

  inputChannel = EEPROM.read(3951);
  if ((inputChannel > 16) || (inputChannel < 1)) {
    inputChannel = 1;
    EEPROM.write(3951, inputChannel);
  }

  //check if enter setup mode
  mux(15);
  if (!digitalRead(A1)) {
    enterSetup();
  }


  //mux(5);//preset up

  // MIDI port at 31250 baud
  Serial.begin(31250);
  //midiSetup();

  mux(14);
  if (!digitalRead(A1)) {
    sendReceive = 2;  //preset down = send midi dump
    bank = 0;
    showSendReceive();
  }
  mux(5);
  if (!digitalRead(A1)) {
    sendReceive = 1;  //preset up = get midi dump
    bank = 0;
    showSendReceive();
  }

  mux(13);
  if ((!digitalRead(A1)) ||
      ((EEPROM.read(0) == 255) && (EEPROM.read(1) == 255) && (EEPROM.read(2) == 255) && (EEPROM.read(3) == 255))) {
    //test mode
    digit(0, 16);
    digit(1, 18);

    for (int i = 0; i < 4000; i++) {
      EEPROM.write(i, factoryPresets[i]);
    }

    loadPreset();
    eWrite(69, 69);
    if (eRead(69) != 69) {

      while (1) {

        digit(0, 18);
        digit(1, 18);
        delay(200);
        digit(0, 21);
        digit(1, 21);
        delay(200);

      }

    }
    EEPROM.write(3964, 0); //bank 0
    test = true;
    bank = 0;
    preset = 0;

  }

  for (int i = 0; i < 50; i++) {
    ledSet(i, test);
  }

  for (int i = 0; i < 16; i++) {
    readMux();
  }
  loadPreset();
  loadPreset();

  bendUp = EEPROM.read(3959);
  if ((bendUp > 48) || (!bendUp)) {
    bendUp = 48;
  }
  bendDown = EEPROM.read(3958);
  if ((bendDown > 48) || (!bendDown)) {
    bendDown = 48;
  }

  if (EEPROM.read(3960) == 1) {
    mpe = 1;
  } else {
    mpe = 0;
  }

  if (preset == 0) {
    digit(0, 0);
    digit(1, 0);
  }
}

