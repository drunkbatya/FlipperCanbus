#pragma once

#include "scenes/flipper_canbus_scene.h"
#include "views/flipper_canbus_view_dashboard.h"
#include "views/flipper_canbus_view_canbus.h"
#include "workers/flipper_canbus_worker.h"

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    Submenu* submenu;
    DialogEx* dialog_ex;
    FlipperCanbusViewDashboard* view_dashboard;
    FlipperCanbusViewCanbus* view_canbus;

    FlipperCanbusWorker* can_worker;
    FlipperCanbusFrame* can_frames;
    FuriString* text;
} FlipperCanbusApp;

typedef enum {
    FlipperCanbusViewSubmenu,
    FlipperCanbusViewDashboardScreen,
    FlipperCanbusViewCanbusDetail,
    FlipperCanbusViewDialogEx,
} FlipperCanbusView;

typedef enum {
    FlipperCanbusCustomEventCanIdSelected,
    FlipperCanbusCustomEventWorkerError,
    FlipperCanbusCustomEventErrorDialogDone,
} FlipperCanbusCustomEvent;

void flipper_canbus_app_start_worker(FlipperCanbusApp* app);
void flipper_canbus_app_stop_worker(FlipperCanbusApp* app);
