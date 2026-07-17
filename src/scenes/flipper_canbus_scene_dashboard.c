#include "../flipper_canbus_app_i.h"

static void flipper_canbus_scene_dashboard_update(FlipperCanbusApp* app) {
    RusEfiCanDecoded decoded = {0};
    rusefi_can_decoder_decode(app->can_worker, &decoded);
    flipper_canbus_view_dashboard_update(app->view_dashboard, &decoded);
}

void flipper_canbus_scene_dashboard_on_enter(void* context) {
    FlipperCanbusApp* app = context;

    flipper_canbus_scene_dashboard_update(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewDashboardScreen);
    flipper_canbus_app_start_worker(app);
}

bool flipper_canbus_scene_dashboard_on_event(void* context, SceneManagerEvent event) {
    FlipperCanbusApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        if(flipper_canbus_app_is_worker_error_pending(app)) {
            flipper_canbus_app_show_worker_error(app, FlipperCanbusSceneDashboard);
        } else {
            flipper_canbus_scene_dashboard_update(app);
        }
        consumed = true;
    }

    return consumed;
}

void flipper_canbus_scene_dashboard_on_exit(void* context) {
    UNUSED(context);
}
