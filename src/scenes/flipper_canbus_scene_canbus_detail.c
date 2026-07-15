#include "../flipper_canbus_app_i.h"

static void flipper_canbus_scene_canbus_detail_update(FlipperCanbusApp* app) {
    FlipperCanbusFrame frame;
    uint32_t selected_can_id =
        scene_manager_get_scene_state(app->scene_manager, FlipperCanbusSceneCanbusDetail);
    if(flipper_canbus_worker_get_frame(app->can_worker, selected_can_id, &frame)) {
        flipper_canbus_view_canbus_update(app->view_canbus, &frame);
    } else {
        flipper_canbus_view_canbus_update(app->view_canbus, NULL);
    }
}

void flipper_canbus_scene_canbus_detail_on_enter(void* context) {
    FlipperCanbusApp* app = context;
    flipper_canbus_scene_canbus_detail_update(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewCanbusDetail);
}

bool flipper_canbus_scene_canbus_detail_on_event(void* context, SceneManagerEvent event) {
    FlipperCanbusApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        flipper_canbus_scene_canbus_detail_update(app);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FlipperCanbusCustomEventWorkerError) {
            scene_manager_set_scene_state(
                app->scene_manager, FlipperCanbusSceneError, FlipperCanbusSceneCanbus);
            scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneError);
            consumed = true;
        }
    }

    return consumed;
}

void flipper_canbus_scene_canbus_detail_on_exit(void* context) {
    UNUSED(context);
}
