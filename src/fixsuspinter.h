// Rboolean TRUE seems to clash with minwindef.h TRUE,
// which breaks the suspend interrupts macros.
// This is a replacement.

#include <R_ext/Boolean.h>

#ifndef HTTPGD_BEGIN_SUSPEND_INTERRUPTS
/* Macros for suspending interrupts */
#define HTTPGD_BEGIN_SUSPEND_INTERRUPTS do { \
    Rboolean __oldsusp__ = R_interrupts_suspended; \
    R_interrupts_suspended = (Rboolean)1;
#define HTTPGD_END_SUSPEND_INTERRUPTS R_interrupts_suspended = __oldsusp__; \
    if (R_interrupts_pending && ! R_interrupts_suspended) \
        Rf_onintr(); \
} while(0)
    
#include <R_ext/libextern.h>
LibExtern Rboolean R_interrupts_suspended;    
LibExtern int R_interrupts_pending;
extern void Rf_onintr(void);
LibExtern Rboolean mbcslocale;
#endif
