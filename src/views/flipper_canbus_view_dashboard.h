#pragma once

#include <decoders/rusefi_can_decoder.h>

#include <gui/view.h>

typedef struct FlipperCanbusViewDashboard FlipperCanbusViewDashboard;

FlipperCanbusViewDashboard* flipper_canbus_view_dashboard_alloc(void);
void flipper_canbus_view_dashboard_free(FlipperCanbusViewDashboard* view_dashboard);
View* flipper_canbus_view_dashboard_get_view(FlipperCanbusViewDashboard* view_dashboard);
void flipper_canbus_view_dashboard_update(
    FlipperCanbusViewDashboard* view_dashboard,
    const RusEfiCanDecoded* decoded);
