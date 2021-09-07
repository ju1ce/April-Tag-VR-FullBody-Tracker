#include <stdlib.h>
#include "tagStandard20h6.h"

static uint64_t codedata[96] = {
   0x0000000000034c3dUL,
   0x00000000000157c7UL,
   0x00000000000f6351UL,
   0x00000000000c74a0UL,
   0x00000000000b7a65UL,
   0x00000000000a802aUL,
   0x0000000000079179UL,
   0x000000000006973eUL,
   0x000000000003a88dUL,
   0x000000000002ae52UL,
   0x000000000000b9dcUL,
   0x000000000006f38eUL,
   0x000000000005f953UL,
   0x000000000001162cUL,
   0x00000000000d2d40UL,
   0x0000000000055b68UL,
   0x00000000000366f2UL,
   0x00000000000d8990UL,
   0x000000000009fcf4UL,
   0x0000000000095f09UL,
   0x0000000000018d31UL,
   0x00000000000ab594UL,
   0x0000000000014b96UL,
   0x000000000000515bUL,
   0x00000000000f5720UL,
   0x00000000000590d2UL,
   0x00000000000cc4bfUL,
   0x00000000000bca84UL,
   0x000000000004f2e7UL,
   0x00000000000454fcUL,
   0x000000000007fc4dUL,
   0x0000000000002a75UL,
   0x00000000000c91a8UL,
   0x000000000009a2f7UL,
   0x00000000000ff75eUL,
   0x00000000000fb5c3UL,
   0x000000000007e3ebUL,
   0x0000000000043069UL,
   0x00000000000d58ccUL,
   0x00000000000350e3UL,
   0x000000000009f569UL,
   0x000000000000cf59UL,
   0x000000000008afb5UL,
   0x00000000000c71bbUL,
   0x000000000006e478UL,
   0x00000000000533f0UL,
   0x00000000000f568eUL,
   0x0000000000093791UL,
   0x00000000000dd91dUL,
   0x0000000000075dd0UL,
   0x000000000007a411UL,
   0x000000000006c6deUL,
   0x00000000000f2092UL,
   0x000000000009e36eUL,
   0x0000000000049a19UL,
   0x00000000000c450bUL,
   0x00000000000b6585UL,
   0x00000000000427baUL,
   0x000000000007444aUL,
   0x00000000000e2947UL,
   0x000000000008e6ebUL,
   0x000000000002dd6bUL,
   0x000000000004be25UL,
   0x00000000000cfad1UL,
   0x000000000007d298UL,
   0x000000000007789bUL,
   0x00000000000b8749UL,
   0x00000000000a6628UL,
   0x00000000000e9302UL,
   0x0000000000013ca1UL,
   0x0000000000012b38UL,
   0x00000000000c4c25UL,
   0x00000000000093b4UL,
   0x0000000000036a03UL,
   0x0000000000033407UL,
   0x000000000006dc87UL,
   0x0000000000062516UL,
   0x00000000000659eeUL,
   0x00000000000093caUL,
   0x00000000000e74f7UL,
   0x000000000001d322UL,
   0x00000000000ddeadUL,
   0x00000000000fdb4bUL,
   0x00000000000d9484UL,
   0x0000000000071e7aUL,
   0x00000000000cabe9UL,
   0x00000000000966bdUL,
   0x00000000000e429bUL,
   0x00000000000db46eUL,
   0x00000000000ed191UL,
   0x00000000000c5d9eUL,
   0x000000000005690fUL,
   0x0000000000016c67UL,
   0x00000000000d6ecfUL,
   0x00000000000ea4acUL,
   0x00000000000dd3e1UL,
};
apriltag_family_t *tagStandard20h6_create()
{
   apriltag_family_t *tf = calloc(1, sizeof(apriltag_family_t));
   tf->name = strdup("tagStandard20h6");
   tf->h = 6;
   tf->ncodes = 96;
   tf->codes = codedata;
   tf->nbits = 20;
   tf->bit_x = calloc(20, sizeof(uint32_t));
   tf->bit_y = calloc(20, sizeof(uint32_t));
   tf->bit_x[0] = -2;
   tf->bit_y[0] = -2;
   tf->bit_x[1] = -1;
   tf->bit_y[1] = -2;
   tf->bit_x[2] = 0;
   tf->bit_y[2] = -2;
   tf->bit_x[3] = 1;
   tf->bit_y[3] = -2;
   tf->bit_x[4] = 2;
   tf->bit_y[4] = -2;
   tf->bit_x[5] = 3;
   tf->bit_y[5] = -2;
   tf->bit_x[6] = 3;
   tf->bit_y[6] = -1;
   tf->bit_x[7] = 3;
   tf->bit_y[7] = 0;
   tf->bit_x[8] = 3;
   tf->bit_y[8] = 1;
   tf->bit_x[9] = 3;
   tf->bit_y[9] = 2;
   tf->bit_x[10] = 3;
   tf->bit_y[10] = 3;
   tf->bit_x[11] = 2;
   tf->bit_y[11] = 3;
   tf->bit_x[12] = 1;
   tf->bit_y[12] = 3;
   tf->bit_x[13] = 0;
   tf->bit_y[13] = 3;
   tf->bit_x[14] = -1;
   tf->bit_y[14] = 3;
   tf->bit_x[15] = -2;
   tf->bit_y[15] = 3;
   tf->bit_x[16] = -2;
   tf->bit_y[16] = 2;
   tf->bit_x[17] = -2;
   tf->bit_y[17] = 1;
   tf->bit_x[18] = -2;
   tf->bit_y[18] = 0;
   tf->bit_x[19] = -2;
   tf->bit_y[19] = -1;
   tf->width_at_border = 2;
   tf->total_width = 6;
   tf->reversed_border = true;
   return tf;
}

void tagStandard20h6_destroy(apriltag_family_t *tf)
{
   free(tf->bit_x);
   free(tf->bit_y);
   free(tf->name);
   free(tf);
}
