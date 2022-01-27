#include <Arduino.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include "leds.h"
#include "megafm.h"
#include "preset.h"

int dumpCounter = 0;
byte dump;
bool alternator;
byte flash;

byte val[] = {0, 128};

/*
const uint16_t bankOffsets[]={0,7900,15800,23700,31600,39500};

void setIndex(){
  index=(preset*79)+bankOffsets[bank];
}
*/

void (*main_entry_point)(void) = 0x0000;

void sendDump() {

  Timer1.detachInterrupt();
  Timer1.stop();

  digit(0, 5);
  digit(1, 18);

  switch (bank) {

    case 0:
      if (!ab) { for (int i = 0; i < 3950; i++) { mem[i] = EEPROM.read(i); }}
      else { for (int i = 0; i < 3950; i++) { mem[i] = eRead(i); }}
      break;

    case 1 ... 5:

      uint16_t offset = bankOffsets[bank - 1] + 3950;
      if (ab)offset += 3950;
      for (int i = 0; i < 3950; i++) { mem[i] = eRead(offset + i); }
      break;

  }

/// SEND

  byte nill = 0;

  Serial.write(240);
  delay(1);
  Serial.write(100);
  delay(1);

  for (int i = 0; i < 3950; i++) {

    if (mem[i] > 127) {
      Serial.write(mem[i] - 128);
      delay(1);
      Serial.write(1);
      delay(1);
    } else {
      Serial.write(mem[i]);
      delay(1);
      Serial.write(nill);
      delay(1);
    }

  }
  Serial.write(247);
  Serial.flush();

  main_entry_point();
}

void recieveDump() {

  dump = 0;
  Timer1.detachInterrupt();
  Timer1.stop();

  digit(0, 16);
  digit(1, 18);

  while (dump != 247) {

    if (Serial.available()) {

      dump = Serial.read();

      if (dumpCounter == 0) { if (dump == 240) { dumpCounter++; } else { dump = 247; }}//must be sysex or cancel
      else if (dumpCounter == 1) {
        if (dump == 100) { dumpCounter++; }
        else { dump = 247; }
      }//must be 100 manuCode or cancel

      else {

        alternator = !alternator;

        if (alternator) {
          flash = dump;
        } else {
          flash += val[dump];
          //
          mem[dumpCounter - 2] = flash;
          dumpCounter++;
        }

      }
    }

  }

  byte ledLast;

  ledNumber(1);

  byte count1 = 0;
  byte count2 = 0;

  switch (bank) {

    case 0:
      if (!ab) {
        for (int i = 0; i < 3950; i++) {
          EEPROM.write(i, mem[i]);
          count1++;
          if (count1 > 39) {
            count1 = 0;
            count2++;
            ledNumberForced(count2);
          }
        }
      }
      else {
        for (int i = 0; i < 3950; i++) {
          eWrite(i, mem[i]);
          delay(2);
          count1++;
          if (count1 > 39) {
            count1 = 0;
            count2++;
            ledNumberForced(count2);
          }
        }
      }
      break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      uint16_t offset = bankOffsets[bank - 1] + 3950;
      if (ab)offset += 3950;
      for (int i = 0; i < 3950; i++) {
        eWrite(i + offset, mem[i]);
        count1++;
        if (count1 > 39) {
          count1 = 0;
          count2++;
          ledNumberForced(count2 + 1);
        }
      }
      break;

  }

  main_entry_point();

}
