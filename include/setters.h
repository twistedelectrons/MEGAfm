#ifndef MEGAFM_CMAKE_SETTERS_H
#define MEGAFM_CMAKE_SETTERS_H

void setThru();
void setLFO1Clock();
void setLFO2Clock();
void setLFO3Clock();
void setVibratoClock();
void setArpClock();
void setArpStep(byte step, byte value);
void setFatSpreadMode();
void setLFO1Vel();
void setLFO2Mod();
void setLFO3Aftertouch();
void setIgnoreVolume();
void setMPEMode();
void setPickupMode();
void setStereoCh3();
void setFatMode();
void setNotePriority();
void setBrightness(byte brightness);
void setOperatorEnvelopeMode(byte op, kEnvelopeMode mode);
kEnvelopeMode getOperatorEnvelopeMode(byte op);
void setLFOShape(byte lfo, byte value);

#endif // MEGAFM_CMAKE_SETTERS_H