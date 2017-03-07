#ifndef _OK_H
#define _OK_H

#if ARDUINO <100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

extern int TrigPin; 
extern int EchoPin; 
extern float distance;
extern void mm(void);

#endif