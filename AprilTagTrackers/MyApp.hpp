#pragma once

#include "Config.hpp"
#include "Connection.hpp"
#include "GUI.hpp"
#include "Localization.hpp"
#include "Tracker.hpp"
#include "utils/Assert.hpp"

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

#ifndef ATT_DEBUG
    void OnFatalException() override;
    void OnUnhandledException() override;
    bool OnExceptionInMainLoop() override;
#endif
};
