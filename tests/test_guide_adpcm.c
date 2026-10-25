#include "guide_adpcm.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    // Standard IMA deltas at initial index zero, including both signs.
    uint8_t data[]={0x71,0xf8,0x40}; int16_t pcm[6]; guide_adpcm_t s={0};
    assert(guide_adpcm_decode(&s,data,sizeof(data),6,pcm,6)==6);
    const int16_t expected[]={1,12,10,-15,-12,19};
    assert(!memcmp(pcm,expected,sizeof(pcm)));
    guide_adpcm_t chunked={0}; int16_t split[6];
    for(unsigned i=0;i<6;i++) assert(guide_adpcm_decode(&chunked,data,sizeof(data),6,split+i,1)==1);
    assert(!memcmp(pcm,split,sizeof(pcm)));
    assert(guide_adpcm_decode(&s,data,sizeof(data),6,pcm,6)==0);
    s=(guide_adpcm_t){0};assert(guide_adpcm_decode(&s,data,1,6,pcm,6)==2);
    s=(guide_adpcm_t){0};assert(guide_adpcm_decode(&s,data,3,5,pcm,6)==5);
    s=(guide_adpcm_t){.predictor=32767,.index=88}; uint8_t extremes[]={0x77,0xff};
    assert(guide_adpcm_decode(&s,extremes,2,4,pcm,6)==4 && pcm[0]==32767 && pcm[1]==32767 && pcm[2]==-28669 && pcm[3]==-32768);
    s=(guide_adpcm_t){.index=89}; assert(!guide_adpcm_decode(&s,data,3,6,pcm,6));
    puts("Guide IMA decoding, sign/clamp, chunking, odd tail and truncation PASS");
}
