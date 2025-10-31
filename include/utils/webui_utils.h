#ifndef WEBUI_UTILS_H
#define WEBUI_UTILS_H

#include <functional>
#include <string>

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
    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief encodeUrl - encodes a *possible* Url as needed by the url spec.
    /// \param maybe_url
    /// \return
    ////////////////////////////////////////////////////////////////////////////////////
    std::string encodeUrl(const std::string &maybe_url);

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief WebUI_Utils::checkUrl - checks if the given string is a valid url
    /// \param maybe_url
    /// \return
    ////////////////////////////////////////////////////////////////////////////////////
    bool checkUrl(const std::string &maybe_url);

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief WebUI_Utils::normalizeUrl - precondition: checkUrl == true
    /// Normalizes the given Url to a Url that can be used
    ///
    /// \param maybe_url
    /// \return
    ////////////////////////////////////////////////////////////////////////////////////

    std::string normalizeUrl(const std::string &checked_url);

public:
    WebUI_Utils();
};

#endif // WEBUI_UTILS_H
