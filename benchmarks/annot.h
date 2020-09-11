#ifndef ANNOT_H
#define ANNOT_H

#define SWITCH ANNOT_SWITCH(); switch

#if defined(ARM_ARCHI)
/* redefinition of some functions */
#include "ARMlib.h"
#endif

// --------------------------------
// New ones in binary
// --------------------------------
#if defined(NO_WCET_INFO)
#define ANNOT_MAXITER(n)
#define ANNOT_SEND(n)
#else

#define stringify(x) #x 

// 1b : one before, 1f: one from
#define ANNOT_MAXITER(n) __asm__("1:\n\
        .section .wcet_annot\n\
        .long 1b\n\
        .long 1\n\
        .long " stringify(n) "\n                \
        .text");

// Annotations for mpsoc WCRT estimation
#define ANNOT_SEND(n,d) __asm__("1:\n\
        .section .wcet_annot\n\
        .long 1b\n\
        .long 2\n\
        .long $" n "\n\
        .long " #d "\n\
        .text");
#define ANNOT_MARKER_RECEIVE(label) __asm__("$"label ":\n");
#define ANNOT_FORK_JOIN(n,args...) __asm__("1:\n\
        .section .wcet_annot\n\
        .long 1b\n\
        .long 3\n\
        .long " #n "\n\
        .long " #args "\n\
        .text");


#define ANNOT_SWITCH() __asm__("1:\n\
        .section .switch_begin\n\
        .long 1b\n\
        .text");

#endif

#endif
