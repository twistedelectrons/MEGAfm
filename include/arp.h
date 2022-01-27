//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_ARP_H
#define MEGAFM_CMAKE_ARP_H

void clearNotes();
byte getHighestArp();
void arpReset();
void nextArpStep();
void arpTick();
void arpFire();

#endif //MEGAFM_CMAKE_ARP_H
