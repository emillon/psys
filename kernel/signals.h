#ifndef _SIGNALS_H
#define _SIGNALS_H 1

#include "context.h"
#include "list.h"


void sigaction(int signalid, void* ad_trt);

void signal_send(int pid, int signalid);

void trt_sigs(void);

void sig_return(void);

extern char* ad_sig_default;

#endif
