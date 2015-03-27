/*
 * header file for mxp.c
 *
 * Brian Graversen
 * Modified for Merc 2.2 by Celestian (celestian1@gmail.com)
 */
#include <arpa/telnet.h>

/* MXP defs */
#define MXP_SAFE                        1
#define MXP_ALL                         2
#define MXP_NONE                        3
#define TELOPT_MXP                     91

/* global strings */
extern const char mxp_do[];
extern const char mxp_dont[];
extern const char mxp_will[];

