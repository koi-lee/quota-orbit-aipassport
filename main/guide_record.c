#include "guide_record.h"
#include <string.h>
static void set32(uint8_t *p, uint32_t v) { for (unsigned i=0;i<4;i++) p[i]=(uint8_t)(v>>(8*i)); }
static uint32_t get32(const uint8_t *p) { uint32_t v=0; for (unsigned i=0;i<4;i++) v|=(uint32_t)p[i]<<(8*i); return v; }
static uint32_t crc(const uint8_t *p, size_t n) {
    uint32_t c=UINT32_MAX;
    for (size_t i=0;i<n;i++) { c^=p[i]; for(unsigned j=0;j<8;j++) c=(c>>1)^((0u-(c&1u))&UINT32_C(0xedb88320)); }
    return ~c;
}
void guide_record_encode(const guide_record_t *r, uint8_t out[GUIDE_RECORD_SIZE]) {
    memset(out,0,GUIDE_RECORD_SIZE); memcpy(out,"PG01",4);
    set32(out+4,r->sequence); set32(out+8,r->seen); set32(out+12,r->liked); out[16]=r->last;
    set32(out+20,crc(out,20));
}
bool guide_record_decode(guide_record_t *r, const uint8_t *p, size_t n) {
    if (!p || n!=GUIDE_RECORD_SIZE || memcmp(p,"PG01",4) || get32(p+20)!=crc(p,20)) return false;
    if ((get32(p+8)|get32(p+12))&~GUIDE_MASK || (p[16]>=GUIDE_SPOTS && p[16]!=GUIDE_NONE)) return false;
    *r=(guide_record_t){.sequence=get32(p+4),.seen=get32(p+8),.liked=get32(p+12),.last=p[16]};
    return true;
}
int guide_record_select(guide_record_t *r, const uint8_t *a, size_t na, const uint8_t *b, size_t nb) {
    guide_record_t ra,rb; bool va=guide_record_decode(&ra,a,na), vb=guide_record_decode(&rb,b,nb);
    if(!va && !vb) { *r=(guide_record_t){.last=GUIDE_NONE}; return -1; }
    if(vb && (!va || (rb.sequence!=ra.sequence && (uint32_t)(rb.sequence-ra.sequence)<UINT32_C(0x80000000)))) { *r=rb; return 1; }
    *r=ra; return 0;
}

bool guide_record_save(guide_record_t *r, int *slot, guide_record_writer_t writer, void *context) {
    guide_record_t next=*r; next.sequence++;
    uint8_t bytes[GUIDE_RECORD_SIZE]; guide_record_encode(&next,bytes);
    unsigned target=*slot==0?1u:0u;
    if(!writer || !writer(context,target,bytes,sizeof(bytes))) return false;
    *r=next; *slot=(int)target; return true;
}
