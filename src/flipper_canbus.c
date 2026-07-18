#include "flipper_canbus_app_i.h"

#include <furi_hal_power.h>

#include <stdlib.h>

static void flipper_canbus_worker_error_callback(
    void* context,
    FlipperCanbusWorkerErrorResult error,
    const char* driver_error);

static bool flipper_canbus_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    FlipperCanbusApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool flipper_canbus_app_back_event_callback(void* context) {
    furi_assert(context);
    FlipperCanbusApp* app = context;
    if(scene_manager_handle_back_event(app->scene_manager)) return true;

    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_NONE);
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

static void flipper_canbus_app_tick_event_callback(void* context) {
    furi_assert(context);
    FlipperCanbusApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static FlipperCanbusApp* flipper_canbus_app_alloc(void) {
    FlipperCanbusApp* app = malloc(sizeof(FlipperCanbusApp));

    furi_hal_power_enable_otg();
    furi_delay_ms(100);

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&flipper_canbus_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, flipper_canbus_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, flipper_canbus_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, flipper_canbus_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipperCanbusViewSubmenu, submenu_get_view(app->submenu));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipperCanbusViewDialogEx, dialog_ex_get_view(app->dialog_ex));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipperCanbusViewWidget, widget_get_view(app->widget));

    app->view_dashboard = flipper_canbus_view_dashboard_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipperCanbusViewDashboardScreen,
        flipper_canbus_view_dashboard_get_view(app->view_dashboard));

    app->view_canbus = flipper_canbus_view_canbus_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipperCanbusViewCanbusDetail,
        flipper_canbus_view_canbus_get_view(app->view_canbus));

    app->can_worker = flipper_canbus_worker_alloc();
    flipper_canbus_worker_set_error_callback(
        app->can_worker, flipper_canbus_worker_error_callback, app);
    app->can_frames = malloc(sizeof(FlipperCanbusFrame) * FLIPPER_CANBUS_MAX_IDS);
    app->text = furi_string_alloc();

    scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneMenu);

    return app;
}

static void flipper_canbus_app_free(FlipperCanbusApp* app) {
    furi_assert(app);

    flipper_canbus_app_stop_worker(app);
    flipper_canbus_worker_free(app->can_worker);

    view_dispatcher_remove_view(app->view_dispatcher, FlipperCanbusViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, FlipperCanbusViewDashboardScreen);
    view_dispatcher_remove_view(app->view_dispatcher, FlipperCanbusViewCanbusDetail);
    view_dispatcher_remove_view(app->view_dispatcher, FlipperCanbusViewDialogEx);
    view_dispatcher_remove_view(app->view_dispatcher, FlipperCanbusViewWidget);

    submenu_free(app->submenu);
    dialog_ex_free(app->dialog_ex);
    widget_free(app->widget);
    flipper_canbus_view_dashboard_free(app->view_dashboard);
    flipper_canbus_view_canbus_free(app->view_canbus);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_string_free(app->text);
    free(app->can_frames);

    furi_record_close(RECORD_GUI);
    furi_hal_power_disable_otg();

    free(app);
}

static void flipper_canbus_worker_error_callback(
    void* context,
    FlipperCanbusWorkerErrorResult error,
    const char* driver_error) {
    FlipperCanbusApp* app = context;
    app->worker_error = error;
    app->driver_error = driver_error;
    app->worker_error_pending = true;
}

void flipper_canbus_app_start_worker(FlipperCanbusApp* app) {
    furi_assert(app);
    app->worker_error_pending = false;
    app->driver_error = NULL;
    flipper_canbus_worker_start(app->can_worker);
}

void flipper_canbus_app_stop_worker(FlipperCanbusApp* app) {
    furi_assert(app);
    flipper_canbus_worker_send_stop(app->can_worker);
    flipper_canbus_worker_await_stop(app->can_worker);
}

bool flipper_canbus_app_is_worker_error_pending(FlipperCanbusApp* app) {
    furi_assert(app);
    return app->worker_error_pending;
}

void flipper_canbus_app_show_worker_error(FlipperCanbusApp* app, FlipperCanbusScene retry_scene) {
    furi_assert(app);
    app->worker_error_pending = false;
    scene_manager_set_scene_state(app->scene_manager, FlipperCanbusSceneError, retry_scene);
    scene_manager_next_scene(app->scene_manager, FlipperCanbusSceneError);
}

int32_t flipper_canbus_app(void* p) {
    UNUSED(p);

    FlipperCanbusApp* app = flipper_canbus_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    flipper_canbus_app_free(app);

    return 0;
}
