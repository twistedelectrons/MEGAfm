//
// Created by Matt Montag on 1/22/22.
//

#ifndef MEGAFM_CMAKE_MUX_H
#define MEGAFM_CMAKE_MUX_H

/**
 * Multiplexed data line reader. Reads one of 16 channels (in global variable muxChannel),
 * then fires movedPot or buttChanged callback.
 * TODO(montag): improve encapsulation
 */
void readMux();
/**
 * Sets the 4 multiplexer bits to the right value in order to read a given channel.
 * @param number The channel to select.
 */
void mux(byte number);

#endif //MEGAFM_CMAKE_MUX_H
