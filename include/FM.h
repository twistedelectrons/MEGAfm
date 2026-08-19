#ifndef MEGAFM_CMAKE_FM_H
#define MEGAFM_CMAKE_FM_H
/**
 * All the parameters are initially treated and modulated as bytes (0-255).
 * Most registers of the YM2612 are less than 8 bits wide, so fmShifts is used
 * to shift them before pushing to the FM chip.
 */
const byte fmShifts[] = {
    // Detune, Multiply, Total Level, Rate Scaling, Attack, Decay, Sustain Rate, Sustain Level, Release Rate
    5, 4, 1, 6, 3, 3, 3, 4, 4, // 0-8
    5, 4, 1, 6, 3, 3, 3, 4, 4, // 9-17
    5, 4, 1, 6, 3, 3, 3, 4, 4, // 18-26
    5, 4, 1, 6, 3, 3, 3, 4, 4, // 27-35
    // LFO 1 clock speed=36 (0-11), NA, LFO 2 cs=38, NA, LFO 3 cs=40, NA, algo=42, feedback=43
    0, 0, 0, 0, 0, 0, 5, 5}; // 36-43

const byte detuneFix[] = {7, 6, 5, 4, 0, 1, 2, 3}; // added 0 before 1?!
void setSSEG(byte op, bool index, bool value);
void updateFine();
void updateFMifNecessary(byte number);
void fmResetValues();
void fmUpdate();
void fm(byte number, byte data);
void fmMpe(byte channel, byte number, byte data);
void op(byte number);
void setupFM();
void updateSSEG(byte op);

#endif // MEGAFM_CMAKE_FM_H
