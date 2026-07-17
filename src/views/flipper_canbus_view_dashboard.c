#include "flipper_canbus_view_dashboard.h"

#include <furi.h>
#include <gui/canvas.h>
#include <input/input.h>

#include <stdlib.h>
#include <string.h>

#define FLIPPER_CANBUS_DASHBOARD_LABEL_X 0U
#define FLIPPER_CANBUS_DASHBOARD_VALUE_X 127U
#define FLIPPER_CANBUS_DASHBOARD_ROWS    5U

typedef enum {
    FlipperCanbusDashboardFormatInt,
    FlipperCanbusDashboardFormatBool,
    FlipperCanbusDashboardFormatFixed1,
    FlipperCanbusDashboardFormatFixed2,
    FlipperCanbusDashboardFormatFixed3,
    FlipperCanbusDashboardFormatFixed4,
} FlipperCanbusDashboardFormat;

typedef enum {
    FlipperCanbusDashboardValueRpm,
    FlipperCanbusDashboardValueMap,
    FlipperCanbusDashboardValueIgnition,
    FlipperCanbusDashboardValueInjPw,
    FlipperCanbusDashboardValueBatt,
    FlipperCanbusDashboardValueInjDuty,
    FlipperCanbusDashboardValueIgnDuty,
    FlipperCanbusDashboardValueSpeed,
    FlipperCanbusDashboardValueGear,
    FlipperCanbusDashboardValueFlex,
    FlipperCanbusDashboardValuePps,
    FlipperCanbusDashboardValueTps1,
    FlipperCanbusDashboardValueTps2,
    FlipperCanbusDashboardValueWastegate,
    FlipperCanbusDashboardValueCylAir,
    FlipperCanbusDashboardValueMaf,
    FlipperCanbusDashboardValueKnockCount,
    FlipperCanbusDashboardValueKnock0,
    FlipperCanbusDashboardValueKnock1,
    FlipperCanbusDashboardValueKnock2,
    FlipperCanbusDashboardValueCoolant,
    FlipperCanbusDashboardValueIntake,
    FlipperCanbusDashboardValueOilTemp,
    FlipperCanbusDashboardValueFuelTemp,
    FlipperCanbusDashboardValueMcuTemp,
    FlipperCanbusDashboardValueAux1,
    FlipperCanbusDashboardValueAux2,
    FlipperCanbusDashboardValueFuelLevel,
    FlipperCanbusDashboardValueFuelUsed,
    FlipperCanbusDashboardValueFuelFlow,
    FlipperCanbusDashboardValueLambda1,
    FlipperCanbusDashboardValueLambda2,
    FlipperCanbusDashboardValueFuelTrim1,
    FlipperCanbusDashboardValueFuelTrim2,
    FlipperCanbusDashboardValueOilPress,
    FlipperCanbusDashboardValueFpLow,
    FlipperCanbusDashboardValueFpHigh,
    FlipperCanbusDashboardValueWarnings,
    FlipperCanbusDashboardValueLastError,
    FlipperCanbusDashboardValueDistance,
    FlipperCanbusDashboardValueMainRelay,
    FlipperCanbusDashboardValueFuelPump,
    FlipperCanbusDashboardValueCel,
    FlipperCanbusDashboardValueRevLimiter,
    FlipperCanbusDashboardValueFan1,
    FlipperCanbusDashboardValueFan2,
    FlipperCanbusDashboardValueEgoHeat,
    FlipperCanbusDashboardValueLambdaProtect,
    FlipperCanbusDashboardValueCam1I,
    FlipperCanbusDashboardValueCam1ITarget,
    FlipperCanbusDashboardValueCam1E,
    FlipperCanbusDashboardValueCam1ETarget,
    FlipperCanbusDashboardValueCam2I,
    FlipperCanbusDashboardValueCam2ITarget,
    FlipperCanbusDashboardValueCam2E,
    FlipperCanbusDashboardValueCam2ETarget,
    FlipperCanbusDashboardValueEgt1,
    FlipperCanbusDashboardValueEgt2,
    FlipperCanbusDashboardValueEgt3,
    FlipperCanbusDashboardValueEgt4,
    FlipperCanbusDashboardValueEgt5,
    FlipperCanbusDashboardValueEgt6,
    FlipperCanbusDashboardValueEgt7,
    FlipperCanbusDashboardValueEgt8,
} FlipperCanbusDashboardValueId;

typedef struct {
    const char* label;
    FlipperCanbusDashboardValueId value_id;
    FlipperCanbusDashboardFormat format;
    const char* unit;
} FlipperCanbusDashboardRow;

typedef struct {
    const char* title;
    FlipperCanbusDashboardRow rows[FLIPPER_CANBUS_DASHBOARD_ROWS];
} FlipperCanbusDashboardPage;

struct FlipperCanbusViewDashboard {
    View* view;
};

typedef struct {
    RusEfiCanDecoded decoded;
    FuriString* line;
    uint8_t page;
} FlipperCanbusViewDashboardModel;

static const FlipperCanbusDashboardPage flipper_canbus_dashboard_pages[] = {
    {
        .title = "Engine",
        .rows =
            {
                {"RPM", FlipperCanbusDashboardValueRpm, FlipperCanbusDashboardFormatInt, "rpm"},
                {"MAP", FlipperCanbusDashboardValueMap, FlipperCanbusDashboardFormatFixed1, "kPa"},
                {"Ignition",
                 FlipperCanbusDashboardValueIgnition,
                 FlipperCanbusDashboardFormatFixed2,
                 "deg"},
                {"Injector PW",
                 FlipperCanbusDashboardValueInjPw,
                 FlipperCanbusDashboardFormatFixed3,
                 "ms"},
                {"Battery",
                 FlipperCanbusDashboardValueBatt,
                 FlipperCanbusDashboardFormatFixed3,
                 "V"},
            },
    },
    {
        .title = "Engine 2",
        .rows =
            {
                {"Injector Duty",
                 FlipperCanbusDashboardValueInjDuty,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Ignition Duty",
                 FlipperCanbusDashboardValueIgnDuty,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Speed", FlipperCanbusDashboardValueSpeed, FlipperCanbusDashboardFormatInt, "kph"},
                {"Gear", FlipperCanbusDashboardValueGear, FlipperCanbusDashboardFormatInt, NULL},
                {"Flex", FlipperCanbusDashboardValueFlex, FlipperCanbusDashboardFormatInt, "%"},
            },
    },
    {
        .title = "Throttle/Air",
        .rows =
            {
                {"Pedal", FlipperCanbusDashboardValuePps, FlipperCanbusDashboardFormatFixed2, "%"},
                {"TPS1", FlipperCanbusDashboardValueTps1, FlipperCanbusDashboardFormatFixed2, "%"},
                {"TPS2", FlipperCanbusDashboardValueTps2, FlipperCanbusDashboardFormatFixed2, "%"},
                {"Wastegate",
                 FlipperCanbusDashboardValueWastegate,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Cylinder Air",
                 FlipperCanbusDashboardValueCylAir,
                 FlipperCanbusDashboardFormatInt,
                 "mg"},
            },
    },
    {
        .title = "Air/Knock",
        .rows =
            {
                {"Estimated MAF",
                 FlipperCanbusDashboardValueMaf,
                 FlipperCanbusDashboardFormatFixed2,
                 "kg/h"},
                {"Knock Count",
                 FlipperCanbusDashboardValueKnockCount,
                 FlipperCanbusDashboardFormatInt,
                 NULL},
                {"Knock 0",
                 FlipperCanbusDashboardValueKnock0,
                 FlipperCanbusDashboardFormatInt,
                 "db"},
                {"Knock 1",
                 FlipperCanbusDashboardValueKnock1,
                 FlipperCanbusDashboardFormatInt,
                 "db"},
                {"Knock 2",
                 FlipperCanbusDashboardValueKnock2,
                 FlipperCanbusDashboardFormatInt,
                 "db"},
            },
    },
    {
        .title = "Temps",
        .rows =
            {
                {"Coolant",
                 FlipperCanbusDashboardValueCoolant,
                 FlipperCanbusDashboardFormatInt,
                 "C"},
                {"Intake Air",
                 FlipperCanbusDashboardValueIntake,
                 FlipperCanbusDashboardFormatInt,
                 "C"},
                {"Oil", FlipperCanbusDashboardValueOilTemp, FlipperCanbusDashboardFormatInt, "C"},
                {"Fuel", FlipperCanbusDashboardValueFuelTemp, FlipperCanbusDashboardFormatInt, "C"},
                {"MCU", FlipperCanbusDashboardValueMcuTemp, FlipperCanbusDashboardFormatInt, "C"},
            },
    },
    {
        .title = "Temps/Fuel",
        .rows =
            {
                {"Aux1", FlipperCanbusDashboardValueAux1, FlipperCanbusDashboardFormatInt, "C"},
                {"Aux2", FlipperCanbusDashboardValueAux2, FlipperCanbusDashboardFormatInt, "C"},
                {"Fuel Level",
                 FlipperCanbusDashboardValueFuelLevel,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Fuel Used",
                 FlipperCanbusDashboardValueFuelUsed,
                 FlipperCanbusDashboardFormatInt,
                 "g"},
                {"Fuel Flow",
                 FlipperCanbusDashboardValueFuelFlow,
                 FlipperCanbusDashboardFormatFixed3,
                 "g/s"},
            },
    },
    {
        .title = "Fuel/Pressure",
        .rows =
            {
                {"Lambda 1",
                 FlipperCanbusDashboardValueLambda1,
                 FlipperCanbusDashboardFormatFixed4,
                 NULL},
                {"Lambda 2",
                 FlipperCanbusDashboardValueLambda2,
                 FlipperCanbusDashboardFormatFixed4,
                 NULL},
                {"Fuel Trim 1",
                 FlipperCanbusDashboardValueFuelTrim1,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Fuel Trim 2",
                 FlipperCanbusDashboardValueFuelTrim2,
                 FlipperCanbusDashboardFormatFixed2,
                 "%"},
                {"Oil Pressure",
                 FlipperCanbusDashboardValueOilPress,
                 FlipperCanbusDashboardFormatFixed1,
                 "kPa"},
            },
    },
    {
        .title = "Pressure",
        .rows =
            {
                {"Fuel Low",
                 FlipperCanbusDashboardValueFpLow,
                 FlipperCanbusDashboardFormatFixed1,
                 "kPa"},
                {"Fuel High",
                 FlipperCanbusDashboardValueFpHigh,
                 FlipperCanbusDashboardFormatFixed1,
                 "bar"},
            },
    },
    {
        .title = "Status",
        .rows =
            {
                {"Warnings",
                 FlipperCanbusDashboardValueWarnings,
                 FlipperCanbusDashboardFormatInt,
                 NULL},
                {"Last Error",
                 FlipperCanbusDashboardValueLastError,
                 FlipperCanbusDashboardFormatInt,
                 NULL},
                {"Distance",
                 FlipperCanbusDashboardValueDistance,
                 FlipperCanbusDashboardFormatFixed1,
                 "km"},
                {"Main Relay",
                 FlipperCanbusDashboardValueMainRelay,
                 FlipperCanbusDashboardFormatBool,
                 NULL},
                {"Fuel Pump",
                 FlipperCanbusDashboardValueFuelPump,
                 FlipperCanbusDashboardFormatBool,
                 NULL},
            },
    },
    {
        .title = "Status 2",
        .rows =
            {
                {"CEL", FlipperCanbusDashboardValueCel, FlipperCanbusDashboardFormatBool, NULL},
                {"Rev Limiter",
                 FlipperCanbusDashboardValueRevLimiter,
                 FlipperCanbusDashboardFormatBool,
                 NULL},
                {"Fan 1", FlipperCanbusDashboardValueFan1, FlipperCanbusDashboardFormatBool, NULL},
                {"Fan 2", FlipperCanbusDashboardValueFan2, FlipperCanbusDashboardFormatBool, NULL},
                {"EGO Heat",
                 FlipperCanbusDashboardValueEgoHeat,
                 FlipperCanbusDashboardFormatBool,
                 NULL},
            },
    },
    {
        .title = "Cams",
        .rows =
            {
                {"Cam1 Intake",
                 FlipperCanbusDashboardValueCam1I,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam1 Intake Tar",
                 FlipperCanbusDashboardValueCam1ITarget,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam1 Exhaust",
                 FlipperCanbusDashboardValueCam1E,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam1 Exhaust Tar",
                 FlipperCanbusDashboardValueCam1ETarget,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam2 Intake",
                 FlipperCanbusDashboardValueCam2I,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
            },
    },
    {
        .title = "Cams/EGT",
        .rows =
            {
                {"Cam2 Intake Tar",
                 FlipperCanbusDashboardValueCam2ITarget,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam2 Exhaust",
                 FlipperCanbusDashboardValueCam2E,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Cam2 Exhaust Tar",
                 FlipperCanbusDashboardValueCam2ETarget,
                 FlipperCanbusDashboardFormatInt,
                 "deg"},
                {"Lambda Protect",
                 FlipperCanbusDashboardValueLambdaProtect,
                 FlipperCanbusDashboardFormatBool,
                 NULL},
                {"EGT 1", FlipperCanbusDashboardValueEgt1, FlipperCanbusDashboardFormatInt, "C"},
            },
    },
    {
        .title = "EGT",
        .rows =
            {
                {"EGT 2", FlipperCanbusDashboardValueEgt2, FlipperCanbusDashboardFormatInt, "C"},
                {"EGT 3", FlipperCanbusDashboardValueEgt3, FlipperCanbusDashboardFormatInt, "C"},
                {"EGT 4", FlipperCanbusDashboardValueEgt4, FlipperCanbusDashboardFormatInt, "C"},
                {"EGT 5", FlipperCanbusDashboardValueEgt5, FlipperCanbusDashboardFormatInt, "C"},
                {"EGT 6", FlipperCanbusDashboardValueEgt6, FlipperCanbusDashboardFormatInt, "C"},
            },
    },
    {
        .title = "EGT 2",
        .rows =
            {
                {"EGT 7", FlipperCanbusDashboardValueEgt7, FlipperCanbusDashboardFormatInt, "C"},
                {"EGT 8", FlipperCanbusDashboardValueEgt8, FlipperCanbusDashboardFormatInt, "C"},
            },
    },
};

static const uint8_t flipper_canbus_dashboard_page_count =
    COUNT_OF(flipper_canbus_dashboard_pages);

static const RusEfiCanValue* flipper_canbus_view_dashboard_get_value(
    const RusEfiCanDecoded* decoded,
    FlipperCanbusDashboardValueId value_id) {
    switch(value_id) {
    case FlipperCanbusDashboardValueRpm:
        return &decoded->rpm;
    case FlipperCanbusDashboardValueMap:
        return &decoded->map_deci_kpa;
    case FlipperCanbusDashboardValueIgnition:
        return &decoded->ignition_cdeg;
    case FlipperCanbusDashboardValueInjPw:
        return &decoded->inj_pw_us;
    case FlipperCanbusDashboardValueBatt:
        return &decoded->batt_mv;
    case FlipperCanbusDashboardValueInjDuty:
        return &decoded->inj_duty_cpercent;
    case FlipperCanbusDashboardValueIgnDuty:
        return &decoded->ign_duty_cpercent;
    case FlipperCanbusDashboardValueSpeed:
        return &decoded->vehicle_speed_kph;
    case FlipperCanbusDashboardValueGear:
        return &decoded->current_gear;
    case FlipperCanbusDashboardValueFlex:
        return &decoded->flex_percent;
    case FlipperCanbusDashboardValuePps:
        return &decoded->pps_cpercent;
    case FlipperCanbusDashboardValueTps1:
        return &decoded->tps1_cpercent;
    case FlipperCanbusDashboardValueTps2:
        return &decoded->tps2_cpercent;
    case FlipperCanbusDashboardValueWastegate:
        return &decoded->wastegate_cpercent;
    case FlipperCanbusDashboardValueCylAir:
        return &decoded->cyl_airmass_mg;
    case FlipperCanbusDashboardValueMaf:
        return &decoded->est_maf_centi_kgh;
    case FlipperCanbusDashboardValueKnockCount:
        return &decoded->knock_count;
    case FlipperCanbusDashboardValueKnock0:
        return &decoded->knock_db[0];
    case FlipperCanbusDashboardValueKnock1:
        return &decoded->knock_db[1];
    case FlipperCanbusDashboardValueKnock2:
        return &decoded->knock_db[2];
    case FlipperCanbusDashboardValueCoolant:
        return &decoded->coolant_temp_c;
    case FlipperCanbusDashboardValueIntake:
        return &decoded->intake_temp_c;
    case FlipperCanbusDashboardValueOilTemp:
        return &decoded->oil_temp_c;
    case FlipperCanbusDashboardValueFuelTemp:
        return &decoded->fuel_temp_c;
    case FlipperCanbusDashboardValueMcuTemp:
        return &decoded->mcu_temp_c;
    case FlipperCanbusDashboardValueAux1:
        return &decoded->aux1_temp_c;
    case FlipperCanbusDashboardValueAux2:
        return &decoded->aux2_temp_c;
    case FlipperCanbusDashboardValueFuelLevel:
        return &decoded->fuel_level_cpercent;
    case FlipperCanbusDashboardValueFuelUsed:
        return &decoded->fuel_used_g;
    case FlipperCanbusDashboardValueFuelFlow:
        return &decoded->fuel_flow_mgps;
    case FlipperCanbusDashboardValueLambda1:
        return &decoded->lambda1_ten_thousandth;
    case FlipperCanbusDashboardValueLambda2:
        return &decoded->lambda2_ten_thousandth;
    case FlipperCanbusDashboardValueFuelTrim1:
        return &decoded->fuel_trim1_cpercent;
    case FlipperCanbusDashboardValueFuelTrim2:
        return &decoded->fuel_trim2_cpercent;
    case FlipperCanbusDashboardValueOilPress:
        return &decoded->oil_press_deci_kpa;
    case FlipperCanbusDashboardValueFpLow:
        return &decoded->fp_low_deci_kpa;
    case FlipperCanbusDashboardValueFpHigh:
        return &decoded->fp_high_decibar;
    case FlipperCanbusDashboardValueWarnings:
        return &decoded->warning_counter;
    case FlipperCanbusDashboardValueLastError:
        return &decoded->last_error;
    case FlipperCanbusDashboardValueDistance:
        return &decoded->distance_deci_km;
    case FlipperCanbusDashboardValueMainRelay:
        return &decoded->main_relay_act;
    case FlipperCanbusDashboardValueFuelPump:
        return &decoded->fuel_pump_act;
    case FlipperCanbusDashboardValueCel:
        return &decoded->cel_act;
    case FlipperCanbusDashboardValueRevLimiter:
        return &decoded->rev_lim_act;
    case FlipperCanbusDashboardValueFan1:
        return &decoded->fan;
    case FlipperCanbusDashboardValueFan2:
        return &decoded->fan2;
    case FlipperCanbusDashboardValueEgoHeat:
        return &decoded->ego_heat_act;
    case FlipperCanbusDashboardValueLambdaProtect:
        return &decoded->lambda_protect_act;
    case FlipperCanbusDashboardValueCam1I:
        return &decoded->cam1_i_deg;
    case FlipperCanbusDashboardValueCam1ITarget:
        return &decoded->cam1_i_target_deg;
    case FlipperCanbusDashboardValueCam1E:
        return &decoded->cam1_e_deg;
    case FlipperCanbusDashboardValueCam1ETarget:
        return &decoded->cam1_e_target_deg;
    case FlipperCanbusDashboardValueCam2I:
        return &decoded->cam2_i_deg;
    case FlipperCanbusDashboardValueCam2ITarget:
        return &decoded->cam2_i_target_deg;
    case FlipperCanbusDashboardValueCam2E:
        return &decoded->cam2_e_deg;
    case FlipperCanbusDashboardValueCam2ETarget:
        return &decoded->cam2_e_target_deg;
    case FlipperCanbusDashboardValueEgt1:
        return &decoded->egt_c[0];
    case FlipperCanbusDashboardValueEgt2:
        return &decoded->egt_c[1];
    case FlipperCanbusDashboardValueEgt3:
        return &decoded->egt_c[2];
    case FlipperCanbusDashboardValueEgt4:
        return &decoded->egt_c[3];
    case FlipperCanbusDashboardValueEgt5:
        return &decoded->egt_c[4];
    case FlipperCanbusDashboardValueEgt6:
        return &decoded->egt_c[5];
    case FlipperCanbusDashboardValueEgt7:
        return &decoded->egt_c[6];
    case FlipperCanbusDashboardValueEgt8:
        return &decoded->egt_c[7];
    }

    furi_crash();
}

static void
    flipper_canbus_view_dashboard_format_fixed(FuriString* out, int32_t value, uint8_t decimals) {
    int32_t div = 1;
    for(uint8_t i = 0; i < decimals; i++) {
        div *= 10;
    }

    const bool negative = value < 0;
    const uint32_t abs_value = negative ? (uint32_t)(-(value + 1)) + 1U : (uint32_t)value;
    const char* sign = negative ? "-" : "";
    const uint32_t whole = abs_value / (uint32_t)div;
    const uint32_t fraction = abs_value % (uint32_t)div;

    furi_string_printf(out, "%s%lu.%0*lu", sign, whole, decimals, fraction);
}

static void flipper_canbus_view_dashboard_format_value(
    FuriString* out,
    const RusEfiCanValue* value,
    FlipperCanbusDashboardFormat format) {
    if(!value->valid) {
        furi_string_set_str(out, format == FlipperCanbusDashboardFormatBool ? "-" : "--");
        return;
    }

    switch(format) {
    case FlipperCanbusDashboardFormatBool:
        furi_string_set_str(out, value->value ? "1" : "0");
        break;
    case FlipperCanbusDashboardFormatInt:
        furi_string_printf(out, "%ld", value->value);
        break;
    case FlipperCanbusDashboardFormatFixed1:
        flipper_canbus_view_dashboard_format_fixed(out, value->value, 1U);
        break;
    case FlipperCanbusDashboardFormatFixed2:
        flipper_canbus_view_dashboard_format_fixed(out, value->value, 2U);
        break;
    case FlipperCanbusDashboardFormatFixed3:
        flipper_canbus_view_dashboard_format_fixed(out, value->value, 3U);
        break;
    case FlipperCanbusDashboardFormatFixed4:
        flipper_canbus_view_dashboard_format_fixed(out, value->value, 4U);
        break;
    default:
        furi_crash();
    }
}

static void flipper_canbus_view_dashboard_draw_row(
    FlipperCanbusViewDashboardModel* model,
    Canvas* canvas,
    uint8_t row,
    const FlipperCanbusDashboardRow* dashboard_row) {
    const uint8_t y = 8 + row * 10;

    canvas_draw_str(canvas, FLIPPER_CANBUS_DASHBOARD_LABEL_X, y, dashboard_row->label);
    flipper_canbus_view_dashboard_format_value(
        model->line,
        flipper_canbus_view_dashboard_get_value(&model->decoded, dashboard_row->value_id),
        dashboard_row->format);
    if(dashboard_row->unit) {
        furi_string_cat_printf(model->line, " %s", dashboard_row->unit);
    }
    canvas_draw_str_aligned(
        canvas,
        FLIPPER_CANBUS_DASHBOARD_VALUE_X,
        y,
        AlignRight,
        AlignBottom,
        furi_string_get_cstr(model->line));
}

static void flipper_canbus_view_dashboard_draw_callback(Canvas* canvas, void* _model) {
    FlipperCanbusViewDashboardModel* model = _model;
    const FlipperCanbusDashboardPage* page = &flipper_canbus_dashboard_pages[model->page];

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    furi_string_printf(
        model->line, "< %u/%u >", model->page + 1U, flipper_canbus_dashboard_page_count);
    canvas_draw_str_aligned(
        canvas, 127, 8, AlignRight, AlignBottom, furi_string_get_cstr(model->line));

    furi_string_set_str(model->line, page->title);
    canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(model->line));

    for(uint8_t i = 0; i < FLIPPER_CANBUS_DASHBOARD_ROWS; i++) {
        if(page->rows[i].label) {
            flipper_canbus_view_dashboard_draw_row(model, canvas, i + 1U, &page->rows[i]);
        }
    }
}

static bool flipper_canbus_view_dashboard_input_callback(InputEvent* event, void* context) {
    FlipperCanbusViewDashboard* view_dashboard = context;

    if((event->type != InputTypeShort) && (event->type != InputTypeRepeat)) {
        return false;
    }

    if((event->key != InputKeyLeft) && (event->key != InputKeyRight)) {
        return false;
    }

    with_view_model(
        view_dashboard->view,
        FlipperCanbusViewDashboardModel * model,
        {
            if(event->key == InputKeyLeft) {
                model->page = (model->page == 0) ? flipper_canbus_dashboard_page_count - 1U :
                                                   model->page - 1U;
            } else {
                model->page = (model->page + 1U) % flipper_canbus_dashboard_page_count;
            }
        },
        true);

    return true;
}

FlipperCanbusViewDashboard* flipper_canbus_view_dashboard_alloc(void) {
    FlipperCanbusViewDashboard* view_dashboard = malloc(sizeof(FlipperCanbusViewDashboard));

    view_dashboard->view = view_alloc();
    view_allocate_model(
        view_dashboard->view, ViewModelTypeLocking, sizeof(FlipperCanbusViewDashboardModel));
    view_set_context(view_dashboard->view, view_dashboard);
    view_set_draw_callback(view_dashboard->view, flipper_canbus_view_dashboard_draw_callback);
    view_set_input_callback(view_dashboard->view, flipper_canbus_view_dashboard_input_callback);

    with_view_model(
        view_dashboard->view,
        FlipperCanbusViewDashboardModel * model,
        {
            memset(&model->decoded, 0, sizeof(model->decoded));
            model->line = furi_string_alloc();
            model->page = 0;
        },
        true);

    return view_dashboard;
}

void flipper_canbus_view_dashboard_free(FlipperCanbusViewDashboard* view_dashboard) {
    furi_assert(view_dashboard);
    with_view_model(
        view_dashboard->view,
        FlipperCanbusViewDashboardModel * model,
        { furi_string_free(model->line); },
        false);
    view_free(view_dashboard->view);
    free(view_dashboard);
}

View* flipper_canbus_view_dashboard_get_view(FlipperCanbusViewDashboard* view_dashboard) {
    furi_assert(view_dashboard);
    return view_dashboard->view;
}

void flipper_canbus_view_dashboard_update(
    FlipperCanbusViewDashboard* view_dashboard,
    const RusEfiCanDecoded* decoded) {
    furi_assert(view_dashboard);
    furi_assert(decoded);

    with_view_model(
        view_dashboard->view,
        FlipperCanbusViewDashboardModel * model,
        { model->decoded = *decoded; },
        true);
}
