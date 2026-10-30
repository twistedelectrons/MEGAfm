#!/usr/bin/env python3
"""Run the production envelope handlers against a recording YM2612 stub."""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def block(source, marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


fm = (ROOT / 'src/FM.cpp').read_text()
pots = (ROOT / 'src/pots.cpp').read_text()
source = r'''
#include <cassert>
#include <cstdio>
using byte = unsigned char;
#define bitRead(v,b) (((v) >> (b)) & 1)
#define bitWrite(v,b,x) ((v) = ((v) & ~(1 << (b))) | ((!!(x)) << (b)))
byte SSEG[4] = {}, fmBase[51] = {}, fmData[51] = {};
bool linked[3][51] = {}, loopChanged;
int showSSEGCounter;
void showSSEG() {}
void ledNumberCond(int, bool) {}
void sendNRPN(int, byte) {}
const int NRPN_OP1_BASE=2000, NRPN_OP2_BASE=3000;
const int NRPN_OP3_BASE=4000, NRPN_OP4_BASE=5000, NRPN_OP_RATE_SCALE=9;
struct YM {
    bool selected[4] = {};
    int rate[4] = {}, ssg[4] = {};
    void selectOperator(int op, int value) { selected[op] = value; }
    void setSSG_EG(int value) { for(int i=0;i<4;i++) if(selected[i]) ssg[i]=value; }
    void setRateScaling(int value) { for(int i=0;i<4;i++) if(selected[i]) rate[i]=value; }
} ym;
void updateSSEG(byte);
'''
for marker in ('void setSSEG(', 'void updateSSEG(', 'void updateFMifNecessary('):
    source += block(fm, marker) + '\n'
for op in range(1, 5):
    case = pots[pots.index('case FADER_DETUNE_%d:' % op):]
    source += 'void moveRate%d(byte data) { bool loopHeld=true, isMidi=false;\n' % op
    source += block(case, 'if (loopHeld)') + '\n}\n'
source += r'''
int main() {
    for(int slot=0;slot<4;slot++) {
        for(int rate=0;rate<4;rate++) {
            for(int bits=0;bits<4;bits++) {
                for(int other=0;other<4;other++) {
                    ym.rate[other]=(rate+other)%4;
                    ym.ssg[other]=0;
                    SSEG[other]=0;
                }
                int before[4];
                for(int other=0;other<4;other++) before[other]=ym.rate[other];
                setSSEG(slot, 0, bits & 1);
                setSSEG(slot, 1, (bits >> 1) & 1);
                assert(ym.ssg[slot]==((bits & 2 ? 8 : 0) | (bits & 1 ? 0 : 6)));
                for(int other=0;other<4;other++) assert(ym.rate[other]==before[other]);
                // Both preset restore and repeated mode changes must leave rates alone.
                for(int other=0;other<4;other++) updateSSEG(other);
                for(int other=0;other<4;other++) assert(ym.rate[other]==before[other]);
            }
        }
    }
    void (*move[])(byte)={moveRate1,moveRate2,moveRate3,moveRate4};
    const int indices[]={3,12,21,30};
    for(int op=0;op<4;op++) {
        for(int value=0;value<256;value++) {
            const int index=indices[op];
            fmBase[index]=255-value;
            fmData[index]=255-value;
            move[op](value);
            assert(fmBase[index]==value && fmData[index]==value);
        }
    }
    puts("Envelope modes preserve all four rate scales; all 256 fader values apply immediately.");
}
'''
with tempfile.TemporaryDirectory(prefix='megafm-envelope-test-') as directory:
    cpp = Path(directory) / 'test.cpp'
    binary = Path(directory) / 'test'
    cpp.write_text(source)
    subprocess.run(shlex.split(os.environ.get('CXX', 'c++')) + [
        '-std=c++11', '-fsanitize=undefined', '-fno-sanitize-recover=all',
        str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
