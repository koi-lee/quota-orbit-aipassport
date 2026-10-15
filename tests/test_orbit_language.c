#include "orbit_language.h"
#include <assert.h>
#include <string.h>

int main(void) {
    orbit_language_set(ORBIT_ZH_CN);
    assert(!strcmp(orbit_tr("设置"), "设置"));
    orbit_language_set(ORBIT_ZH_HANT);
    assert(!strcmp(orbit_tr("设置"), "設定"));
    assert(!strcmp(orbit_tr("网络条件不兼容"), "網路條件不相容"));
    assert(!strcmp(orbit_tr("额度星盘"), "額度星盤"));
    orbit_language_set(ORBIT_EN);
    assert(!strcmp(orbit_tr("设置"), "Settings"));
    assert(!strcmp(orbit_tr("尚未收到完成公告"), "Not completed yet"));
    assert(!strcmp(orbit_tr("重新配网？"), "Start Wi-Fi setup?"));
    assert(!strcmp(orbit_tr("取消"), "Cancel"));
    assert(!strcmp(orbit_tr("确认"), "Confirm"));
    assert(!strcmp(orbit_tr("未登记的动态文本"), "未登记的动态文本"));
    orbit_language_set(99);
    assert(orbit_language_get() == ORBIT_ZH_CN);
    assert(!strcmp(orbit_tr("设置"), "设置"));
    return 0;
}
