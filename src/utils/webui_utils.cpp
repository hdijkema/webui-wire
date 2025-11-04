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
    // Idle processing, process Gtk events.
    while (gtk_events_pending()) {
        gtk_main_iteration_do(0);
    }
#endif
#ifdef __APPLE__
    process_events_apple();
#endif
}

std::string WebUI_Utils::encodeUrl(const std::string &maybe_url)
{
    const char *hex = "0123456789ABCDEF";
    std::string encodedMsg = "";

    const char *msg = maybe_url.c_str();
    while (*msg != '\0') {
        if (
            ('a' <= *msg && *msg <= 'z') ||
            ('A' <= *msg && *msg <= 'Z') ||
            ('0' <= *msg && *msg <= '9') ||
            *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[(unsigned char)*msg >> 4];
            encodedMsg += hex[*msg & 0xf];
        }
        msg++;
    }
    return encodedMsg;
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

///////////////////////////////////////////////////////////////////////////////////////
// Url Utils
///////////////////////////////////////////////////////////////////////////////////////
#include "deps/CxxUrl/url.hpp"

bool WebUI_Utils::checkUrl(const std::string &maybe_url)
{
    try {
        Url u(maybe_url);
        u.str();
        return true;
    } catch (...) {
        return false;
    }
}

std::string WebUI_Utils::normalizeUrl(const std::string &checked_url)
{
    Url u(checked_url);
    return u.str();
}

