#include "../flipper_canbus_app_i.h"

#define FLIPPER_CANBUS_VERSION_APP FAP_VERSION
#define FLIPPER_CANBUS_DEVELOPER   "drunkbatya"
#define FLIPPER_CANBUS_GITHUB      "https://github.com/drunkbatya/FlipperCanbus"
#define FLIPPER_CANBUS_NAME        "\e#\e!        FlipperCanbus        \e!\n"
#define FLIPPER_CANBUS_BLANK_INV \
    "\e#\e!                                                      \e!\n"

void flipper_canbus_scene_about_on_enter(void* context) {
    FlipperCanbusApp* app = context;
    FuriString* tmp_string = furi_string_alloc();

    widget_add_text_box_element(
        app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom, FLIPPER_CANBUS_BLANK_INV, false);
    widget_add_text_box_element(
        app->widget, 0, 2, 128, 14, AlignCenter, AlignBottom, FLIPPER_CANBUS_NAME, false);
    furi_string_printf(tmp_string, "\e#%s\n", "Information");
    furi_string_cat_printf(tmp_string, "Version: %s\n", FLIPPER_CANBUS_VERSION_APP);
    furi_string_cat_printf(tmp_string, "Developed by: %s\n", FLIPPER_CANBUS_DEVELOPER);
    furi_string_cat_printf(tmp_string, "Github: %s\n\n", FLIPPER_CANBUS_GITHUB);
    furi_string_cat_printf(tmp_string, "\e#%s\n", "Description");
    furi_string_cat_printf(
        tmp_string,
        "CAN monitor and dashboard.\n"
        "For Flipper CAN module with MCP2518FD.\n\n");
    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(tmp_string));

    furi_string_free(tmp_string);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperCanbusViewWidget);
}

bool flipper_canbus_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipper_canbus_scene_about_on_exit(void* context) {
    FlipperCanbusApp* app = context;
    widget_reset(app->widget);
}
