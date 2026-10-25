#include "guide_adpcm.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv) {
    if(argc!=4)return 2;
    FILE *f=fopen(argv[1],"rb");if(!f)return 3;
    fseek(f,0,SEEK_END);long length=ftell(f);rewind(f);
    if(length<=0||length>4000000)return 4;
    unsigned char *data=malloc((size_t)length);if(!data)return 5;
    if(fread(data,1,(size_t)length,f)!=(size_t)length)return 6;
    fclose(f);f=fopen(argv[3],"wb");if(!f)return 7;
    guide_adpcm_t state={0};int16_t pcm[160];size_t count,samples=strtoul(argv[2],NULL,10);
    while((count=guide_adpcm_decode(&state,data,(size_t)length,samples,pcm,160))) {
        for(size_t i=0;i<count;i++) {unsigned v=(uint16_t)pcm[i];fputc(v&255,f);fputc(v>>8,f);}
    }
    free(data);fclose(f);return state.position==samples?0:8;
}
