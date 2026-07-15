#include "../flipper_canbus_app_i.h"

void flipper_canbus_scene_settings_on_enter(void* context) {
    FlipperCanbusApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_reset(submenu);
    submenu_set_header(submenu, "Settings");
    submenu_add_item(submenu, "Classic CAN", 0, NULL, app);
    submenu_add_item(submenu, "500 kbps", 1, NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewSubmenu);
}

bool flipper_canbus_scene_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipper_canbus_scene_settings_on_exit(void* context) {
    FlipperCanbusApp* app = context;
    submenu_reset(app->submenu);
}
