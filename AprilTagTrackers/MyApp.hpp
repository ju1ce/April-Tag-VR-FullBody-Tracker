#pragma once

#include "Config.hpp"
#include "Connection.hpp"
#include "Debug.hpp"
#include "GUI.hpp"
#include "Localization.hpp"
#include "Tracker.hpp"

#include <wx/app.h>

class MyApp : public wxApp
{
    std::unique_ptr<Tracker> tracker;
    std::unique_ptr<GUI> gui;

    UserConfig userConfig;
    CalibrationConfig calibConfig;
    ArucoConfig arucoConfig;
    Localization lc;

public:
    int OnExit() override;
    bool OnInit() override;

#ifdef ATT_OVERRIDE_ERROR_HANDLERS
    void OnFatalException() override;
    void OnUnhandledException() override;
    bool OnExceptionInMainLoop() override;
#endif
};
