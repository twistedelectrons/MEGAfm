#ifndef MEGAFM_CMAKE_NRPN_H
#define MEGAFM_CMAKE_NRPN_H

extern int nrpn_msg;
extern int nrpn_data;
extern byte nrpn_state;

void sendNRPN(int msg, int value);
void handleNRPN(int msg, int int_val);
void initLastNRPN();

#endif // MEGAFM_CMAKE_NRPN_H
