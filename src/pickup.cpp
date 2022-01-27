#include <Arduino.h>
#include "megafm.h"
#include "leds.h"
#include "pots.h"

void doPickup(byte number, byte data) {

  int target = -1;
  switch (number) {
    //@formatter:off
    //OP1
    case 18: target=0;  break; //detune
    case 27: target=1;  break; //multiple
    case 19: target=2;  break; //op level
    case 29: target=4;  break; //attack
    case 21: target=5;  break; //decay1
    case 25: target=7;  break; //sustain
    case 17: target=6;  break; //sustain rate
    case 30: target=8;  break; //release
    //OP2
    case 31: target=18; break; //detune
    case 32: target=19; break; //multiple
    case 40: target=20; break; //op level
    case 36: target=22; break; //attack WAS 59
    case 44: target=23; break; //decay1 WAS 50
    case 42: target=25; break; //sustain WAS 60
    case 34: target=24; break; //sustain rate WAS 55
    case 11: target=26; break; //release WAS 52
    //OP3
    case 20: target=9;  break; //detune
    case 24: target=10; break; //multiple
    case 16: target=11; break; //op level
    case 8:  target=13; break; //attack
    case 0:  target=14; break; //decay1
    case 7:  target=16; break; //sustain
    case 45: target=15; break; //sustain rate
    case 37: target=17; break; //release
    //OP4
    case 47: target=27; break; //detune
    case 39: target=28; break; //multiple
    case 38: target=29; break; //op level
    case 46: target=31; break; //attack
    case 33: target=32; break; //decay1
    case 41: target=34; break; //sustain
    case 43: target=33; break; //sustain rate
    case 35: target=35; break; //release

    case 3:  target=43; break; //feedback
    case 15: target=36; break; //lfo 1 rate
    case 12: target=37; break; //lfo 1 depth
    case 10: target=38; break; //lfo 2 rate
    case 9:  target=39; break; //lfo 2 depth
    case 14: target=40; break; //lfo 3 rate
    case 2:  target=41; break; //lfo 3 depth
    case 6:  target=46; break; ///arp rate
    case 5:  target=47; break; //arp range
    case 48: target=48; break; //vibrato rate WAS 7
    //@formatter:on
  }

  if (target != -1) {

    if (fmBase[target] > data + 10) { rightArrow(); }
    else if (fmBase[target] < data - 10) { leftArrow(); }
    else {
      //MATCH
      pickup[number] = 0;
      movedPot(number, data, 0);
    }

  } else {
    //disable pickup
    pickup[number] = 0;
    movedPot(number, data, 0);
  }

}
