


byte bankTp;

uint16_t index;

void loadZero(){
//the default preset when reset is pressed or blank/invalid preset slot
  bankTp=bank;bank=0;presetTp=preset;preset=0;loadPreset();preset=presetTp;bank=bankTp;
}

void panel(){
  for(int i=0;i<64;i++){
    potLast[i]=1000;
  }

  muxChannel=16;
digit(0,14);
digit(1,17);
delay(500);
}

void setIndex(){

  index=(preset*79)+bankOffsets[bank];
}

void loadPreset(){

for(int i=0;i<49;i++){pickup[i]=1;}

Serial.end();


stopTimer();

 byte temp;byte offset;

  setIndex();

//detach all LFO
  for(int i=0;i<51;i++){
    linked[0][i]=0;
    linked[1][i]=0;
    linked[2][i]=0;
  }

//clear all fmBase
for(int i=0;i<51;i++){fmBase[i]=0;}

//load operators
for(int i=0;i<4;i++){
temp=getByte();
bitWrite(fmBase[0+offset],0,bitRead(temp,4));//detune
bitWrite(fmBase[0+offset],1,bitRead(temp,5));
bitWrite(fmBase[0+offset],2,bitRead(temp,6));
bitWrite(fmBase[1+offset],0,bitRead(temp,0));//mult
bitWrite(fmBase[1+offset],1,bitRead(temp,1));
bitWrite(fmBase[1+offset],2,bitRead(temp,2));
bitWrite(fmBase[1+offset],3,bitRead(temp,3));
fmBase[2+offset]=getByte();//total level
temp=getByte();
bitWrite(fmBase[3+offset],0,bitRead(temp,6));//rate scale
bitWrite(fmBase[3+offset],1,bitRead(temp,7));
bitWrite(fmBase[4+offset],0,bitRead(temp,0));//AR
bitWrite(fmBase[4+offset],1,bitRead(temp,1));
bitWrite(fmBase[4+offset],2,bitRead(temp,2));
bitWrite(fmBase[4+offset],3,bitRead(temp,3));
bitWrite(fmBase[4+offset],4,bitRead(temp,4));
temp=getByte();
//if(i==0){invertedSquare[0]=bitRead(temp,5);invertedSquare[1]=bitRead(temp,6);invertedSquare[2]=bitRead(temp,7);}
//if(i==1){invertedSaw[0]=bitRead(temp,5);invertedSaw[1]=bitRead(temp,6);invertedSaw[2]=bitRead(temp,7);}
bitWrite(fmBase[5+offset],0,bitRead(temp,0));//D1R
bitWrite(fmBase[5+offset],1,bitRead(temp,1));
bitWrite(fmBase[5+offset],2,bitRead(temp,2));
bitWrite(fmBase[5+offset],3,bitRead(temp,3));
bitWrite(fmBase[5+offset],4,bitRead(temp,4));
temp=getByte();
bitWrite(fmBase[6+offset],0,bitRead(temp,0));//D2R
bitWrite(fmBase[6+offset],1,bitRead(temp,1));
bitWrite(fmBase[6+offset],2,bitRead(temp,2));
bitWrite(fmBase[6+offset],3,bitRead(temp,3));
bitWrite(fmBase[6+offset],4,bitRead(temp,4));
temp=getByte();
bitWrite(fmBase[7+offset],0,bitRead(temp,4));//Sustain
bitWrite(fmBase[7+offset],1,bitRead(temp,5));//
bitWrite(fmBase[7+offset],2,bitRead(temp,6));//
bitWrite(fmBase[7+offset],3,bitRead(temp,7));//
bitWrite(fmBase[8+offset],0,bitRead(temp,0));//release
bitWrite(fmBase[8+offset],1,bitRead(temp,1));//
bitWrite(fmBase[8+offset],2,bitRead(temp,2));//
bitWrite(fmBase[8+offset],3,bitRead(temp,3));//
offset+=9;

}

//load the others
for(int i=36;i<51;i++){
fmBase[i]=getByte();
}

//boost all to 8bit
for (int i=0;i<44;i++){
fmBase[i]=fmBase[i]<<fmShifts[i];
}








voiceMode=arpMode=lfoShape[0]=lfoShape[1]=lfoShape[2]=0;
temp=getByte();
bitWrite(voiceMode,0,bitRead(temp,0));
bitWrite(voiceMode,1,bitRead(temp,1));
bitWrite(voiceMode,2,bitRead(temp,2));
bitWrite(arpMode,0,bitRead(temp,3));
bitWrite(arpMode,1,bitRead(temp,4));
bitWrite(arpMode,2,bitRead(temp,5));
bitWrite(lfoShape[0],0,bitRead(temp,6));
bitWrite(lfoShape[0],1,bitRead(temp,7));

temp=getByte();
bitWrite(lfoShape[1],0,bitRead(temp,0));
bitWrite(lfoShape[1],1,bitRead(temp,1));
bitWrite(lfoShape[2],0,bitRead(temp,2));
bitWrite(lfoShape[2],1,bitRead(temp,3));
retrig[0]=bitRead(temp,4);
retrig[1]=bitRead(temp,5);
retrig[2]=bitRead(temp,6);
looping[0]=bitRead(temp,7);


temp=getByte();
looping[1]=bitRead(temp,0);
looping[2]=bitRead(temp,1);
linked[0][0]=bitRead(temp,2);
linked[0][1]=bitRead(temp,3);
linked[0][2]=bitRead(temp,4);
linked[0][4]=bitRead(temp,5);
linked[0][5]=bitRead(temp,6);
linked[0][6]=bitRead(temp,7);
temp=getByte();
linked[0][7]=bitRead(temp,0);
linked[0][8]=bitRead(temp,1);
linked[0][9]=bitRead(temp,2);
linked[0][10]=bitRead(temp,3);
linked[0][11]=bitRead(temp,4);
linked[0][13]=bitRead(temp,5);
linked[0][14]=bitRead(temp,6);
linked[0][15]=bitRead(temp,7);
temp=getByte();
linked[0][16]=bitRead(temp,0);
linked[0][17]=bitRead(temp,1);
linked[0][18]=bitRead(temp,2);
linked[0][19]=bitRead(temp,3);
linked[0][20]=bitRead(temp,4);
linked[0][22]=bitRead(temp,5);
linked[0][23]=bitRead(temp,6);
linked[0][24]=bitRead(temp,7);
temp=getByte();
linked[0][25]=bitRead(temp,0);
linked[0][26]=bitRead(temp,1);
linked[0][27]=bitRead(temp,2);
linked[0][28]=bitRead(temp,3);
linked[0][29]=bitRead(temp,4);
linked[0][31]=bitRead(temp,5);
linked[0][32]=bitRead(temp,6);
linked[0][33]=bitRead(temp,7);
temp=getByte();
linked[0][34]=bitRead(temp,0);
linked[0][35]=bitRead(temp,1);
linked[0][36]=bitRead(temp,2);
linked[0][37]=bitRead(temp,3);
linked[0][38]=bitRead(temp,4);
linked[0][39]=bitRead(temp,5);
linked[0][40]=bitRead(temp,6);
linked[0][41]=bitRead(temp,7);
temp=getByte();
linked[0][42]=bitRead(temp,0);
linked[0][43]=bitRead(temp,1);
linked[0][44]=bitRead(temp,2);
linked[0][45]=bitRead(temp,3);
linked[0][46]=bitRead(temp,4);
linked[0][47]=bitRead(temp,5);
linked[0][48]=bitRead(temp,6);
linked[0][49]=bitRead(temp,7);
temp=getByte();
linked[0][50]=bitRead(temp,0);
linked[1][0]=bitRead(temp,1);
linked[1][1]=bitRead(temp,2);
linked[1][2]=bitRead(temp,3);
linked[1][4]=bitRead(temp,4);
linked[1][5]=bitRead(temp,5);
linked[1][6]=bitRead(temp,6);
linked[1][7]=bitRead(temp,7);
temp=getByte();
linked[1][8]=bitRead(temp,0);
linked[1][9]=bitRead(temp,1);
linked[1][10]=bitRead(temp,2);
linked[1][11]=bitRead(temp,3);
linked[1][13]=bitRead(temp,4);
linked[1][14]=bitRead(temp,5);
linked[1][15]=bitRead(temp,6);
linked[1][16]=bitRead(temp,7);
temp=getByte();
linked[1][17]=bitRead(temp,0);
linked[1][18]=bitRead(temp,1);
linked[1][19]=bitRead(temp,2);
linked[1][20]=bitRead(temp,3);
linked[1][22]=bitRead(temp,4);
linked[1][23]=bitRead(temp,5);
linked[1][24]=bitRead(temp,6);
linked[1][25]=bitRead(temp,7);
temp=getByte();
linked[1][26]=bitRead(temp,0);
linked[1][27]=bitRead(temp,1);
linked[1][28]=bitRead(temp,2);
linked[1][29]=bitRead(temp,3);
linked[1][31]=bitRead(temp,4);
linked[1][32]=bitRead(temp,5);
linked[1][33]=bitRead(temp,6);
linked[1][34]=bitRead(temp,7);
temp=getByte();
linked[1][35]=bitRead(temp,0);
linked[1][36]=bitRead(temp,1);
linked[1][37]=bitRead(temp,2);
linked[1][38]=bitRead(temp,3);
linked[1][39]=bitRead(temp,4);
linked[1][40]=bitRead(temp,5);
linked[1][41]=bitRead(temp,6);
linked[1][42]=bitRead(temp,7);
temp=getByte();
linked[1][43]=bitRead(temp,0);
linked[1][44]=bitRead(temp,1);
linked[1][45]=bitRead(temp,2);
linked[1][46]=bitRead(temp,3);
linked[1][47]=bitRead(temp,4);
linked[1][48]=bitRead(temp,5);
linked[1][49]=bitRead(temp,6);
linked[1][50]=bitRead(temp,7);
temp=getByte();
linked[2][0]=bitRead(temp,0);
linked[2][1]=bitRead(temp,1);
linked[2][2]=bitRead(temp,2);
linked[2][4]=bitRead(temp,3);
linked[2][5]=bitRead(temp,4);
linked[2][6]=bitRead(temp,5);
linked[2][7]=bitRead(temp,6);
linked[2][8]=bitRead(temp,7);
temp=getByte();
linked[2][9]=bitRead(temp,0);
linked[2][10]=bitRead(temp,1);
linked[2][11]=bitRead(temp,2);
linked[2][13]=bitRead(temp,3);
linked[2][14]=bitRead(temp,4);
linked[2][15]=bitRead(temp,5);
linked[2][16]=bitRead(temp,6);
linked[2][17]=bitRead(temp,7);
temp=getByte();
linked[2][18]=bitRead(temp,0);
linked[2][19]=bitRead(temp,1);
linked[2][20]=bitRead(temp,2);
linked[2][22]=bitRead(temp,3);
linked[2][23]=bitRead(temp,4);
linked[2][24]=bitRead(temp,5);
linked[2][25]=bitRead(temp,6);
linked[2][26]=bitRead(temp,7);
temp=getByte();
linked[2][27]=bitRead(temp,0);
linked[2][28]=bitRead(temp,1);
linked[2][29]=bitRead(temp,2);
linked[2][31]=bitRead(temp,3);
linked[2][32]=bitRead(temp,4);
linked[2][33]=bitRead(temp,5);
linked[2][34]=bitRead(temp,6);
linked[2][35]=bitRead(temp,7);
temp=getByte();
linked[2][36]=bitRead(temp,0);
linked[2][37]=bitRead(temp,1);
linked[2][38]=bitRead(temp,2);
linked[2][39]=bitRead(temp,3);
linked[2][40]=bitRead(temp,4);
linked[2][41]=bitRead(temp,5);
linked[2][42]=bitRead(temp,6);
linked[2][43]=bitRead(temp,7);
temp=getByte();
linked[2][44]=bitRead(temp,0);
linked[2][45]=bitRead(temp,1);
linked[2][46]=bitRead(temp,2);
linked[2][47]=bitRead(temp,3);
linked[2][48]=bitRead(temp,4);
linked[2][49]=bitRead(temp,5);
linked[2][50]=bitRead(temp,6);


//54 bytes used so far

for(int i=0;i<16;i++){
  seq[i]=getByte();
}

 temp=getByte();

 seqLength=0;
 bitWrite(seqLength,0,bitRead(temp,0));
 bitWrite(seqLength,1,bitRead(temp,1));
 bitWrite(seqLength,2,bitRead(temp,2));
 bitWrite(seqLength,3,bitRead(temp,3));

 bitWrite(glide,0,bitRead(temp,4));
 bitWrite(glide,1,bitRead(temp,5));
 bitWrite(glide,2,bitRead(temp,6));
 bitWrite(glide,3,bitRead(temp,7));



//75 bytes used so far


fine=getByte();updateFine();


vol=getByte();

if((lastVol!=vol)&&(!ignoreVolume)){changeVol=true;lastVol=vol;volumeCounter=10;}



if((arpMode)&&(!mpe)){ledSet(23,1);}else{ledSet(23,0);}
 //



//}

ledNumberTimeOut=200;


//load default seq if it's empty
if(seq[0]==255){
  for(int i=0;i<16;i++){seq[i]=defaultSeq[i];}
}

if((vol==0)||(vol>128)){loadZero();}

resetVoices();
vibPitch=0;
clearLfoLeds();

showVoiceMode();

Serial.begin(31250);

fmResetValues();

dumpPreset();

startTimer();

//make sure fmBaseLast != fmBase (to reset the engine)
for(int i=0;i<51;i++){

fmData[i]=fmBase[i];

  if(fmBaseLast[i]==fmBase[i])fmBaseLast[i]++;
  if(fmDataLast[i]==fmData[i])fmDataLast[i]++;



}

//setup PP






}


void savePreset(){



stopTimer();

 byte temp;byte offset;

setIndex();


//shift all from 8bit
for (int i=0;i<44;i++){fmBase[i]=fmBase[i]>>fmShifts[i];}

//store operators
for(int i=0;i<4;i++){
  temp=0;
bitWrite(temp,4,bitRead(fmBase[0+offset],0));//detune
bitWrite(temp,5,bitRead(fmBase[0+offset],1));//
bitWrite(temp,6,bitRead(fmBase[0+offset],2));//
bitWrite(temp,0,bitRead(fmBase[1+offset],0));//mult
bitWrite(temp,1,bitRead(fmBase[1+offset],1));//
bitWrite(temp,2,bitRead(fmBase[1+offset],2));//
bitWrite(temp,3,bitRead(fmBase[1+offset],3));//
store(temp);

store(fmBase[2+offset]);;//total level

temp=0;
bitWrite(temp,6,bitRead(fmBase[3+offset],0));//rate scale
bitWrite(temp,7,bitRead(fmBase[3+offset],1));//rate scale
bitWrite(temp,0,bitRead(fmBase[4+offset],0));//AR
bitWrite(temp,1,bitRead(fmBase[4+offset],1));//
bitWrite(temp,2,bitRead(fmBase[4+offset],2));//
bitWrite(temp,3,bitRead(fmBase[4+offset],3));//
bitWrite(temp,4,bitRead(fmBase[4+offset],4));//
store(temp);

temp=0;
//if(i==0){bitWrite(temp,5,invertedSquare[0]);bitWrite(temp,6,invertedSquare[1]);bitWrite(temp,7,invertedSquare[2]);}
//if(i==1){bitWrite(temp,5,invertedSaw[0]);bitWrite(temp,6,invertedSaw[1]);bitWrite(temp,7,invertedSaw[2]);}
bitWrite(temp,0,bitRead(fmBase[5+offset],0));//D1R
bitWrite(temp,1,bitRead(fmBase[5+offset],1));
bitWrite(temp,2,bitRead(fmBase[5+offset],2));
bitWrite(temp,3,bitRead(fmBase[5+offset],3));
bitWrite(temp,4,bitRead(fmBase[5+offset],4));
store(temp);

temp=0;
bitWrite(temp,0,bitRead(fmBase[6+offset],0));//D2R
bitWrite(temp,1,bitRead(fmBase[6+offset],1));
bitWrite(temp,2,bitRead(fmBase[6+offset],2));
bitWrite(temp,3,bitRead(fmBase[6+offset],3));
bitWrite(temp,4,bitRead(fmBase[6+offset],4));
store(temp);

temp=0;
bitWrite(temp,4,bitRead(fmBase[7+offset],0));//sustain
bitWrite(temp,5,bitRead(fmBase[7+offset],1));
bitWrite(temp,6,bitRead(fmBase[7+offset],2));
bitWrite(temp,7,bitRead(fmBase[7+offset],3));
bitWrite(temp,0,bitRead(fmBase[8+offset],0));//release
bitWrite(temp,1,bitRead(fmBase[8+offset],1));
bitWrite(temp,2,bitRead(fmBase[8+offset],2));
bitWrite(temp,3,bitRead(fmBase[8+offset],3));
store(temp);

offset+=9;

}


//save the others
for(int i=36;i<51;i++){
  store(fmBase[i]);

}

//shift all back to 8bit
for (int i=0;i<44;i++){
  fmBase[i]=fmBase[i]<<fmShifts[i];
}
  bitWrite(temp,0,bitRead(voiceMode,0));
  bitWrite(temp,1,bitRead(voiceMode,1));
  bitWrite(temp,2,bitRead(voiceMode,2));
  bitWrite(temp,3,bitRead(arpMode,0));
  bitWrite(temp,4,bitRead(arpMode,1));
  bitWrite(temp,5,bitRead(arpMode,2));
  bitWrite(temp,6,bitRead(lfoShape[0],0));
  bitWrite(temp,7,bitRead(lfoShape[0],1));
  store(temp);

  bitWrite(temp,0,bitRead(lfoShape[1],0));
  bitWrite(temp,1,bitRead(lfoShape[1],1));
  bitWrite(temp,2,bitRead(lfoShape[2],0));
  bitWrite(temp,3,bitRead(lfoShape[2],1));
  bitWrite(temp,4,retrig[0]);
  bitWrite(temp,5,retrig[1]);
  bitWrite(temp,6,retrig[2]);
  bitWrite(temp,7,looping[0]);
  store(temp);

  bitWrite(temp,0,looping[1]);
  bitWrite(temp,1,looping[2]);
  bitWrite(temp,2,linked[0][0]);
bitWrite(temp,3,linked[0][1]);
bitWrite(temp,4,linked[0][2]);
bitWrite(temp,5,linked[0][4]);
bitWrite(temp,6,linked[0][5]);
bitWrite(temp,7,linked[0][6]);
store(temp);
bitWrite(temp,0,linked[0][7]);
bitWrite(temp,1,linked[0][8]);
bitWrite(temp,2,linked[0][9]);
bitWrite(temp,3,linked[0][10]);
bitWrite(temp,4,linked[0][11]);
bitWrite(temp,5,linked[0][13]);
bitWrite(temp,6,linked[0][14]);
bitWrite(temp,7,linked[0][15]);
store(temp);
bitWrite(temp,0,linked[0][16]);
bitWrite(temp,1,linked[0][17]);
bitWrite(temp,2,linked[0][18]);
bitWrite(temp,3,linked[0][19]);
bitWrite(temp,4,linked[0][20]);
bitWrite(temp,5,linked[0][22]);
bitWrite(temp,6,linked[0][23]);
bitWrite(temp,7,linked[0][24]);
store(temp);
bitWrite(temp,0,linked[0][25]);
bitWrite(temp,1,linked[0][26]);
bitWrite(temp,2,linked[0][27]);
bitWrite(temp,3,linked[0][28]);
bitWrite(temp,4,linked[0][29]);
bitWrite(temp,5,linked[0][31]);
bitWrite(temp,6,linked[0][32]);
bitWrite(temp,7,linked[0][33]);
store(temp);
bitWrite(temp,0,linked[0][34]);
bitWrite(temp,1,linked[0][35]);
bitWrite(temp,2,linked[0][36]);
bitWrite(temp,3,linked[0][37]);
bitWrite(temp,4,linked[0][38]);
bitWrite(temp,5,linked[0][39]);
bitWrite(temp,6,linked[0][40]);
bitWrite(temp,7,linked[0][41]);
store(temp);
bitWrite(temp,0,linked[0][42]);
bitWrite(temp,1,linked[0][43]);
bitWrite(temp,2,linked[0][44]);
bitWrite(temp,3,linked[0][45]);
bitWrite(temp,4,linked[0][46]);
bitWrite(temp,5,linked[0][47]);
bitWrite(temp,6,linked[0][48]);
bitWrite(temp,7,linked[0][49]);
store(temp);
bitWrite(temp,0,linked[0][50]);
bitWrite(temp,1,linked[1][0]);
bitWrite(temp,2,linked[1][1]);
bitWrite(temp,3,linked[1][2]);
bitWrite(temp,4,linked[1][4]);
bitWrite(temp,5,linked[1][5]);
bitWrite(temp,6,linked[1][6]);
bitWrite(temp,7,linked[1][7]);
store(temp);
bitWrite(temp,0,linked[1][8]);
bitWrite(temp,1,linked[1][9]);
bitWrite(temp,2,linked[1][10]);
bitWrite(temp,3,linked[1][11]);
bitWrite(temp,4,linked[1][13]);
bitWrite(temp,5,linked[1][14]);
bitWrite(temp,6,linked[1][15]);
bitWrite(temp,7,linked[1][16]);
store(temp);
bitWrite(temp,0,linked[1][17]);
bitWrite(temp,1,linked[1][18]);
bitWrite(temp,2,linked[1][19]);
bitWrite(temp,3,linked[1][20]);
bitWrite(temp,4,linked[1][22]);
bitWrite(temp,5,linked[1][23]);
bitWrite(temp,6,linked[1][24]);
bitWrite(temp,7,linked[1][25]);
store(temp);
bitWrite(temp,0,linked[1][26]);
bitWrite(temp,1,linked[1][27]);
bitWrite(temp,2,linked[1][28]);
bitWrite(temp,3,linked[1][29]);
bitWrite(temp,4,linked[1][31]);
bitWrite(temp,5,linked[1][32]);
bitWrite(temp,6,linked[1][33]);
bitWrite(temp,7,linked[1][34]);
store(temp);
bitWrite(temp,0,linked[1][35]);
bitWrite(temp,1,linked[1][36]);
bitWrite(temp,2,linked[1][37]);
bitWrite(temp,3,linked[1][38]);
bitWrite(temp,4,linked[1][39]);
bitWrite(temp,5,linked[1][40]);
bitWrite(temp,6,linked[1][41]);
bitWrite(temp,7,linked[1][42]);
store(temp);
bitWrite(temp,0,linked[1][43]);
bitWrite(temp,1,linked[1][44]);
bitWrite(temp,2,linked[1][45]);
bitWrite(temp,3,linked[1][46]);
bitWrite(temp,4,linked[1][47]);
bitWrite(temp,5,linked[1][48]);
bitWrite(temp,6,linked[1][49]);
bitWrite(temp,7,linked[1][50]);
store(temp);
bitWrite(temp,0,linked[2][0]);
bitWrite(temp,1,linked[2][1]);
bitWrite(temp,2,linked[2][2]);
bitWrite(temp,3,linked[2][4]);
bitWrite(temp,4,linked[2][5]);
bitWrite(temp,5,linked[2][6]);
bitWrite(temp,6,linked[2][7]);
bitWrite(temp,7,linked[2][8]);
store(temp);
bitWrite(temp,0,linked[2][9]);
bitWrite(temp,1,linked[2][10]);
bitWrite(temp,2,linked[2][11]);
bitWrite(temp,3,linked[2][13]);
bitWrite(temp,4,linked[2][14]);
bitWrite(temp,5,linked[2][15]);
bitWrite(temp,6,linked[2][16]);
bitWrite(temp,7,linked[2][17]);
store(temp);
bitWrite(temp,0,linked[2][18]);
bitWrite(temp,1,linked[2][19]);
bitWrite(temp,2,linked[2][20]);
bitWrite(temp,3,linked[2][22]);
bitWrite(temp,4,linked[2][23]);
bitWrite(temp,5,linked[2][24]);
bitWrite(temp,6,linked[2][25]);
bitWrite(temp,7,linked[2][26]);
store(temp);
bitWrite(temp,0,linked[2][27]);
bitWrite(temp,1,linked[2][28]);
bitWrite(temp,2,linked[2][29]);
bitWrite(temp,3,linked[2][31]);
bitWrite(temp,4,linked[2][32]);
bitWrite(temp,5,linked[2][33]);
bitWrite(temp,6,linked[2][34]);
bitWrite(temp,7,linked[2][35]);
store(temp);
bitWrite(temp,0,linked[2][36]);
bitWrite(temp,1,linked[2][37]);
bitWrite(temp,2,linked[2][38]);
bitWrite(temp,3,linked[2][39]);
bitWrite(temp,4,linked[2][40]);
bitWrite(temp,5,linked[2][41]);
bitWrite(temp,6,linked[2][42]);
bitWrite(temp,7,linked[2][43]);
store(temp);
bitWrite(temp,0,linked[2][44]);
bitWrite(temp,1,linked[2][45]);
bitWrite(temp,2,linked[2][46]);
bitWrite(temp,3,linked[2][47]);
bitWrite(temp,4,linked[2][48]);
bitWrite(temp,5,linked[2][49]);
bitWrite(temp,6,linked[2][50]);
store(temp);

 for(int i=0;i<16;i++){store(seq[i]);}

temp=0;
bitWrite(temp,0,bitRead(seqLength,0));
bitWrite(temp,1,bitRead(seqLength,1));
bitWrite(temp,2,bitRead(seqLength,2));
bitWrite(temp,3,bitRead(seqLength,3));

bitWrite(temp,4,bitRead(glide,0));
bitWrite(temp,5,bitRead(glide,1));
bitWrite(temp,6,bitRead(glide,2));
bitWrite(temp,7,bitRead(glide,3));

store(temp);

 store(fine);

 store(vol);
  saved=true;

startTimer();



}


void shuffle(){
shuffled=true;

  for(int i=0;i<51;i++){fmBase[i]=fmData[i]=random(255);fmDataLast[i]=32;}
voiceMode=random(4);
for(int i=0;i<51;i++){
  linked[0][i]=random(20)/18;
  linked[1][i]=random(20)/18;
  linked[2][i]=random(20)/18;

}

for(int i=0;i<16;i++){
seq[i]=random(25);
}
if((random(2))&&(random(2))&&(random(2))){arpMode=random(6);}else{arpMode=0;}
showVoiceMode();


}


byte getByte(){



  byte data;

  if(index<3950){int indexInt=int(index);data=EEPROM.read(indexInt);} // internal eeprom
  else{data=eRead(index-3950);}//overflow to external eeprom

index++;

return(data);
  }

void store(byte input){


  if(index<3950){int indexInt=int(index);EEPROM.write(indexInt,input);}// internal eeprom
else{eWrite(index-3950,input);}//overflow to external eeprom

index++;

}



#define DEVICEADDRESS_READ   ((uint8_t)(0b01010000))
uint8_t deviceadress= DEVICEADDRESS_READ;

void eWrite( uint16_t theMemoryAddress, uint8_t u8Byte)
{
 Wire.beginTransmission(deviceadress);
 Wire.write((uint8_t) (((theMemoryAddress >> 8  )  ) & 0xFF) );
 Wire.write((uint8_t) (((theMemoryAddress ) ) & 0xFF) );
 Wire.write((uint8_t) u8Byte);
 Wire.endTransmission();
 delay(10);
}

uint8_t eRead(uint16_t theMemoryAddress)
{
  uint8_t u8retVal = 0;
  Wire.beginTransmission(DEVICEADDRESS_READ);
  Wire.write((uint8_t) (((theMemoryAddress >> 8 ) ) & 0xFF) );
  Wire.write((uint8_t) (((theMemoryAddress ) ) & 0xFF) );
  Wire.endTransmission();
  //delay(1);
  Wire.requestFrom(DEVICEADDRESS_READ, (uint8_t)1);

  while(!Wire.available()){}
  if (Wire.available()) u8retVal = Wire.read();
  return u8retVal ;
}


void storeInvert(){
  byte invertTemp;
bitWrite(invertTemp,0,invertedSaw[0]);
bitWrite(invertTemp,1,invertedSaw[1]);
bitWrite(invertTemp,2,invertedSaw[2]);
bitWrite(invertTemp,3,invertedSquare[0]);
bitWrite(invertTemp,4,invertedSquare[1]);
bitWrite(invertTemp,5,invertedSquare[2]);
bitWrite(invertTemp,6,stereoCh3);
EEPROM.update(3966,invertTemp);

}
