#include "../flipper_canbus_app_i.h"

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const flipper_canbus_scene_on_enter_handlers[])(void*) = {
#include "flipper_canbus_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const flipper_canbus_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "flipper_canbus_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const flipper_canbus_scene_on_exit_handlers[])(void* context) = {
#include "flipper_canbus_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers flipper_canbus_scene_handlers = {
    .on_enter_handlers = flipper_canbus_scene_on_enter_handlers,
    .on_event_handlers = flipper_canbus_scene_on_event_handlers,
    .on_exit_handlers = flipper_canbus_scene_on_exit_handlers,
    .scene_num = FlipperCanbusSceneNum,
};
