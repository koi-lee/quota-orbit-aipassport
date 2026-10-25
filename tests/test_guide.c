#include "guide_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void down(guide_model_t *m,unsigned n) { while(n--) guide_key(m,1); }
static void storage(void) {
    guide_record_t r={.seen=1,.liked=1u<<21,.last=21,.sequence=UINT32_MAX},out;
    uint8_t a[GUIDE_RECORD_SIZE],b[GUIDE_RECORD_SIZE]; guide_record_encode(&r,a);
    assert(guide_record_decode(&out,a,sizeof(a)) && out.last==21 && out.seen==1 && out.liked==r.liked);
    for(unsigned i=0;i<sizeof(a);i++) {
        memcpy(b,a,sizeof(a)); b[i]^=1; assert(!guide_record_decode(&out,b,sizeof(b)));
    }
    for(unsigned i=0;i<sizeof(a);i++) assert(!guide_record_decode(&out,a,i));
    assert(guide_record_select(&out,NULL,0,NULL,0)==-1 && out.last==GUIDE_NONE);
    r.sequence=0; r.last=0; guide_record_encode(&r,b);
    assert(guide_record_select(&out,a,sizeof(a),b,sizeof(b))==1 && out.sequence==0);
    assert(guide_record_select(&out,b,sizeof(b),a,sizeof(a))==0 && out.sequence==0);
    b[0]^=1; assert(guide_record_select(&out,a,sizeof(a),b,sizeof(b))==0 && out.last==21);
    r.last=22; guide_record_encode(&r,b); assert(!guide_record_decode(&out,b,sizeof(b)));
    r.last=GUIDE_NONE; r.seen=1u<<22; guide_record_encode(&r,b); assert(!guide_record_decode(&out,b,sizeof(b)));
}
static unsigned written_slot;
static bool fail_write;
static uint8_t persisted[2][GUIDE_RECORD_SIZE];
static bool writer(void *context,unsigned slot,const uint8_t *p,size_t len) {
    assert(context==persisted && slot<2 && len==GUIDE_RECORD_SIZE);
    written_slot=slot;
    if(fail_write) return false;
    memcpy(persisted[slot],p,len); return true;
}
static void save_failure(void) {
    guide_record_t r={.last=0,.sequence=7}; int slot=-1;
    assert(guide_record_save(&r,&slot,writer,persisted) && r.sequence==8 && slot==0 && written_slot==0);
    uint8_t before[GUIDE_RECORD_SIZE]; memcpy(before,persisted[0],sizeof(before));
    r.liked=1; fail_write=true;
    assert(!guide_record_save(&r,&slot,writer,persisted) && r.sequence==8 && slot==0 && written_slot==1);
    assert(!memcmp(before,persisted[0],sizeof(before)));
    guide_record_t recovered; assert(guide_record_select(&recovered,persisted[0],24,persisted[1],24)==0 && recovered.liked==0);
    fail_write=false; assert(guide_record_save(&r,&slot,writer,persisted) && slot==1 && r.sequence==9);
    assert(guide_record_select(&recovered,persisted[0],24,persisted[1],24)==1 && recovered.liked==1);
    persisted[1][5]^=1;
    assert(guide_record_select(&recovered,persisted[0],24,persisted[1],24)==0 && recovered.sequence==8);
}
static void navigation(void) {
    guide_model_t m; guide_init(&m,(guide_record_t){.last=GUIDE_NONE});
    assert(guide_count(&m)==5); guide_key(&m,0); assert(m.nav.view==GUIDE_LIST && guide_count(&m)==15);
    guide_key(&m,0); assert(m.nav.view==GUIDE_FILTER); down(&m,2); guide_key(&m,0);
    assert(m.nav.view==GUIDE_LIST && m.nav.filter==2 && guide_count(&m)==3);
    down(&m,2); assert(guide_key(&m,0)); assert(m.nav.spot==7 && m.nav.view==GUIDE_READ);
    guide_key(&m,1); assert(m.nav.page==1); guide_key(&m,0); assert(m.nav.view==GUIDE_ACTIONS);
    guide_key(&m,1); assert(guide_key(&m,0)); assert(m.record.seen==(1u<<7) && m.nav.page==1 && m.notice);
    guide_key(&m,0); down(&m,2); assert(guide_key(&m,0)); assert(m.record.liked==(1u<<7));
    guide_key(&m,2); assert(m.nav.view==GUIDE_LIST && m.nav.selected==2);
    guide_key(&m,2); assert(m.nav.view==GUIDE_HOME && guide_count(&m)==6 && m.nav.selected==1);
    guide_key(&m,-1); // Continue is inserted above the retained Jiuzhaigou selection.
    assert(!guide_key(&m,0)); assert(m.nav.view==GUIDE_READ && m.nav.spot==7);
    guide_key(&m,2); down(&m,3); guide_key(&m,0); assert(m.nav.view==GUIDE_RECORDS);
    guide_key(&m,0); assert(m.nav.view==GUIDE_COLLECTION && guide_count(&m)==1);
    guide_key(&m,0); assert(m.nav.spot==7); guide_key(&m,2); guide_key(&m,2);
    down(&m,3); guide_key(&m,0); assert(m.nav.view==GUIDE_CLEAR && m.nav.selected==0);
    guide_key(&m,0); assert(m.record.liked && m.record.seen); // cancellation is default
    guide_key(&m,0); guide_key(&m,1); assert(guide_key(&m,0));
    assert(!m.record.liked && !m.record.seen && m.record.last==GUIDE_NONE);
    guide_key(&m,2); assert(m.nav.view==GUIDE_HOME);
    // Home selection may shrink after clear. It must remain valid.
    assert(m.nav.selected==2 && !strcmp(guide_item(&m,m.nav.selected),"我的足迹"));
    m.record.liked=(1u<<1)|(1u<<2)|(1u<<3);
    m.nav=(guide_nav_t){.view=GUIDE_COLLECTION,.selected=2}; m.depth=0;
    guide_key(&m,0); guide_key(&m,0); down(&m,2); guide_key(&m,0); guide_key(&m,2);
    assert(m.nav.view==GUIDE_COLLECTION && guide_count(&m)==2 && m.nav.selected==1);
}
static void content(void) {
    unsigned parks[2]={0};
    for(unsigned id=0;id<GUIDE_SPOTS;id++) {
        const guide_spot_t *s=&guide_spots[id]; assert(s->park<2 && s->area>=1 && s->area<=5 && s->source<4);
        parks[s->park]++; assert(strlen(s->name)>0 && strlen(s->intro)>30 && strlen(s->observe)>20);
        guide_model_t m; guide_init(&m,(guide_record_t){.last=GUIDE_NONE});
        m.nav.view=GUIDE_LIST; m.nav.park=s->park;
        unsigned row=0; while(guide_list_id(&m,row)!=id) { assert(row<22); row++; }
        m.nav.selected=(uint8_t)(row+(s->park==0)); assert(guide_key(&m,0)); assert(m.nav.spot==id);
        for(unsigned j=0;j<20;j++) { guide_key(&m,1); assert(m.nav.page<2); }
        guide_key(&m,2); assert(m.nav.view==GUIDE_LIST && m.nav.selected==row+(s->park==0));
    }
    assert(parks[0]==14 && parks[1]==8);
}
static void voice(void) {
    guide_model_t m; guide_init(&m,(guide_record_t){.last=GUIDE_NONE});
    m.nav=(guide_nav_t){.view=GUIDE_LIST,.park=1};
    guide_key(&m,0); assert(m.voice==GUIDE_VOICE_OFF && m.command==GUIDE_COMMAND_NONE);
    guide_key(&m,0); assert(m.nav.view==GUIDE_ACTIONS && guide_count(&m)==6 && !strcmp(guide_item(&m,0),"听介绍"));
    assert(!guide_key(&m,0)); assert(m.command==GUIDE_COMMAND_PLAY && m.voice==GUIDE_VOICE_STARTING && m.nav.view==GUIDE_READ);
    guide_key(&m,0); assert(m.command==GUIDE_COMMAND_STOP && m.voice==GUIDE_VOICE_STOPPING);
    m.voice=GUIDE_VOICE_PLAYING; guide_key(&m,1); assert(m.command==GUIDE_COMMAND_NONE && m.nav.page==1);
    guide_key(&m,2); assert(m.command==GUIDE_COMMAND_STOP && m.nav.view==GUIDE_LIST);
    m.voice=GUIDE_VOICE_ERROR; guide_key(&m,0); guide_key(&m,0); guide_key(&m,0);
    assert(m.command==GUIDE_COMMAND_PLAY); // failure remains retryable
}
static void sound(void) {
    guide_model_t m;guide_init(&m,(guide_record_t){.last=14});
    down(&m,5);guide_key(&m,0);assert(m.nav.view==GUIDE_SOUND&&m.nav.spot==14&&m.volume==95);
    guide_key(&m,0);assert(m.command==GUIDE_COMMAND_PLAY&&m.voice==GUIDE_VOICE_STARTING);
    m.voice=GUIDE_VOICE_PLAYING;guide_key(&m,-1);assert(m.volume==100&&m.command==GUIDE_COMMAND_NONE);
    for(unsigned i=0;i<30;i++)guide_key(&m,-1);assert(m.volume==100);
    for(unsigned i=0;i<30;i++)guide_key(&m,1);assert(m.volume==0&&m.command==GUIDE_COMMAND_STOP);
    m.voice=GUIDE_VOICE_OFF;guide_key(&m,0);assert(m.command==GUIDE_COMMAND_NONE&&m.notice);
    guide_key(&m,-1);guide_key(&m,0);assert(m.volume==5&&m.command==GUIDE_COMMAND_PLAY);
    guide_key(&m,2);assert(m.nav.view==GUIDE_HOME&&m.save_volume&&m.command==GUIDE_COMMAND_STOP);
    guide_key(&m,0);assert(!m.save_volume);
}
int main(void) { sound(); storage(); save_failure(); navigation(); content(); voice(); puts("Guide records/navigation/content: PASS"); }
