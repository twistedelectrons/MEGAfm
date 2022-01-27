//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_PITCHENGINE_H
#define MEGAFM_CMAKE_PITCHENGINE_H

float noteToFrequency(int note);
float noteToFrequencyMpe(int note,int channel);
void updatePitch();
void setNote(uint8_t channel, uint8_t note);
void skipGlide(uint8_t channel);
void updateGlideIncrements();
float calculateIncrement(float present,float future);
void setFat(int number) ;

#endif //MEGAFM_CMAKE_PITCHENGINE_H
