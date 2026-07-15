#include "flipper_canbus_view_canbus.h"

#include <furi.h>

#include <stdlib.h>

struct FlipperCanbusViewCanbus {
    View* view;
};

typedef struct {
    FlipperCanbusFrame frame;
    FuriString* line;
    bool has_frame;
} FlipperCanbusViewCanbusModel;

static void flipper_canbus_view_canbus_draw_callback(Canvas* canvas, void* _model) {
    FlipperCanbusViewCanbusModel* model = _model;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if(!model->has_frame) {
        furi_string_set_str(model->line, "CAN ID not found");
        canvas_draw_str(canvas, 0, 12, furi_string_get_cstr(model->line));
        return;
    }

    furi_string_printf(model->line, "ID 0x%03lX", (unsigned long)model->frame.id);
    canvas_draw_str(canvas, 0, 10, furi_string_get_cstr(model->line));

    canvas_set_font(canvas, FontSecondary);

    furi_string_printf(
        model->line,
        "Count: %lu  Len: %lu",
        (unsigned long)model->frame.count,
        (unsigned long)model->frame.last_len);
    canvas_draw_str(canvas, 0, 24, furi_string_get_cstr(model->line));

    furi_string_set_str(model->line, "Data:");
    for(uint32_t i = 0; i < 8U; i++) {
        if(i < model->frame.last_len) {
            furi_string_cat_printf(model->line, " %02X", model->frame.last_data[i]);
        } else {
            furi_string_cat_str(model->line, " --");
        }
    }
    canvas_draw_str(canvas, 0, 38, furi_string_get_cstr(model->line));
}

FlipperCanbusViewCanbus* flipper_canbus_view_canbus_alloc(void) {
    FlipperCanbusViewCanbus* view_canbus = malloc(sizeof(FlipperCanbusViewCanbus));

    view_canbus->view = view_alloc();
    view_allocate_model(
        view_canbus->view, ViewModelTypeLocking, sizeof(FlipperCanbusViewCanbusModel));
    view_set_draw_callback(view_canbus->view, flipper_canbus_view_canbus_draw_callback);

    with_view_model(
        view_canbus->view,
        FlipperCanbusViewCanbusModel * model,
        {
            model->line = furi_string_alloc();
            model->has_frame = false;
        },
        true);

    return view_canbus;
}

void flipper_canbus_view_canbus_free(FlipperCanbusViewCanbus* view_canbus) {
    furi_assert(view_canbus);
    with_view_model(
        view_canbus->view,
        FlipperCanbusViewCanbusModel * model,
        { furi_string_free(model->line); },
        false);
    view_free(view_canbus->view);
    free(view_canbus);
}

View* flipper_canbus_view_canbus_get_view(FlipperCanbusViewCanbus* view_canbus) {
    furi_assert(view_canbus);
    return view_canbus->view;
}

void flipper_canbus_view_canbus_update(
    FlipperCanbusViewCanbus* view_canbus,
    const FlipperCanbusFrame* frame) {
    with_view_model(
        view_canbus->view,
        FlipperCanbusViewCanbusModel * model,
        {
            if(frame) {
                model->frame = *frame;
                model->has_frame = true;
            } else {
                model->has_frame = false;
            }
        },
        true);
}
