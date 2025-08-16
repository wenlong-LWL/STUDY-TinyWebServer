#include "log/log.h"
#include <iostream>
#include <unistd.h>


int main() {
    Log::get_instance()->init("./serverlogs/log", false, true, 200);
    bool m_close_log = false;
    for (int i = 0; i < 5; ++i) {
        LOG_INFO("nihao");
    }
    for (int i = 0; i < 5; ++i) {
        LOG_DEBUG("wohao");
    }
    sleep(5);
}