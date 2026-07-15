#include "../flipper_canbus_app_i.h"

static void flipper_canbus_scene_error_dialog_callback(DialogExResult result, void* context) {
    if(result != DialogExResultCenter) return;

    FlipperCanbusApp* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, FlipperCanbusCustomEventErrorDialogDone);
}

void flipper_canbus_scene_error_on_enter(void* context) {
    FlipperCanbusApp* app = context;

    flipper_canbus_app_stop_worker(app);
    flipper_canbus_worker_format_last_error(app->can_worker, app->text);

    DialogEx* dialog_ex = app->dialog_ex;
    dialog_ex_reset(dialog_ex);
    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, flipper_canbus_scene_error_dialog_callback);
    dialog_ex_set_header(dialog_ex, "CAN error", 64, 4, AlignCenter, AlignTop);
    dialog_ex_set_text(
        dialog_ex, furi_string_get_cstr(app->text), 64, 24, AlignCenter, AlignCenter);
    dialog_ex_set_center_button_text(dialog_ex, "Retry");

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewDialogEx);
}

bool flipper_canbus_scene_error_on_event(void* context, SceneManagerEvent event) {
    FlipperCanbusApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FlipperCanbusCustomEventErrorDialogDone) {
            uint32_t retry_scene =
                scene_manager_get_scene_state(app->scene_manager, FlipperCanbusSceneError);
            if(retry_scene == FlipperCanbusSceneCanbus) {
                scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, FlipperCanbusSceneCanbus);
            } else if(retry_scene == FlipperCanbusSceneDashboard) {
                scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, FlipperCanbusSceneDashboard);
            } else {
                scene_manager_previous_scene(app->scene_manager);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, FlipperCanbusSceneMenu);
        consumed = true;
    }

    return consumed;
}

void flipper_canbus_scene_error_on_exit(void* context) {
    FlipperCanbusApp* app = context;
    dialog_ex_reset(app->dialog_ex);
}
