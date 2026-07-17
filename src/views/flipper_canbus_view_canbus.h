#pragma once

#include <gui/view.h>
#include <workers/flipper_canbus_worker.h>

typedef struct FlipperCanbusViewCanbus FlipperCanbusViewCanbus;

FlipperCanbusViewCanbus* flipper_canbus_view_canbus_alloc(void);
void flipper_canbus_view_canbus_free(FlipperCanbusViewCanbus* view_canbus);
View* flipper_canbus_view_canbus_get_view(FlipperCanbusViewCanbus* view_canbus);
void flipper_canbus_view_canbus_update(
    FlipperCanbusViewCanbus* view_canbus,
    const FlipperCanbusFrame* frame);
