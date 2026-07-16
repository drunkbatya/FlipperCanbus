#include "../flipper_canbus_app_i.h"

typedef enum {
    SubmenuIndexDashboard,
    SubmenuIndexCanMonitor,
    SubmenuIndexSettings,
    SubmenuIndexAbout,
} SubmenuIndex;

static void flipper_canbus_scene_menu_submenu_callback(void* context, uint32_t index) {
    FlipperCanbusApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void flipper_canbus_scene_menu_on_enter(void* context) {
    FlipperCanbusApp* app = context;
    Submenu* submenu = app->submenu;

    flipper_canbus_app_stop_worker(app);

    submenu_reset(submenu);
    submenu_add_item(
        submenu,
        "Dashboard",
        SubmenuIndexDashboard,
        flipper_canbus_scene_menu_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "CAN monitor",
        SubmenuIndexCanMonitor,
        flipper_canbus_scene_menu_submenu_callback,
        app);
    submenu_add_item(
        submenu, "Settings", SubmenuIndexSettings, flipper_canbus_scene_menu_submenu_callback, app);
    submenu_add_item(
        submenu, "About", SubmenuIndexAbout, flipper_canbus_scene_menu_submenu_callback, app);
    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, FlipperCanbusSceneMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewSubmenu);
}

bool flipper_canbus_scene_menu_on_event(void* context, SceneManagerEvent event) {
    FlipperCanbusApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexDashboard) {
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneDashboard);
            consumed = true;
        } else if(event.event == SubmenuIndexCanMonitor) {
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneCanbus);
            consumed = true;
        } else if(event.event == SubmenuIndexSettings) {
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneSettings);
            consumed = true;
        } else if(event.event == SubmenuIndexAbout) {
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneAbout);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, FlipperCanbusSceneMenu, event.event);
    }

    return consumed;
}

void flipper_canbus_scene_menu_on_exit(void* context) {
    FlipperCanbusApp* app = context;
    submenu_reset(app->submenu);
}
