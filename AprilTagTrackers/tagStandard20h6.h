#ifndef _TAGStandard20H6
#define _TAGStandard20H6

#include "apriltag.h"

#ifdef __cplusplus
extern "C" {
#endif

apriltag_family_t *tagStandard20h6_create();
void tagStandard20h6_destroy(apriltag_family_t *tf);

#ifdef __cplusplus
}
#endif

#endif
