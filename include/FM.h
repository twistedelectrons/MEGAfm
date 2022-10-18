#ifndef MEGAFM_CMAKE_FM_H
#define MEGAFM_CMAKE_FM_H
/**
 * All the parameters are initially treated and modulated as bytes (0-255).
 * Most registers of the YM2612 are less than 8 bits wide, so fmShifts is used
 * to shift them before pushing to the FM chip.
 */
const byte fmShifts[] = {5, 4, 1, 6, 3, 3, 3, 4, 4, 5, 4, 1, 6, 3, 3, 3, 4, 4, 5, 4, 1, 6,
                         3, 3, 3, 4, 4, 5, 4, 1, 6, 3, 3, 3, 4, 4, 0, 0, 0, 0, 0, 0, 5, 5};

const byte detuneFix[] = {7, 6, 5, 4, 0, 1, 2, 3}; // added 0 before 1?!

void updateFine();
void updateFMifNecessary(byte number);
void fmResetValues();
void fmUpdate();
void fm(byte number, byte data);
void op(byte number);
void setupFM();
void WriteYMData(byte data);
float noteToFrequency(uint8_t note);
void setFrequency(byte chan, float frequency);

#endif // MEGAFM_CMAKE_FM_H
