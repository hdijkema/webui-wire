#ifndef WEBUI_UTILS_H
#define WEBUI_UTILS_H

#include <functional>

class WebUI_Utils
{
public:
    typedef enum {
        wu_timeout = 0,
        wu_condition_met
    } WaitResult;
public:
    void processCurrentEvents();
    WaitResult waitUntil(std::function<bool ()>, int timeout_ms);

public:
    WebUI_Utils();
};

#endif // WEBUI_UTILS_H
