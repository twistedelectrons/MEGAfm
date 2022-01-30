#include <Arduino.h>
#include "mux.h"
#include "megafm.h"
#include "pots.h"
#include "buttons.h"

const byte range = 7;
const byte range2 = 10;

void Aon() { PORTB |= _BV (0); }

void Aoff() { PORTB &= ~_BV (0); }

void Bon() { PORTB |= _BV (1); }

void Boff() { PORTB &= ~_BV (1); }

void Con() { PORTB |= _BV (2); }

void Coff() { PORTB &= ~_BV (2); }

void Don() { PORTB |= _BV (3); }

void Doff() { PORTB &= ~_BV (3); }

void readMux() {
  muxChannel++;
  if (muxChannel > 15) { muxChannel = 0; }
  mux(muxChannel);

  int pot;

  pot = analogRead(A0) >> 1;
  if ((pot > potLast[muxChannel] + range) || (pot < potLast[muxChannel] - range)) {
    potLast[muxChannel] = pot;
    movedPot(muxChannel, pot >> 1, 0);
  }

  pot = analogRead(A2) >> 1;
  if ((pot > potLast[muxChannel + 16] + range) || (pot < potLast[muxChannel + 16] - range)) {
    potLast[muxChannel + 16] = pot;
    movedPot(muxChannel + 16, pot >> 1, 0);
  }

  pot = analogRead(A3) >> 1;
  if ((pot > potLast[muxChannel + 32] + range) || (pot < potLast[muxChannel + 32] - range)) {
    potLast[muxChannel + 32] = pot;
    movedPot(muxChannel + 32, pot >> 1, 0);
  }

  if (muxChannel == 15) {
    // ADC7 (pin 30) is hard-wired to pot 48
    pot = analogRead(A7) >> 1;
    if ((pot > potLast[48] + range) || (pot < potLast[48] - range)) {
      potLast[48] = pot;
      movedPot(48, pot >> 1, 0);
    }
  }

  //butts
  pot = digitalRead(A1);
  if (pot != buttLast[muxChannel]) {
    buttLast[muxChannel] = pot;
    buttChanged(static_cast<Button>(muxChannel), buttLast[muxChannel]);
  }

  if (muxChannel == 15) {
    // PCINT22 (pin 25) is hard-wired to Retrig
    pot = digitalRead(22);
    if (pot != buttLast[kButtonRetrig]) {
      buttLast[kButtonRetrig] = pot;
      buttChanged(kButtonRetrig, buttLast[kButtonRetrig]);
    }
  }

}

void mux(byte number) {
  switch (number) {
    case 0:
      Aoff();
      Boff();
      Coff();
      Doff();

      break;

    case 1:
      Aoff();
      Boff();
      Coff();
      Don();

      break;

    case 2:
      Aoff();
      Boff();
      Con();
      Doff();

      break;

    case 3:
      Aoff();
      Boff();
      Con();
      Don();

      break;

    case 4:
      Aoff();
      Bon();
      Coff();
      Doff();

      break;

    case 5:
      Aoff();
      Bon();
      Coff();
      Don();

      break;

    case 6:
      Aoff();
      Bon();
      Con();
      Doff();

      break;

    case 7:
      Aoff();
      Bon();
      Con();
      Don();

      break;

    case 8:
      Aon();
      Boff();
      Coff();
      Doff();

      break;

    case 9:
      Aon();
      Boff();
      Coff();
      Don();

      break;

    case 10:
      Aon();
      Boff();
      Con();
      Doff();

      break;

    case 11:
      Aon();
      Boff();
      Con();
      Don();

      break;

    case 12:
      Aon();
      Bon();
      Coff();
      Doff();

      break;

    case 13:
      Aon();
      Bon();
      Coff();
      Don();
      break;

    case 14:
      Aon();
      Bon();
      Con();
      Doff();
      break;

    case 15:
      Aon();
      Bon();
      Con();
      Don();
      break;
  }
}
