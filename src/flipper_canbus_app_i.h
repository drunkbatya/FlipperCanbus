#pragma once

#include "scenes/flipper_canbus_scene.h"
#include "views/flipper_canbus_view_dashboard.h"
#include "views/flipper_canbus_view_canbus.h"
#include <workers/flipper_canbus_worker.h>

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    Submenu* submenu;
    DialogEx* dialog_ex;
    Widget* widget;
    FlipperCanbusViewDashboard* view_dashboard;
    FlipperCanbusViewCanbus* view_canbus;

    FlipperCanbusWorker* can_worker;
    FlipperCanbusFrame* can_frames;
    uint32_t canbus_id_count;
    FuriString* text;
    volatile FlipperCanbusWorkerErrorResult worker_error;
    const char* volatile driver_error;
    volatile bool worker_error_pending;
} FlipperCanbusApp;

typedef enum {
    FlipperCanbusViewSubmenu,
    FlipperCanbusViewDashboardScreen,
    FlipperCanbusViewCanbusDetail,
    FlipperCanbusViewDialogEx,
    FlipperCanbusViewWidget,
} FlipperCanbusView;

typedef enum {
    FlipperCanbusCustomEventCanIdSelected,
    FlipperCanbusCustomEventErrorDialogDone,
} FlipperCanbusCustomEvent;

void flipper_canbus_app_start_worker(FlipperCanbusApp* app);
void flipper_canbus_app_stop_worker(FlipperCanbusApp* app);
bool flipper_canbus_app_is_worker_error_pending(FlipperCanbusApp* app);
void flipper_canbus_app_show_worker_error(FlipperCanbusApp* app, FlipperCanbusScene retry_scene);
