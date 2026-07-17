#include "../flipper_canbus_app_i.h"

static void flipper_canbus_scene_canbus_submenu_callback(void* context, uint32_t index);

static void
    flipper_canbus_scene_canbus_rebuild_submenu(FlipperCanbusApp* app, uint32_t selected_id) {
    size_t can_frames_count = flipper_canbus_worker_copy_snapshot(
        app->can_worker, app->can_frames, FLIPPER_CANBUS_MAX_IDS);

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "CAN messages");

    if(can_frames_count == 0) {
        submenu_add_item(app->submenu, "No messages", 0, NULL, app);
        submenu_set_selected_item(app->submenu, 0);
        return;
    }

    for(size_t i = 0; i < can_frames_count; i++) {
        furi_string_printf(app->text, "0x%03lX", app->can_frames[i].id);
        submenu_add_item(
            app->submenu,
            furi_string_get_cstr(app->text),
            app->can_frames[i].id,
            flipper_canbus_scene_canbus_submenu_callback,
            app);
    }

    submenu_set_selected_item(app->submenu, selected_id);
}

static void flipper_canbus_scene_canbus_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    FlipperCanbusApp* app = context;
    scene_manager_set_scene_state(app->scene_manager, FlipperCanbusSceneCanbusDetail, index);
    view_dispatcher_send_custom_event(app->view_dispatcher, FlipperCanbusCustomEventCanIdSelected);
}

bool flipper_canbus_scene_canbus_on_event(void* context, SceneManagerEvent event) {
    FlipperCanbusApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FlipperCanbusCustomEventCanIdSelected) {
            scene_manager_set_scene_state(
                app->scene_manager,
                FlipperCanbusSceneCanbus,
                scene_manager_get_scene_state(app->scene_manager, FlipperCanbusSceneCanbusDetail));
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneCanbusDetail);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(flipper_canbus_app_is_worker_error_pending(app)) {
            flipper_canbus_app_show_worker_error(app, FlipperCanbusSceneCanbus);
        } else {
            uint32_t id_count = flipper_canbus_worker_get_count(app->can_worker);
            if(id_count != app->canbus_id_count) {
                app->canbus_id_count = id_count;
                flipper_canbus_scene_canbus_rebuild_submenu(
                    app, submenu_get_selected_item(app->submenu));
            }
        }
        consumed = true;
    }
    return consumed;
}

void flipper_canbus_scene_canbus_on_enter(void* context) {
    FlipperCanbusApp* app = context;

    flipper_canbus_scene_canbus_rebuild_submenu(
        app, scene_manager_get_scene_state(app->scene_manager, FlipperCanbusSceneCanbus));
    app->canbus_id_count = flipper_canbus_worker_get_count(app->can_worker);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewSubmenu);
    flipper_canbus_app_start_worker(app);
}

void flipper_canbus_scene_canbus_on_exit(void* context) {
    FlipperCanbusApp* app = context;
    scene_manager_set_scene_state(
        app->scene_manager, FlipperCanbusSceneCanbus, submenu_get_selected_item(app->submenu));
    submenu_reset(app->submenu);
}
