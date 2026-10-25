#include "guide_model.h"
#include <string.h>
unsigned guide_bits(uint32_t bits) { unsigned n=0; while(bits) { n+=bits&1u; bits>>=1; } return n; }
void guide_init(guide_model_t *m, guide_record_t r) { memset(m,0,sizeof(*m)); m->record=r; m->storage_ok=true; m->volume=95; }
static void push(guide_model_t *m, guide_view_t v) {
    if(m->depth>=8) return;
    if(m->nav.view==GUIDE_HOME) m->nav.home_continue=m->record.last<GUIDE_SPOTS;
    m->stack[m->depth++]=m->nav;
    m->nav.view=v; m->nav.selected=0; m->nav.page=0;
}
static void back(guide_model_t *m) {
    if(m->depth) m->nav=m->stack[--m->depth];
    if(m->nav.view==GUIDE_HOME) {
        bool continued=m->record.last<GUIDE_SPOTS;
        if(continued && !m->nav.home_continue) m->nav.selected++;
        else if(!continued && m->nav.home_continue && m->nav.selected) m->nav.selected--;
        m->nav.home_continue=continued;
    }
    unsigned n=guide_count(m);
    if(m->nav.selected>=n) m->nav.selected=n?(uint8_t)(n-1):0;
}
uint8_t guide_list_id(const guide_model_t *m, unsigned row) {
    unsigned found=0;
    for(uint8_t i=0;i<GUIDE_SPOTS;i++) {
        bool match=m->nav.view==GUIDE_COLLECTION ? ((m->nav.collection ? m->record.seen:m->record.liked)&(1u<<i))!=0 :
            guide_spots[i].park==m->nav.park && (!m->nav.filter || guide_spots[i].area==m->nav.filter);
        if(match && found++==row) return i;
    }
    return GUIDE_NONE;
}
unsigned guide_count(const guide_model_t *m) {
    switch(m->nav.view) {
    case GUIDE_HOME: return m->record.last<GUIDE_SPOTS ? 6:5;
    case GUIDE_LIST: { unsigned n=0; while(guide_list_id(m,n)!=GUIDE_NONE) n++; return n+(m->nav.park==0); }
    case GUIDE_FILTER: return 5;
    case GUIDE_ACTIONS: return 6;
    case GUIDE_RECORDS: return 4;
    case GUIDE_COLLECTION: { unsigned n=0; while(guide_list_id(m,n)!=GUIDE_NONE) n++; return n; }
    case GUIDE_CLEAR: return 2;
    default: return 0;
    }
}
const char *guide_item(const guide_model_t *m, unsigned row) {
    static const char *const home[]={"九寨沟 · 14 个景点","黄龙 · 8 个景点","我的足迹","按键与说明","声音设置"};
    static const char *const records[]={"我喜欢的","标记看过","足迹统计","清空导览记录"};
    if(m->nav.view==GUIDE_HOME) { if(m->record.last<GUIDE_SPOTS) { if(!row) return "继续上次阅读"; row--; } return row<5?home[row]:""; }
    if(m->nav.view==GUIDE_FILTER) return row<5?guide_areas[row]:"";
    if(m->nav.view==GUIDE_RECORDS) return row<4?records[row]:"";
    if(m->nav.view==GUIDE_CLEAR) return row?"确认清空":"取消，保留记录";
    if(m->nav.view==GUIDE_ACTIONS) {
        if(row==0) return m->voice==GUIDE_VOICE_PLAYING || m->voice==GUIDE_VOICE_STARTING ? "停止播放":"听介绍";
        if(row==1) return m->record.seen&(1u<<m->nav.spot)?"撤销看过":"标记看过";
        if(row==2) return m->record.liked&(1u<<m->nav.spot)?"取消喜欢":"加入喜欢";
        return row==3?"资料说明":row==4?"结束阅读":"声音设置";
    }
    if(m->nav.view==GUIDE_LIST && !m->nav.park) { if(!row) return "按区域筛选"; row--; }
    uint8_t id=guide_list_id(m,row); return id<GUIDE_SPOTS?guide_spots[id].name:"";
}
static bool open_spot(guide_model_t *m, uint8_t id) {
    if(id>=GUIDE_SPOTS) return false;
    push(m,GUIDE_READ); m->nav.spot=id; m->nav.park=guide_spots[id].park;
    bool changed=m->record.last!=id; m->record.last=id; return changed;
}
static bool key_impl(guide_model_t *m, int key) {
    m->notice=NULL;
    if(m->nav.view==GUIDE_SOUND) {
        if(key==2) { m->save_volume=true; back(m); return false; }
        if(key==-1 || key==1) {
            int next=(int)m->volume+(key==-1?5:-5); if(next<0) next=0; if(next>100) next=100;
            if(next!=m->volume) { m->volume=(uint8_t)next; }
            if(!m->volume && m->voice!=GUIDE_VOICE_OFF) { m->command=GUIDE_COMMAND_STOP; m->voice=GUIDE_VOICE_STOPPING; }
        } else if(key==0) {
            bool active=m->voice==GUIDE_VOICE_PLAYING || m->voice==GUIDE_VOICE_STARTING;
            if(active) { m->command=GUIDE_COMMAND_STOP; m->voice=GUIDE_VOICE_STOPPING; }
            else if(m->volume) { m->command=GUIDE_COMMAND_PLAY; m->voice=GUIDE_VOICE_STARTING; }
            else m->notice="已静音，上键增加音量";
        }
        return false;
    }
    if(key==2) { back(m); return false; }
    unsigned n=guide_count(m);
    if(key==-1 || key==1) {
        if(m->nav.view==GUIDE_READ) m->nav.page^=1;
        else if(n) m->nav.selected=(uint8_t)((m->nav.selected+n+key)%n);
        return false;
    }
    if(key!=0) return false;
    unsigned row=m->nav.selected;
    switch(m->nav.view) {
    case GUIDE_HOME:
        if(m->record.last<GUIDE_SPOTS) { if(!row) return open_spot(m,m->record.last); row--; }
        if(row<2) { push(m,GUIDE_LIST); m->nav.park=(uint8_t)row; m->nav.filter=0; }
        else if(row==4) { push(m,GUIDE_SOUND); m->nav.spot=m->record.last<GUIDE_SPOTS?m->record.last:0; }
        else push(m,row==2?GUIDE_RECORDS:GUIDE_HELP);
        break;
    case GUIDE_LIST:
        if(!m->nav.park) { if(!row) { push(m,GUIDE_FILTER); break; } row--; }
        return open_spot(m,guide_list_id(m,row));
    case GUIDE_FILTER: back(m); m->nav.filter=(uint8_t)row; m->nav.selected=0; break;
    case GUIDE_READ:
        if(m->voice==GUIDE_VOICE_PLAYING || m->voice==GUIDE_VOICE_STARTING) { m->command=GUIDE_COMMAND_STOP; m->voice=GUIDE_VOICE_STOPPING; }
        else push(m,GUIDE_ACTIONS);
        break;
    case GUIDE_ACTIONS:
        if(row==0) {
            if(!m->volume) { m->notice="已静音，请调整音量"; break; }
            bool active=m->voice==GUIDE_VOICE_PLAYING || m->voice==GUIDE_VOICE_STARTING;
            m->command=active?GUIDE_COMMAND_STOP:GUIDE_COMMAND_PLAY;
            m->voice=active?GUIDE_VOICE_STOPPING:GUIDE_VOICE_STARTING; back(m); break;
        }
        if(row<3) {
            uint32_t bit=1u<<m->nav.spot;
            if(row==1) m->record.seen^=bit; else m->record.liked^=bit;
            back(m); m->notice="已保存"; return true;
        }
        if(row==3) push(m,GUIDE_INFO);
        else if(row==4) { back(m); back(m); }
        else push(m,GUIDE_SOUND);
        break;
    case GUIDE_RECORDS:
        if(row<2) { push(m,GUIDE_COLLECTION); m->nav.collection=(uint8_t)row; }
        else push(m,row==2?GUIDE_STATS:GUIDE_CLEAR);
        break;
    case GUIDE_COLLECTION: return open_spot(m,guide_list_id(m,row));
    case GUIDE_CLEAR:
        back(m);
        if(row) { m->record=(guide_record_t){.sequence=m->record.sequence,.last=GUIDE_NONE}; m->notice="导览记录已清空"; return true; }
        break;
    default: back(m); break;
    }
    return false;
}

bool guide_key(guide_model_t *m,int key) {
    uint8_t spot=m->nav.spot;
    guide_view_t previous=m->nav.view;
    m->save_volume=false;
    m->command=GUIDE_COMMAND_NONE;
    bool save=key_impl(m,key);
    bool in_read=m->nav.view==GUIDE_READ || m->nav.view==GUIDE_ACTIONS || m->nav.view==GUIDE_INFO || m->nav.view==GUIDE_SOUND;
    if(m->voice!=GUIDE_VOICE_OFF && (m->nav.spot!=spot || !in_read || (previous==GUIDE_SOUND && m->nav.view!=GUIDE_SOUND))) {
        m->command=GUIDE_COMMAND_STOP; m->voice=GUIDE_VOICE_STOPPING;
    }
    return save;
}
