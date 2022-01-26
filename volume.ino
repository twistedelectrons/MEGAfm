


void digitalPotWrite(byte address, byte value) {

  // take the SS pin low to select the chip:
  digitalWrite(21,LOW);
  //  send in the address and value via SPI:
  shiftOut(18, potClock, MSBFIRST, address);
  shiftOut(18, potClock, MSBFIRST, value);
 
  // take the SS pin high to de-select the chip:
  digitalWrite(21,HIGH); 

}
