#include "webui_utils.h"

#include <thread>

#ifdef __linux
#include <gtk/gtk.h>
#endif

static void initGUI()
{
#ifdef __linux
    static bool initialized = false;
    static int argc = 0;
    static const char *argv[] = { NULL };
    if (!initialized) {
        char **av = (char **) argv;
        gtk_init(&argc, &av);
        initialized = true;
    }
#endif
}

void WebUI_Utils::processCurrentEvents()
{
#ifdef __linux
    else {  // Idle processing, process Gtk events.
        while (gtk_events_pending()) {
            gtk_main_iteration_do(0);
        }
    }
#endif
#ifdef __APPLE__
    else {
        process_events_apple();
    }
#endif
}

WebUI_Utils::WaitResult WebUI_Utils::waitUntil(std::function<bool ()> condition_f, int timeout_ms)
{
    int tick = 0;
    int timeout_ticks = timeout_ms;
    while(!condition_f() && tick < timeout_ticks) {
        processCurrentEvents();
        std::chrono::milliseconds d = std::chrono::milliseconds(1);
        std::this_thread::sleep_for(d);
        tick += 1;
    }

    if (tick >= timeout_ticks) {
        return WaitResult::wu_timeout;
    }

    return wu_condition_met;
}

WebUI_Utils::WebUI_Utils()
{
    initGUI();
}
