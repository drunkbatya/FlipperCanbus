#pragma once

#include <gui/scene_manager.h>

#define ADD_SCENE(prefix, name, id) FlipperCanbusScene##id,
typedef enum {
#include "flipper_canbus_scene_config.h"
    FlipperCanbusSceneNum,
} FlipperCanbusScene;
#undef ADD_SCENE

extern const SceneManagerHandlers flipper_canbus_scene_handlers;

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "flipper_canbus_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "flipper_canbus_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "flipper_canbus_scene_config.h"
#undef ADD_SCENE
