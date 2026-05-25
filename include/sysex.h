#ifndef MEGAFM_CMAKE_SYSEX_H
#define MEGAFM_CMAKE_SYSEX_H

#define MAX_SYSEX_DATA_LENGTH 3950
#define SYSEX_STATUS_OK 0
#define SYSEX_STATUS_BYTE_ERROR 1
#define SYSEX_STATUS_HEADER_MISMATCH 2
#define SYSEX_STATUS_LENGTH_ERROR 3
#define SYSEX_STATUS_DUMP_LENGTH_ERROR 4
#define SYSEX_STATUS_UNKNOWN_COMMAND 5

void handleIncomingSysEx();
void sysExExitStatus(byte error);
void sysExReset();
void sysExAppendByte(byte b);
bool processingSysex();

#endif // MEGAFM_CMAKE_SYSEX_H
