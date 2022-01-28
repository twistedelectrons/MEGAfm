#ifndef YM2612_h
#define YM2612_h

#include "Arduino.h"
#include <inttypes.h>
#include "LinkedList.h"

#define UNUSED(x) (void)(x)
#define YM_MASTER_ADDR (0x22)
#define YM_CHN_ADDR    (0x30)

#define YM_MA_LFO_E    offsetof(Master_t,LFO), 1, 3
#define YM_MA_LFO_F    offsetof(Master_t,LFO), 3, 0
#define YM_MA_CH3_M    offsetof(Master_t,CHAN3_MODE_TIMERS), 2, 6
#define YM_MA_OP_CHAN  offsetof(Master_t,OP_CHAN), 8, 0

#define YM_OP_DT1 offsetof(Channels_t,DT1_MUL), 3, 4
#define YM_OP_MUL offsetof(Channels_t,DT1_MUL), 4, 0
#define YM_OP_TL  offsetof(Channels_t,TL), 7, 0
#define YM_OP_RS  offsetof(Channels_t,RS_AR), 2, 6
#define YM_OP_AR  offsetof(Channels_t,RS_AR), 5, 0
#define YM_OP_AM  offsetof(Channels_t,AM_D1R), 1, 7
#define YM_OP_D1R offsetof(Channels_t,AM_D1R), 5, 0
#define YM_OP_D2R offsetof(Channels_t,D2R), 5, 0
#define YM_OP_D1L offsetof(Channels_t,D1L_RR), 4, 4
#define YM_OP_RR  offsetof(Channels_t,D1L_RR), 4, 0
#define YM_OP_SSG_EG offsetof(Channels_t,SSG_EG), 8, 0

#define YM_CH_FB  offsetof(Channels_t,FB_ALG), 3, 3
#define YM_CH_ALG offsetof(Channels_t,FB_ALG), 3, 0
#define YM_CH_L_R offsetof(Channels_t,L_R_AMS_FMS), 2, 6
#define YM_CH_AMS offsetof(Channels_t,L_R_AMS_FMS), 2, 4
#define YM_CH_FMS offsetof(Channels_t,L_R_AMS_FMS), 3, 0

#define mask(s) (~(~0<<s))

typedef struct{
  uint8_t LFO;
  uint8_t RESERVED0;
  uint8_t TIMERA0;
  uint8_t TIMERA1;
  uint8_t TIMERB;
  uint8_t CHAN3_MODE_TIMERS;
  uint8_t OP_CHAN;
  uint8_t RESERVED1;
  uint8_t DAC;
  uint8_t DAC_EN;
} Master_t;

typedef struct
{
  uint8_t DT1_MUL[16];
  uint8_t TL[16];
  uint8_t RS_AR[16];
  uint8_t AM_D1R[16];
  uint8_t D2R[16];
  uint8_t D1L_RR[16];
  uint8_t SSG_EG[16]; //proprietary and should be set to zero.  
  uint8_t FREQ[16];
  uint8_t FB_ALG[4];
  uint8_t L_R_AMS_FMS[4];
} Channels_t;

typedef enum {
  MONO1x6,
  POLY6x1,
  POLY3x2,
  POLY2x3,
  SPLIT_MONO1x3_MONO1x3,
  SPLIT_MONO1x3_POLY3x1,
  SPLIT_POLY3x1_MONO1x3,
  SPLIT_POLY3x1_POLY3x1
} playmode_t;


typedef struct
{
  bool on;
  uint8_t note;
  float frequency;
} voice_t;

class YM2612
{
  public:
    void setup(uint8_t ic_pin,
               uint8_t cs_pin,
               uint8_t wr_pin,
               uint8_t rd_pin,
               uint8_t a0_pin,
               uint8_t a1_pin,
               uint8_t mc_pin,
               uint8_t data0_pin,
               uint8_t data1_pin,
               uint8_t data2_pin,
               uint8_t data3_pin,
               uint8_t data4_pin,
               uint8_t data5_pin,
               uint8_t data6_pin,
               uint8_t data7_pin);



    void setAmVib(byte number,int data);
    void setPlaymode(int value);
    void selectChannel(int value) {selected_channel = value;}

    void selectOperator(int op, int value) {operators[op] = (value>0);}

    //master params
    void setLFO (int value);
    void setChan3Mode           (int value) { setMasterParameter( YM_MA_CH3_M, value);}

    //channel params
    void setFeedback            (int value) { setChannelParameter( YM_CH_FB, value);}
    void setAlgorithm           (int value) { setChannelParameter( YM_CH_ALG, value);}
    void setStereo              (int value) { setChannelParameter( YM_CH_L_R, value);}
    void setStereo              (int l, int r) { setStereo((l<<1)|r);}
    void setAMS                 (int value) { setChannelParameter( YM_CH_AMS, value);}
    void setFMS                 (int value) { setChannelParameter( YM_CH_FMS, value);}
    //operator params
    void setAmplitudeModulation (int value) { setOperatorParameter( YM_OP_AM, (value>0));}
    void setAttackRate          (int value) { setOperatorParameter( YM_OP_AR, value);}
    void setDecayRate           (int value) { setOperatorParameter( YM_OP_D1R, value);}
    void setSustainRate         (int value) { setOperatorParameter( YM_OP_D2R, value);}
    void setReleaseRate         (int value) { setOperatorParameter( YM_OP_RR, value);}
    void setTotalLevel          (int value) { setOperatorParameter( YM_OP_TL, value);}
    void setSustainLevel        (int value) { setOperatorParameter( YM_OP_D1L, value);}
    void setMultiply            (int value) { setOperatorParameter( YM_OP_MUL, value);}
    void setDetune              (int value) { setOperatorParameter( YM_OP_DT1, value);}
    void setRateScaling         (int value) { setOperatorParameter( YM_OP_RS, value);}
    void setSSG_EG              (int value) { setOperatorParameter( YM_OP_SSG_EG, value);}


    void noteOn(byte chan);
    void noteOff(byte chan);
    void pitchBend(byte channel, int bend);
    void update();
    void updatePitch();

    void updateBend(float input);void setFine(float input);
    void setFrequency3(byte op,uint8_t channel, float frequency);
    void setFrequencySingle(uint8_t channel, float frequency);
    void setStagger(bool data);

  private:
  byte chip;float fat;int amVib[4];
  float bendy=1;
  float finey=1;
  bool stag;
  bool megaChip;

    voice_t voices[6];
    uint8_t voices_order[6];
    uint8_t voices_order_index = 0;


    LinkedList<uint8_t> notes_stack0 = LinkedList<uint8_t>();
    LinkedList<uint8_t> voices_stack0 = LinkedList<uint8_t>();
    LinkedList<uint8_t> notes_stack1 = LinkedList<uint8_t>();
    LinkedList<uint8_t> voices_stack1 = LinkedList<uint8_t>();
    uint8_t splitNote = 60;
    int pitchBendValue = 0;

    playmode_t playmode;
    uint8_t selected_channel;
    bool operators[4];

    Master_t master;
    Channels_t channels[2];
    uint8_t ic_pin;
    uint8_t cs_pin;
    uint8_t wr_pin;
    uint8_t rd_pin;
    uint8_t a0_pin;
    uint8_t a1_pin;
    uint8_t mc_pin;
    uint8_t data0_pin;
    uint8_t data1_pin;
    uint8_t data2_pin;
    uint8_t data3_pin;
    uint8_t data4_pin;
    uint8_t data5_pin;
    uint8_t data6_pin;
    uint8_t data7_pin;
    uint8_t channelPart();
    uint8_t channelOffset();
    void sendData(uint8_t data);
    void setRegister(uint8_t part, uint8_t reg, uint8_t data);
    void setMasterParameter(int reg_offset, int val_size, int val_shift,int val);
    void setChannelParameter(int chan,int reg_offset, int val_size, int val_shift,int val);
    void setChannelParameter(int reg_offset, int val_size, int val_shift,int val);
    void setOperatorParameter(int reg_offset, int val_size, int val_shift,int val);
    void setOperatorParameter(int chan,int reg_offset, int val_size, int val_shift,int val);
    void setOperatorParameter(int chan,int oper, int reg_offset, int val_size, int val_shift, int val);
    void setFrequency(uint8_t channel, float frequency);
    void setDefaults();

    void setSupplementaryFrequency(uint8_t channel, uint8_t oper, float frequency);
    void keyOn(uint8_t channel);
    void keyOff(uint8_t channel);
    void sendMegaChip(byte number, byte data);
    void setMegaChip(boolean input);

    float noteToFrequency(int note);
};


#endif




/*

from http://www.smspower.org/maxim/Documents/YM2612#reg27

Part I memory map
=================

+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| D7   | D6                                 | D5  | D4                      | D3      | D2         | D1                                 | D0           |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 22H  |                                    |     |                         |         | LFO enable | LFO frequency                      |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 24H  | Timer A MSBs                       |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 25H  |                                    |     |                         |         |            |                                    | Timer A LSBs |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 26H  | Timer B                            |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 27H  | Ch3 mode                           |     | Reset B                 | Reset A | Enable B   | Enable A                           | Load B       | Load A |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 28H  | Operator                           |     |                         |         |            | Channel                            |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 29H  |                                    |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 2AH  | DAC                                |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 2BH  | DAC en                             |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
|      |                                    |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 30H+ |                                    | DT1 |                         |         | MUL        |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 40H+ |                                    | TL  |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 50H+ | RS                                 |     |                         | AR      |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 60H+ | AM                                 |     |                         | D1R     |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 70H+ |                                    |     |                         | D2R     |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 80H+ | D1L                                |     |                         |         | RR         |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| 90H+ |                                    |     |                         |         | SSG-EG     |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
|      |                                    |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| A0H+ | Frequency number LSB               |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| A4H+ |                                    |     | Block                   |         |            | Frequency Number MSB               |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| A8H+ | Ch3 supplementary frequency number |     |                         |         |            |                                    |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| ACH+ |                                    |     | Ch3 supplementary block |         |            | Ch3 supplementary frequency number |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| B0H+ |                                    |     | Feedback                |         |            | Algorithm                          |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+
| B4H+ | L                                  | R   | AMS                     |         |            | FMS                                |              |        |
+------+------------------------------------+-----+-------------------------+---------+------------+------------------------------------+--------------+--------+









MIDI Note Number to Frequency Conversion Chart
==============================================

 Note     Frequency      Note   Frequency       Note   Frequency
 C1  0    8.1757989156    12    16.3515978313    24    32.7031956626
 Db  1    8.6619572180    13    17.3239144361    25    34.6478288721
 D   2    9.1770239974    14    18.3540479948    26    36.7080959897
 Eb  3    9.7227182413    15    19.4454364826    27    38.8908729653
 E   4   10.3008611535    16    20.6017223071    28    41.2034446141
 F   5   10.9133822323    17    21.8267644646    29    43.6535289291
 Gb  6   11.5623257097    18    23.1246514195    30    46.2493028390
 G   7   12.2498573744    19    24.4997147489    31    48.9994294977
 Ab  8   12.9782717994    20    25.9565435987    32    51.9130871975
 A   9   13.7500000000    21    27.5000000000    33    55.0000000000
 Bb  10  14.5676175474    22    29.1352350949    34    58.2704701898
 B   11  15.4338531643    23    30.8677063285    35    61.7354126570

 C4  36  65.4063913251    48   130.8127826503    60   261.6255653006
 Db  37  69.2956577442    49   138.5913154884    61   277.1826309769
 D   38  73.4161919794    50   146.8323839587    62   293.6647679174
 Eb  39  77.7817459305    51   155.5634918610    63   311.1269837221
 E   40  82.4068892282    52   164.8137784564    64   329.6275569129
 F   41  87.3070578583    53   174.6141157165    65   349.2282314330
 Gb  42  92.4986056779    54   184.9972113558    66   369.9944227116
 G   43  97.9988589954    55   195.9977179909    67   391.9954359817
 Ab  44  103.8261743950   56   207.6523487900    68   415.3046975799
 A   45  110.0000000000   57   220.0000000000    69   440.0000000000
 Bb  46  116.5409403795   58   233.0818807590    70   466.1637615181
 B   47  123.4708253140   59   246.9416506281    71   493.8833012561

 C7  72  523.2511306012   84  1046.5022612024    96  2093.0045224048
 Db  73  554.3652619537   85  1108.7305239075    97  2217.4610478150
 D   74  587.3295358348   86  1174.6590716696    98  2349.3181433393
 Eb  75  622.2539674442   87  1244.5079348883    99  2489.0158697766
 E   76  659.2551138257   88  1318.5102276515   100  2637.0204553030
 F   77  698.4564628660   89  1396.9129257320   101  2793.8258514640
 Gb  78  739.9888454233   90  1479.9776908465   102  2959.9553816931
 G   79  783.9908719635   91  1567.9817439270   103  3135.9634878540
 Ab  80  830.6093951599   92  1661.2187903198   104  3322.4375806396
 A   81  880.0000000000   93  1760.0000000000   105  3520.0000000000
 Bb  82  932.3275230362   94  1864.6550460724   106  3729.3100921447
 B   83  987.7666025122   95  1975.5332050245   107  3951.0664100490

 C10 108 4186.0090448096  120  8372.0180896192
 Db  109 4434.9220956300  121  8869.8441912599
 D   110 4698.6362866785  122  9397.2725733570
 Eb  111 4978.0317395533  123  9956.0634791066
 E   112 5274.0409106059  124 10548.0818212118
 F   113 5587.6517029281  125 11175.3034058561
 Gb  114 5919.9107633862  126 11839.8215267723
 G   115 6271.9269757080  127 12543.8539514160
 Ab  116 6644.8751612791
 A   117 7040.0000000000
 Bb  118 7458.6201842894
 B   119 7902.1328200980

 
NOTES: Middle C is note #60. Frequency is in Hertz.

Here is C code to calculate an array with all of the above frequencies (ie, so that midi[0], which is midi note #0, is assigned the value of 8.1757989156). Tuning is based upon A=440.

float midi[127];
int a = 440; // a is 440 hz...
for (int x = 0; x < 127; ++x)
{
   midi[x] = (a / 32) * (2 ^ ((x - 9) / 12));
}




*/
