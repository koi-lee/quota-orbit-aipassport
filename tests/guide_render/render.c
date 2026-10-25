#include "lvgl.h"
#include "guide_ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
LV_FONT_DECLARE(guide_font_12);
LV_FONT_DECLARE(guide_font_14);
LV_FONT_DECLARE(guide_font_16);
LV_FONT_DECLARE(guide_font_20);
static unsigned char pixels[240*320*3],draw[240*40*2];
static unsigned states;
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *map) {
    for(int y=a->y1;y<=a->y2;y++) for(int x=a->x1;x<=a->x2;x++) {
        unsigned v=map[0]|((unsigned)map[1]<<8); map+=2;
        if(x<0||x>=240||y<0||y>=320) continue;
        unsigned p=(unsigned)(y*240+x)*3;
        pixels[p]=((v>>11)&31)*255/31; pixels[p+1]=((v>>5)&63)*255/63; pixels[p+2]=(v&31)*255/31;
    }
    lv_display_flush_ready(d);
}
static void labels(lv_obj_t *o) {
    if(lv_obj_check_type(o,&lv_label_class)) {
        const char *body=lv_label_get_text(o);
        const char *const prohibited[]={"\n，","\n。","\n？","\n！","\n：","\n；","\n、","\n”","\n）"};
        for(unsigned k=0;k<sizeof(prohibited)/sizeof(prohibited[0]);k++) assert(!strstr(body,prohibited[k]));
        lv_area_t a; lv_obj_get_coords(o,&a);
        if(a.x1<12||a.y1<12||a.x2>=228||a.y2>=309) {
            fprintf(stderr,"Out of safe area: %s (%d,%d)-(%d,%d)\n",lv_label_get_text(o),a.x1,a.y1,a.x2,a.y2); abort();
        }
        // Text must not overlap another label; explicit multi-line labels are fine.
        lv_obj_t *parent=lv_obj_get_parent(o);
        for(uint32_t i=0;i<lv_obj_get_child_count(parent);i++) {
            lv_obj_t *other=lv_obj_get_child(parent,i);
            if(other==o || !lv_obj_check_type(other,&lv_label_class)) continue;
            lv_area_t b; lv_obj_get_coords(other,&b);
            if(a.x1<=b.x2&&a.x2>=b.x1&&a.y1<=b.y2&&a.y2>=b.y1) {
                fprintf(stderr,"Overlapping labels: %s / %s\n",lv_label_get_text(o),lv_label_get_text(other)); abort();
            }
        }
    }
    for(uint32_t i=0;i<lv_obj_get_child_count(o);i++) labels(lv_obj_get_child(o,i));
}
static void save(const char *dir,const char *name,guide_model_t *m,int battery) {
    guide_ui_draw(m,battery); lv_refr_now(NULL); labels(lv_screen_active()); states++;
    if(m->nav.view==GUIDE_READ && m->nav.page) {
        bool found=false;
        for(uint32_t i=0;i<lv_obj_get_child_count(lv_screen_active());i++) {
            lv_obj_t *child=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(child,&lv_label_class) && strstr(lv_label_get_text(child),guide_spots[m->nav.spot].name)) found=true;
        }
        assert(found);
    }
    if(!name) return;
    char path[512]; snprintf(path,sizeof(path),"%s/%s.ppm",dir,name);
    FILE *f=fopen(path,"wb"); assert(f); fprintf(f,"P6\n240 320\n255\n");
    assert(fwrite(pixels,1,sizeof(pixels),f)==sizeof(pixels)); assert(!fclose(f));
}
static void glyphs(const char *path) {
    FILE *f=fopen(path,"rb"); assert(f); unsigned count=0; int b;
    const lv_font_t *fonts[]={&guide_font_12,&guide_font_14,&guide_font_16,&guide_font_20};
    while((b=fgetc(f))!=EOF) {
        uint32_t cp=(unsigned)b; unsigned tail=0;
        if(cp>=0xf0) { cp&=7;tail=3; } else if(cp>=0xe0) { cp&=15;tail=2; } else if(cp>=0xc0) { cp&=31;tail=1; }
        while(tail--) { int c=fgetc(f); assert(c!=EOF && (c&0xc0)==0x80); cp=(cp<<6)|(c&63); }
        if(cp<32) continue;
        for(unsigned i=0;i<4;i++) { lv_font_glyph_dsc_t dsc; assert(lv_font_get_glyph_dsc(fonts[i],&dsc,cp,0)); }
        count++;
    }
    fclose(f); printf("Glyphs: %u x 4 fonts PASS\n",count);
}
int main(int argc,char **argv) {
    assert(argc==3 || argc==4); lv_init();
    lv_display_t *d=lv_display_create(240,320); lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(d,draw,NULL,sizeof(draw),LV_DISPLAY_RENDER_MODE_PARTIAL); lv_display_set_flush_cb(d,flush);
    glyphs(argv[2]);
    guide_model_t m; guide_init(&m,(guide_record_t){.last=GUIDE_NONE});
    if(argc==4) {
        for(const char *key=argv[3];*key;key++) {
            if(*key=='e' || *key=='f') { m.voice=*key=='e'?GUIDE_VOICE_OFF:GUIDE_VOICE_ERROR; continue; }
            int action=*key=='u'?-1:*key=='d'?1:*key=='b'?2:0;
            guide_key(&m,action);
            if(m.command==GUIDE_COMMAND_PLAY) m.voice=GUIDE_VOICE_PLAYING;
            if(m.command==GUIDE_COMMAND_STOP) m.voice=GUIDE_VOICE_OFF;
        }
        save(argv[1],"live",&m,-1);
        printf("{\"view\":%u,\"spot\":%u,\"seen\":%u,\"liked\":%u,\"last\":%u,\"voice\":%u,\"voice_id\":%u,\"volume\":%u}\n",
            m.nav.view,m.nav.spot,guide_bits(m.record.seen),guide_bits(m.record.liked),m.record.last,m.voice,m.voice==GUIDE_VOICE_PLAYING?m.nav.spot:GUIDE_NONE,m.volume);
        lv_deinit(); return 0;
    }
    save(argv[1],"00-home",&m,78);
    for(unsigned park=0;park<2;park++) {
        m.nav=(guide_nav_t){.view=GUIDE_LIST,.park=park};
        unsigned count=guide_count(&m);
        for(unsigned row=0;row<count;row++) { m.nav.selected=row; save(argv[1],row==1?(park?"02-huanglong":"01-jiuzhai"):NULL,&m,78); }
    }
    for(unsigned spot=0;spot<GUIDE_SPOTS;spot++) {
        m.nav=(guide_nav_t){.view=GUIDE_READ,.spot=spot,.park=guide_spots[spot].park};
        char name[40]; snprintf(name,sizeof(name),"spot-%02u-intro",spot); save(argv[1],name,&m,78);
        m.nav.page=1; snprintf(name,sizeof(name),"spot-%02u-observe",spot); save(argv[1],name,&m,78);
        m.nav.page=0; m.nav.view=GUIDE_INFO; save(argv[1],spot==0?"05-source":NULL,&m,78);
        m.nav.view=GUIDE_ACTIONS;
        for(unsigned row=0;row<6;row++) { m.nav.selected=row; save(argv[1],spot==0&&row==0?"04-actions":NULL,&m,78); }
    }
    for(unsigned view=0;view<=GUIDE_SOUND;view++) {
        m.nav=(guide_nav_t){.view=(guide_view_t)view};
        unsigned count=guide_count(&m);
        for(unsigned row=0;row<(count?count:1);row++) {
            m.nav.selected=row; char name[40]; snprintf(name,sizeof(name),"view-%02u-%02u",view,row); save(argv[1],name,&m,-1);
        }
    }
    m.record=(guide_record_t){.seen=GUIDE_MASK,.liked=GUIDE_MASK,.last=21};
    m.nav=(guide_nav_t){.view=GUIDE_HOME}; save(argv[1],"06-continue",&m,100);
    m.nav.view=GUIDE_COLLECTION;
    for(unsigned row=0;row<GUIDE_SPOTS;row++) { m.nav.selected=row; save(argv[1],row==21?"07-collection":NULL,&m,1); }
    m.nav.view=GUIDE_STATS; save(argv[1],"08-stats",&m,78);
    m.nav=(guide_nav_t){.view=GUIDE_READ,.spot=0}; m.storage_ok=false; save(argv[1],"09-storage-failed",&m,-1);
    m.notice="未能保存，原记录已保留"; save(argv[1],"10-notice",&m,78);
    m.notice=NULL; m.storage_ok=true;
    for(unsigned voice=GUIDE_VOICE_STARTING;voice<=GUIDE_VOICE_ERROR;voice++) {
        m.voice=(guide_voice_state_t)voice;
        for(unsigned view=0;view<=GUIDE_SOUND;view++) { m.nav.view=(guide_view_t)view; m.nav.selected=0; save(argv[1],NULL,&m,78); }
    }
    m.voice=GUIDE_VOICE_OFF;
    m.nav=(guide_nav_t){.view=GUIDE_SOUND,.spot=14};
    m.volume=85;save(argv[1],"sound-idle",&m,78);
    m.volume=100;m.voice=GUIDE_VOICE_PLAYING;save(argv[1],"sound-playing",&m,78);
    m.volume=0;m.voice=GUIDE_VOICE_OFF;save(argv[1],"sound-muted",&m,78);
    for(unsigned i=0;i<1000;i++) { m.nav.view=(guide_view_t)(i%12); unsigned n=guide_count(&m); m.nav.selected=n?i%n:0; save(argv[1],NULL,&m,78); }
    lv_mem_monitor_t before,after; lv_mem_monitor(&before);
    for(unsigned i=0;i<100;i++) save(argv[1],NULL,&m,78);
    lv_mem_monitor(&after); assert(after.used_cnt==before.used_cnt && after.free_size==before.free_size);
    printf("Actual LVGL render: %u states; label safety, overlap, redraw memory PASS\n",states);
    printf("LVGL pool: total=%zu free=%zu peak_used=%zu bytes\n",after.total_size,after.free_size,after.max_used);
    lv_deinit(); return 0;
}
