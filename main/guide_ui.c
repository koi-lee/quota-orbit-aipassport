#include "guide_ui.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
LV_FONT_DECLARE(guide_font_12);
LV_FONT_DECLARE(guide_font_14);
LV_FONT_DECLARE(guide_font_16);
LV_FONT_DECLARE(guide_font_20);
LV_IMAGE_DECLARE(guide_jiuzhai_hero);
LV_IMAGE_DECLARE(guide_huanglong_hero);
LV_IMAGE_DECLARE(guide_jiuzhai_scene);
LV_IMAGE_DECLARE(guide_huanglong_scene);
LV_IMAGE_DECLARE(guide_jiuzhai_thumb);
LV_IMAGE_DECLARE(guide_huanglong_thumb);
#define INK 0x172b34
#define PAPER 0xfff0ca
#define MINT 0x91d7ba
#define MUTED 0x536b6d
static void rect(int x,int y,int w,int h,uint32_t color) {
    lv_obj_t *o=lv_obj_create(lv_screen_active()); lv_obj_remove_style_all(o);
    lv_obj_set_pos(o,x,y); lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0); lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE);
}
static void art(int x,int y,unsigned park,unsigned size) {
    const lv_image_dsc_t *image=size==0?(park?&guide_huanglong_hero:&guide_jiuzhai_hero):
        size==1?(park?&guide_huanglong_scene:&guide_jiuzhai_scene):(park?&guide_huanglong_thumb:&guide_jiuzhai_thumb);
    lv_obj_t *o=lv_image_create(lv_screen_active()); lv_image_set_src(o,image); lv_obj_set_pos(o,x,y);
}
/* Fixed-screen CJK wrapping: keep closing punctuation off the next line. */
static unsigned codepoint(const char *s,unsigned *length) {
    const unsigned char *p=(const unsigned char *)s;
    unsigned cp=p[0],n=cp<0x80?1:cp<0xe0?2:cp<0xf0?3:4;
    if(n>1) { cp&=(1u<<(7-n))-1; for(unsigned i=1;i<n;i++) cp=(cp<<6)|(p[i]&63); }
    *length=n; return cp;
}
static void label_text(lv_obj_t *label,const char *s,const lv_font_t *font,int width) {
    char out[384]; size_t used=0,last=0; int line=0,last_width=0; unsigned last_cp=0;
    const char *cursor=s;
    while(*cursor) {
        unsigned bytes,next_bytes; unsigned cp=codepoint(cursor,&bytes);
        unsigned next=cursor[bytes]?codepoint(cursor+bytes,&next_bytes):0;
        int advance=cp=='\n'?0:lv_font_get_glyph_width(font,cp,next);
        if(used+bytes+3>=sizeof(out)) { lv_label_set_text(label,s); return; }
        if(cp!='\n' && line+advance>width) {
            bool closing=cp==0x3002 || cp==0xff0c || cp==0xff1f || cp==0xff01 || cp==0xff1a || cp==0xff1b || cp==0x3001 || cp==0x201d || cp==0x2019 || cp==0xff09;
            bool opening=last_cp==0x201c || last_cp==0x2018 || last_cp==0xff08;
            if((closing || opening) && line>last_width) {
                memmove(out+last+1,out+last,used-last); out[last]='\n'; used++; line=last_width;
            } else { out[used++]='\n'; line=0; }
        }
        last=used; memcpy(out+used,cursor,bytes); used+=bytes;
        line=cp=='\n'?0:line+advance; last_width=advance; last_cp=cp; cursor+=bytes;
    }
    out[used]=0; lv_label_set_text(label,out);
}
static lv_obj_t *text(int x,int y,int w,const char *s,const lv_font_t *font,uint32_t color) {
    lv_obj_t *o=lv_label_create(lv_screen_active());
    lv_obj_set_pos(o,x,y); lv_obj_set_width(o,w);
    lv_obj_set_style_text_font(o,font,0); lv_obj_set_style_text_color(o,lv_color_hex(color),0);
    lv_obj_set_style_text_line_space(o,4,0); label_text(o,s,font,w); return o;
}
static void box(int y,int height) {
    rect(12,y,216,height,MINT);
    rect(12,y,3,3,PAPER); rect(225,y,3,3,PAPER);
    rect(12,y+height-3,3,3,PAPER); rect(225,y+height-3,3,3,PAPER);
    rect(20,y+height/2-3,6,6,INK);
}
void guide_ui_draw(const guide_model_t *m,int battery) {
    lv_obj_t *screen=lv_screen_active(); lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen,lv_color_hex(PAPER),0);
    lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
    rect(0,0,240,39,INK);
    rect(0,285,240,35,INK);
    char s[160];
    const guide_nav_t *n=&m->nav;
    const guide_spot_t *spot=&guide_spots[n->spot<GUIDE_SPOTS?n->spot:0];
    const char *brand="口袋景区导览";
    if(n->view==GUIDE_LIST || n->view==GUIDE_FILTER || n->view==GUIDE_READ || n->view==GUIDE_ACTIONS || n->view==GUIDE_INFO) brand=guide_parks[n->park];
    text(20,17,148,brand,&guide_font_12,PAPER);
    if(battery>=0 && battery<=100) snprintf(s,sizeof(s),"%d%%",battery); else snprintf(s,sizeof(s),"--");
    lv_obj_t *bat=text(173,17,47,s,&guide_font_12,MINT); lv_obj_set_style_text_align(bat,LV_TEXT_ALIGN_RIGHT,0);
    const char *title="选择景区";
    switch(n->view) {
    case GUIDE_LIST: title=n->filter?guide_areas[n->filter]:"景点一览"; break;
    case GUIDE_FILTER: title="选择区域"; break;
    case GUIDE_READ: title=n->page?"换个角度看":spot->name; break;
    case GUIDE_ACTIONS: title="这个景点"; break;
    case GUIDE_INFO: title="资料说明"; break;
    case GUIDE_RECORDS: title="我的足迹"; break;
    case GUIDE_COLLECTION: title=n->collection?"标记看过":"我喜欢的"; break;
    case GUIDE_STATS: title="足迹统计"; break;
    case GUIDE_CLEAR: title="清空导览记录？"; break;
    case GUIDE_HELP: title="出发前看一眼"; break;
    case GUIDE_SOUND: title="声音设置"; break;
    default: break;
    }
    if(n->view!=GUIDE_HOME) text(20,55,n->view==GUIDE_READ&&!n->page?128:200,title,&guide_font_20,INK);
    unsigned count=guide_count(m);
    if(n->view==GUIDE_HOME) {
        unsigned has_continue=m->record.last<GUIDE_SPOTS;
        unsigned park=n->selected==has_continue+1?1:0;
        if(has_continue && n->selected==0) park=guide_spots[m->record.last].park;
        art(0,56,park,0);
        rect(0,39,240,27,INK);
        text(20,41,200,"九寨 · 黄龙",&guide_font_20,PAPER);
        static const char *const labels[]={"九寨沟","黄龙","我的足迹","按键与说明","声音设置"};
        unsigned start=n->selected/4*4;
        for(unsigned row=start;row<count && row<start+4;row++) {
            int y=145+(int)(row-start)*30;
            bool continued=has_continue && !row;
            unsigned item=continued?0:row-has_continue;
            if(row==n->selected) box(y-3,28);
            text(32,y,132,continued?"继续上次阅读":labels[item],&guide_font_16,INK);
            if(!continued && item<2) text(172,y+3,44,item?"8处":"14处",&guide_font_12,INK);
        }
        if(count>4) { snprintf(s,sizeof(s),"%u / %u",n->selected+1,count); text(20,265,200,s,&guide_font_12,MUTED); }
    } else if(n->view==GUIDE_SOUND) {
        snprintf(s,sizeof(s),"%u%%",m->volume); text(20,100,200,s,&guide_font_20,INK);
        rect(20,142,200,12,0xd1d7bb);
        if(m->volume) rect(20,142,2*m->volume,12,MINT);
        text(20,170,200,"上键增加，下键减小\n确定：试听或停止\n长按确定：保存并返回",&guide_font_14,INK);
        snprintf(s,sizeof(s),"试听：%s · %s",guide_parks[spot->park],spot->name); text(20,246,200,s,&guide_font_12,MUTED);
        text(20,267,200,m->volume?"建议从95%开始调整":"已静音",&guide_font_12,MUTED);
    } else if(n->view==GUIDE_READ) {
        snprintf(s,sizeof(s),"%s  ·  %s",n->page?spot->name:guide_areas[spot->area],n->page?"小观察":"景点介绍");
        text(20,88,200,s,&guide_font_12,MUTED);
        if(n->page) {
            art(12,111,n->park,1);
            text(20,194,200,spot->observe,&guide_font_16,INK);
        } else {
            art(156,52,n->park,2);
            text(20,116,200,spot->highlight,&guide_font_14,INK);
            rect(20,143,200,1,0xb6c3aa);
            text(20,153,200,spot->intro,&guide_font_16,INK);
        }
        snprintf(s,sizeof(s),"%s  %s",m->record.seen&(1u<<n->spot)?"已看过":"未标记",m->record.liked&(1u<<n->spot)?"已喜欢":"");
        text(20,265,160,s,&guide_font_12,MUTED);
        snprintf(s,sizeof(s),"%u/2",n->page+1); text(193,265,28,s,&guide_font_12,MUTED);
    } else if(n->view==GUIDE_INFO) {
        static const char *const sources[]={"九寨沟管理局官网\njiuzhai.com","黄龙景区官网\nhuanglong.com","黄龙管理局历史介绍\n中国风景名胜区协会","湖南省林业局历史资料\n黄龙景点名称"};
        text(20,98,200,sources[spot->source],&guide_font_16,INK);
        text(20,258,200,"语音：离线AI合成",&guide_font_12,MUTED);
        text(20,163,200,"离线整理：2026-10-25\n介绍为简述，小观察为游览建议。开放情况以现场公告为准。",&guide_font_14,MUTED);
    } else if(n->view==GUIDE_HELP) {
        text(20,98,200,"无需配网，选择景区即可看。\n上下键：选择或翻页\n确定键：进入或打开更多\n长按确定：返回",&guide_font_16,INK);
        text(20,222,200,"30秒变暗，90秒息屏。\n功能键先点亮，再按操作。\n听介绍：景点页确定进入更多。",&guide_font_12,MUTED);
    } else if(n->view==GUIDE_STATS) {
        snprintf(s,sizeof(s),"看过 %u / 22 个景点\n喜欢 %u 个景点",guide_bits(m->record.seen),guide_bits(m->record.liked));
        text(20,108,200,s,&guide_font_20,INK);
        text(20,198,200,"记录来自手动标记，\n不代表定位或自动打卡。",&guide_font_14,MUTED);
    } else {
        int top=n->view==GUIDE_CLEAR?147:98;
        if(n->view==GUIDE_CLEAR) text(20,93,200,"仅删除本导览的喜欢、看过和继续阅读记录。",&guide_font_14,MUTED);
        if(!count) text(20,110,200,"这里还没有记录。\n阅读景点时按确定，\n可以标记看过或加入喜欢。",&guide_font_16,MUTED);
        unsigned start=n->selected/4*4;
        for(unsigned row=start;row<count && row<start+4;row++) {
            int y=top+(int)(row-start)*40;
            bool selected=row==n->selected;
            if(selected) box(y-5,36);
            text(32,y+3,188,guide_item(m,row),&guide_font_16,INK);
        }
        if(count>4) { snprintf(s,sizeof(s),"%u / %u",n->selected+1,count); text(20,263,200,s,&guide_font_12,MUTED); }
    }
    const char *footer=n->view==GUIDE_HOME?"上下选择 · 确定进入":n->view==GUIDE_READ?"上下翻页 · 确定更多 · 长按返回":"确定选择 · 长按确定返回";
    if(n->view==GUIDE_INFO || n->view==GUIDE_HELP || n->view==GUIDE_STATS) footer="确定或长按确定返回";
    if(m->voice==GUIDE_VOICE_PLAYING) footer="播报中 · 确定停止 · 长按返回";
    if(m->voice==GUIDE_VOICE_STARTING) footer="准备播放 · 确定可取消";
    if(m->voice==GUIDE_VOICE_STOPPING) footer="正在停止播放";
    if(m->voice==GUIDE_VOICE_ERROR) footer="语音暂不可用 · 可重试";
    if(n->view==GUIDE_SOUND) footer=m->voice==GUIDE_VOICE_PLAYING?"试听中 · 上下调音 · 确定停止":"上加 · 下减 · 确定试听";
    if(n->view==GUIDE_SOUND && m->voice==GUIDE_VOICE_ERROR) footer="播放失败 · 确定重试";
    if(n->view==GUIDE_SOUND && m->voice==GUIDE_VOICE_STARTING) footer="准备试听 · 确定取消";
    if(n->view==GUIDE_SOUND && m->voice==GUIDE_VOICE_STOPPING) footer="正在停止试听";
    if(!m->storage_ok) footer="记录暂不能保存 · 仍可浏览";
    if(m->notice) footer=m->notice;
    text(20,290,200,footer,&guide_font_12,PAPER);
}
