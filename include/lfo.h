#ifndef MEGAFM_CMAKE_LFO_H
#define MEGAFM_CMAKE_LFO_H

void applyLfo();
void lfoBlink();
void lfoTick();
void lfoAdvance();
void clearLinks();
byte getRandom(byte number);
void fillRandomLfo(byte number);
void lfoLedOn();
void fillAllLfoTables();

#endif //MEGAFM_CMAKE_LFO_H
