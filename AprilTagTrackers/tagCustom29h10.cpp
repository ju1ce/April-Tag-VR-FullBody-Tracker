#include <cstdlib>
#include "tagCustom29h10.hpp"

static uint64_t codedata[125] = {
   0x00000000197153fcUL,
   0x0000000003ad6b10UL,
   0x00000000135a7c5fUL,
   0x00000000087887e9UL,
   0x000000001d969373UL,
   0x00000000177fbbd6UL,
   0x000000000c9dc760UL,
   0x00000000072ccd25UL,
   0x000000001c4ad8afUL,
   0x000000000686efc3UL,
   0x000000000115f588UL,
   0x000000001ba4fb4dUL,
   0x000000001fca3ac4UL,
   0x000000000814b9b2UL,
   0x0000000011051602UL,
   0x00000000105f38a0UL,
   0x000000000856c8ddUL,
   0x0000000007b0eb7bUL,
   0x00000000066530b7UL,
   0x000000000a8a702eUL,
   0x000000001b1f6e0aUL,
   0x00000000078f2c6fUL,
   0x000000001cad37f9UL,
   0x0000000009c2d3c0UL,
   0x0000000015cec3eeUL,
   0x00000000103c0aa7UL,
   0x0000000004b438cfUL,
   0x000000001d51a66eUL,
   0x0000000004506a98UL,
   0x000000001a9702edUL,
   0x000000000b2c00c9UL,
   0x00000000126cd41eUL,
   0x00000000006a5c72UL,
   0x000000001b5bc181UL,
   0x000000000b6b321aUL,
   0x0000000012483738UL,
   0x00000000061a87feUL,
   0x0000000017fb409eUL,
   0x0000000010973f50UL,
   0x000000001b9814f8UL,
   0x000000000f05289aUL,
   0x000000001bf90555UL,
   0x000000000a58ecf3UL,
   0x0000000019c2802aUL,
   0x000000000a54a02cUL,
   0x0000000005a586abUL,
   0x000000000ab143c2UL,
   0x000000001f8bd134UL,
   0x0000000011abbb25UL,
   0x000000000518cec7UL,
   0x000000000f2eda08UL,
   0x000000000eeb5bf0UL,
   0x00000000027472eaUL,
   0x00000000102fec13UL,
   0x000000000cf29929UL,
   0x00000000079399cbUL,
   0x000000000b2665aaUL,
   0x000000000ea00c0bUL,
   0x000000000b7d4d8cUL,
   0x00000000188a4fc5UL,
   0x000000001c8c6143UL,
   0x0000000013ea5347UL,
   0x0000000007531a22UL,
   0x000000000ba71f8fUL,
   0x000000001cefa8e3UL,
   0x000000000bc27bbbUL,
   0x0000000012d29396UL,
   0x0000000013fc8f4eUL,
   0x0000000015aa0230UL,
   0x000000000099a4bcUL,
   0x000000000417b879UL,
   0x000000001d5ea915UL,
   0x000000000a0ee2e0UL,
   0x0000000016d7e06aUL,
   0x0000000015a6540fUL,
   0x000000001f7e9cecUL,
   0x000000001aab896eUL,
   0x000000000da0bde3UL,
   0x000000001b3b3fc1UL,
   0x000000000c3ce68cUL,
   0x000000000564095dUL,
   0x0000000012e1398bUL,
   0x0000000000f0f560UL,
   0x000000000a778b78UL,
   0x0000000008ce85f2UL,
   0x000000000c9058f1UL,
   0x000000000327b772UL,
   0x000000001df8cc62UL,
   0x0000000010e66bf1UL,
   0x00000000047ec083UL,
   0x00000000110c9b19UL,
   0x0000000003784637UL,
   0x0000000015c3a579UL,
   0x0000000010ee12daUL,
   0x000000000aec9205UL,
   0x000000001e41fa5cUL,
   0x000000000a126194UL,
   0x0000000009213101UL,
   0x00000000074dd3d8UL,
   0x0000000012ac6c8dUL,
   0x000000000c843476UL,
   0x000000001453341cUL,
   0x0000000005b33994UL,
   0x0000000011c26d12UL,
   0x000000001d834a42UL,
   0x000000001724719cUL,
   0x000000000039362bUL,
   0x000000000057ac46UL,
   0x000000001fa59ae0UL,
   0x0000000005d1692eUL,
   0x000000000ff6c547UL,
   0x0000000018083e17UL,
   0x0000000016b245ebUL,
   0x000000001cedd028UL,
   0x00000000024c41a6UL,
   0x0000000013c05e6aUL,
   0x000000000907eefbUL,
   0x00000000121d156eUL,
   0x000000001eae01b7UL,
   0x00000000165e6db9UL,
   0x000000000398228fUL,
   0x000000000658c91fUL,
   0x0000000008f5a091UL,
   0x00000000193fc7d2UL,
   0x000000001ae23623UL,
};
apriltag_family_t *tagCustom29h10_create()
{
   apriltag_family_t *tf = (apriltag_family_t*)std::calloc(1, sizeof(apriltag_family_t));
   tf->name = strdup("tagCustom29h10");
   tf->h = 10;
   tf->ncodes = 125;
   tf->codes = codedata;
   tf->nbits = 29;
   tf->bit_x = (uint32_t*)std::calloc(29, sizeof(uint32_t));
   tf->bit_y = (uint32_t*)std::calloc(29, sizeof(uint32_t));
   tf->bit_x[0] = 0;
   tf->bit_y[0] = -2;
   tf->bit_x[1] = 1;
   tf->bit_y[1] = -2;
   tf->bit_x[2] = 2;
   tf->bit_y[2] = -2;
   tf->bit_x[3] = 3;
   tf->bit_y[3] = -2;
   tf->bit_x[4] = 4;
   tf->bit_y[4] = -2;
   tf->bit_x[5] = 1;
   tf->bit_y[5] = 1;
   tf->bit_x[6] = 2;
   tf->bit_y[6] = 1;
   tf->bit_x[7] = 6;
   tf->bit_y[7] = 0;
   tf->bit_x[8] = 6;
   tf->bit_y[8] = 1;
   tf->bit_x[9] = 6;
   tf->bit_y[9] = 2;
   tf->bit_x[10] = 6;
   tf->bit_y[10] = 3;
   tf->bit_x[11] = 6;
   tf->bit_y[11] = 4;
   tf->bit_x[12] = 3;
   tf->bit_y[12] = 1;
   tf->bit_x[13] = 3;
   tf->bit_y[13] = 2;
   tf->bit_x[14] = 4;
   tf->bit_y[14] = 6;
   tf->bit_x[15] = 3;
   tf->bit_y[15] = 6;
   tf->bit_x[16] = 2;
   tf->bit_y[16] = 6;
   tf->bit_x[17] = 1;
   tf->bit_y[17] = 6;
   tf->bit_x[18] = 0;
   tf->bit_y[18] = 6;
   tf->bit_x[19] = 3;
   tf->bit_y[19] = 3;
   tf->bit_x[20] = 2;
   tf->bit_y[20] = 3;
   tf->bit_x[21] = -2;
   tf->bit_y[21] = 4;
   tf->bit_x[22] = -2;
   tf->bit_y[22] = 3;
   tf->bit_x[23] = -2;
   tf->bit_y[23] = 2;
   tf->bit_x[24] = -2;
   tf->bit_y[24] = 1;
   tf->bit_x[25] = -2;
   tf->bit_y[25] = 0;
   tf->bit_x[26] = 1;
   tf->bit_y[26] = 3;
   tf->bit_x[27] = 1;
   tf->bit_y[27] = 2;
   tf->bit_x[28] = 2;
   tf->bit_y[28] = 2;
   tf->width_at_border = 5;
   tf->total_width = 9;
   tf->reversed_border = true;
   return tf;
}

void tagCustom29h10_destroy(apriltag_family_t *tf)
{
   free(tf->bit_x);
   free(tf->bit_y);
   free(tf->name);
   free(tf);
}
