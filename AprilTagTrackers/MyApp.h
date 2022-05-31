#pragma once

#include "Config.h"
#include "Connection.h"
#include "Debug.h"
#include "GUI.h"
#include "Localization.h"
#include "Tracker.h"

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
