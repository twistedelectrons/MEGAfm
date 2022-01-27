


/*
  0
5   1
  6
4   2
  3   7
  

   
  
  1-a1
  2-a2
  3-a3
  4-a4
  5-a5
  6-a6
  7-a7
  8-a8

  9-poly12
  10-wide6
  11-dual ch3
  12-unison

  
  13-chain1
  14-chain2
  15-chain3
  
  16-square
  17-tri?
  18-saw1
  19-noise
  
  20-retrig
  21-loop

  22-rec

  23-am1
  24-am2
  25-am3
  26-am4

  27-fm1
  28-fm2
  29-fm3
  30-fm4
 
*/



void updateLedNumber(){

int theNumber=-1;

  for(byte ledNumberIndex=0;ledNumberIndex<49;ledNumberIndex++){

  switch(ledNumberIndex){

    
case 2:case 11:case 20:case 29:case 37:case 39:case 41:case 49:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>2){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>2;theNumber=(fmBaseLastNumber[ledNumberIndex]);



}break;//>>2

case 4:case 5:case 6:case 13:case 14:case 15:case 22:case 23:case 24:case 31:case 32:case 33:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>3){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>3;theNumber=(fmBaseLastNumber[ledNumberIndex]);}break;//>>3


case 7:case 35:case 8:case 16:case 17:case 25:case 26:case 34:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>4){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>4;theNumber=(fmBaseLastNumber[ledNumberIndex]);}break;//>>4

case 1:case 10:case 19:case 28:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>4){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>4;theNumber=(fmBaseLastNumber[ledNumberIndex]);if(!theNumber){theNumber=666;}}break;//mult

case 43:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>5){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>5;theNumber=(fmBaseLastNumber[ledNumberIndex]);}break;//>>5

case 42:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>5){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>5;theNumber=(1+fmBaseLastNumber[ledNumberIndex]);}break;//>>5 +1

case 47:if(fmBaseLastNumber[ledNumberIndex]!=fmBase[ledNumberIndex]>>6){fmBaseLastNumber[ledNumberIndex]=fmBase[ledNumberIndex]>>6;theNumber=(1+fmBaseLastNumber[ledNumberIndex]);}break;//>>6 +1
   
    
     
  }
  if(dontShow[ledNumberIndex]){theNumber=-1;}
}

if(!setupMode){if(preset!=presetLast){presetLast=preset;lastNumber=-1;ledNumber(preset);}}else{
if(!setupChanged){digit(0,5);
digit(1,18);
}}


if((theNumber!=-1)&&(!ledNumberTimeOut)){ledNumber(theNumber);}
}


void clearLfoLeds(){//hide the lfo leds

ledSet(16,0);
ledSet(17,0);
ledSet(18,0);
ledSet(19,0);
ledSet(20,0);
ledSet(21,0);
}


void showLfo(){
  if(!bankCounter){
ledSet(16,0);
ledSet(17,0);
ledSet(18,0);
ledSet(19,0);
    ledSet(16+lfoShape[selectedLfo],1);

  ledSet(20,retrig[selectedLfo]);
  ledSet(21,looping[selectedLfo]);
}}


void showLink(){
ledSet(13,linked[0][targetPot]);
ledSet(14,linked[1][targetPot]);
ledSet(15,linked[2][targetPot]);

  }

  
void showVoiceMode(){

  
 

if(mpe){

  ledSet(9,1);
  ledSet(10,0);
  ledSet(11,0);
  ledSet(12,0);
  
}else{

  if(voiceMode==1){ym.setStagger(0);}else{ym.setStagger(1);}
  

  if(voiceMode==2){ym.setChan3Mode(1);

  /*
  op(0);ym.setMultiply(2);
  op(1);ym.setMultiply(2);
  op(2);ym.setMultiply(2);
  op(3);ym.setMultiply(2);
  */
  }else{ym.setChan3Mode(0);

  /*
  op(0);ym.setMultiply(fmData[27]>>4);
  op(1);ym.setMultiply(fmData[32]>>4);
  op(2);ym.setMultiply(fmData[24]>>4);
  op(3);ym.setMultiply(fmData[39]>>4);
  */
  
  }
  
  ledSet(9,0);
  ledSet(10,0);
  ledSet(11,0);
  ledSet(12,0);
 ledSet(9+voiceMode,1);

resetVoices();


}}
void rightDot(){


mydisplay.setLed(0,7,7,1);


dotTimer=50;
}

void leftDot(){

mydisplay.setLed(0,7,6,1);
dotTimer=50;
}



void showSendReceive(){

ledSet(16,0);
ledSet(17,0);
ledSet(18,0);
ledSet(19,0);
ledSet(20,0);
ledSet(21,0);

if(sendReceive==1){digit(0,16);}else{digit(0,5);}
if(ab){digit(1,25);}else{digit(1,17);}

  
}
  

void ledNumber(int value){

if(value==666){//.5
  digit(0,21);
  digit(1,5);
  mydisplay.setLed(0,7,6,1);
  lastNumber=value;
  }else{

dotTimer=10;

  if(value<0){
if(value>-10){
    digit(0,20);//minus
    digit(1,-value);
     lastNumber=value;
} 
  }else{
  if(value>99)value=99;

if(lastNumber!=value){lastNumber=value;
digit(0,value/10);
digit(1,value-((value/10)*10));
}
}}}

void ledNumberForced(int value){

if(value>99)value=99;
digit(0,value/10);
digit(1,value-((value/10)*10));
}



/*
  0
5   1
  6
4   2
  3   7
  
  */


  void rightArrow(){
digit(0,29);
digit(1,29);
    
  }

  void leftArrow(){
digit(0,28);
digit(1,28);
    
  }
  
void digit(byte channel,byte number){
  switch(number){

case 29://left arrow
    mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
break;

case 28://right arrow
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);
break;
   
case 27://o
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);

    break;
    
case 26://t
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);

    break;
    
case 25://b
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);

    break;
   
case 24://all lit (plus dots)
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);

   mydisplay.setLed(0,7,7,1);
   mydisplay.setLed(0,7,6,1);

   break;
   
case 22://random
   mydisplay.setLed(0,0,6+channel,random(2));
   mydisplay.setLed(0,5,6+channel,random(2));
   mydisplay.setLed(0,1,6+channel,random(2));
   mydisplay.setLed(0,6,6+channel,random(2));
   mydisplay.setLed(0,4,6+channel,random(2));
   mydisplay.setLed(0,2,6+channel,random(2));
   mydisplay.setLed(0,3,6+channel,random(2));
   break;
   
case 21://blank
lastNumber=-1;
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   break;
   
case 20://-
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   break;

        case 23://h
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
   break;
   
   
      case 19://n
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
   break;
   
        case 18://E
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);
   break;
          case 17://A
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
  
   break; 
   
               case 16://r
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   
  
   break;
   
            case 15://d
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
   
  
   break;
   
        case 14://P
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   
  
   break;
   
       case 13://U
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
   
  
   break; 
   
    case 12://F
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   
  
   break;
          case 11://L
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);
   
  
   break; 
   
       case 10://C
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);
   
  
   break; 
   
          case 9:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
  
   break; 
   
          case 8:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
  
   break; 
   
          case 7:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
  
   break; 
          case 6:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
  
   break; 
   
          case 5:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
  
   break; 
   
          case 4:
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
  
   break; 
   
          case 3:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
  
   break; 
   
         case 2:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,1);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,1);
  
   break; 
   
     case 1:
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,0);
  
   break; 
   
   case 0:
   mydisplay.setLed(0,0,6+channel,1);
   mydisplay.setLed(0,5,6+channel,1);
   mydisplay.setLed(0,1,6+channel,1);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,1);
   mydisplay.setLed(0,2,6+channel,1);
   mydisplay.setLed(0,3,6+channel,1);
   
  
   break; 
   
     case 99://BLANK
   mydisplay.setLed(0,0,6+channel,0);
   mydisplay.setLed(0,5,6+channel,0);
   mydisplay.setLed(0,1,6+channel,0);
   mydisplay.setLed(0,6,6+channel,0);
   mydisplay.setLed(0,4,6+channel,0);
   mydisplay.setLed(0,2,6+channel,0);
   mydisplay.setLed(0,3,6+channel,0);
   
  
   break;
    
  }
  
}

void showAlgo(byte number){

for(int i=1;i<9;i++)ledSet(i,0);
ledSet(number+1,1);
  
}


void ledSet(byte number,bool value){
  
  switch(number){
  case 1: mydisplay.setLed(0,0,1,value); break;
  case 2: mydisplay.setLed(0,1,1,value); break;
  case 3: mydisplay.setLed(0,2,1,value); break;
  case 4: mydisplay.setLed(0,3,1,value); break;
  
  case 5: mydisplay.setLed(0,4,1,value); break;
  case 6: mydisplay.setLed(0,5,1,value); break;
  case 7: mydisplay.setLed(0,6,1,value); break;
  case 8: mydisplay.setLed(0,7,1,value); break;
  
  case 9: mydisplay.setLed(0,0,2,value); break;
  case 10: mydisplay.setLed(0,1,2,value); break;
  case 11: mydisplay.setLed(0,2,2,value); break;
  case 12: mydisplay.setLed(0,3,2,value); break;
  
  case 13: mydisplay.setLed(0,4,2,value); break;
  case 14: mydisplay.setLed(0,5,2,value); break;
  case 15: mydisplay.setLed(0,6,2,value); break;
  
  case 16: mydisplay.setLed(0,7,2,value); break;
  case 17: mydisplay.setLed(0,0,3,value); break;
  
  case 18: mydisplay.setLed(0,1,3,value); break;
  case 19: mydisplay.setLed(0,2,3,value); break;
  
  case 20: mydisplay.setLed(0,3,3,value); break;
  case 21: mydisplay.setLed(0,4,3,value); break;
  
  case 22: mydisplay.setLed(0,5,3,value); break;
  case 23: mydisplay.setLed(0,6,3,value); break;
  case 24: mydisplay.setLed(0,7,3,value); break;
  case 25: mydisplay.setLed(0,0,4,value); break;
  case 26: mydisplay.setLed(0,1,4,value); break;
  
  case 27: mydisplay.setLed(0,2,4,value); break;
  case 28: mydisplay.setLed(0,3,4,value); break;
  case 29: mydisplay.setLed(0,4,4,value); break;
  
  case 30: mydisplay.setLed(0,5,4,value); break;
  case 31: mydisplay.setLed(0,6,4,value); break;
  
  

  }
}
