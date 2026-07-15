#include "rusefi_can_decoder.h"

#include <furi.h>

#include <string.h>

static bool
    rusefi_can_decoder_get_u8(const FlipperCanbusFrame* frame, uint8_t offset, uint8_t* value) {
    if(frame->last_len < (uint32_t)offset + 1U) return false;
    *value = frame->last_data[offset];
    return true;
}

static bool
    rusefi_can_decoder_get_i8(const FlipperCanbusFrame* frame, uint8_t offset, int8_t* value) {
    uint8_t raw = 0;
    if(!rusefi_can_decoder_get_u8(frame, offset, &raw)) return false;
    *value = (int8_t)raw;
    return true;
}

static bool rusefi_can_decoder_get_u16_le(
    const FlipperCanbusFrame* frame,
    uint8_t offset,
    uint16_t* value) {
    if(frame->last_len < (uint32_t)offset + 2U) return false;
    *value = (uint16_t)frame->last_data[offset] | ((uint16_t)frame->last_data[offset + 1U] << 8);
    return true;
}

static bool
    rusefi_can_decoder_get_i16_le(const FlipperCanbusFrame* frame, uint8_t offset, int16_t* value) {
    uint16_t raw = 0;
    if(!rusefi_can_decoder_get_u16_le(frame, offset, &raw)) return false;
    *value = (int16_t)raw;
    return true;
}

static bool rusefi_can_decoder_get_bit(const FlipperCanbusFrame* frame, uint8_t bit, bool* value) {
    const uint8_t offset = bit / 8U;
    const uint8_t mask = 1U << (bit % 8U);
    uint8_t raw = 0;
    if(!rusefi_can_decoder_get_u8(frame, offset, &raw)) return false;
    *value = (raw & mask) != 0;
    return true;
}

static void rusefi_can_decoder_set(RusEfiCanValue* dst, int32_t value) {
    dst->valid = true;
    dst->value = value;
}

static void rusefi_can_decoder_decode_u8(
    const FlipperCanbusFrame* frame,
    uint8_t offset,
    RusEfiCanValue* dst,
    int32_t mul,
    int32_t div,
    int32_t offset_value) {
    uint8_t raw = 0;
    if(rusefi_can_decoder_get_u8(frame, offset, &raw)) {
        rusefi_can_decoder_set(dst, ((int32_t)raw * mul) / div + offset_value);
    }
}

static void rusefi_can_decoder_decode_i8(
    const FlipperCanbusFrame* frame,
    uint8_t offset,
    RusEfiCanValue* dst,
    int32_t mul,
    int32_t div,
    int32_t offset_value) {
    int8_t raw = 0;
    if(rusefi_can_decoder_get_i8(frame, offset, &raw)) {
        rusefi_can_decoder_set(dst, ((int32_t)raw * mul) / div + offset_value);
    }
}

static void rusefi_can_decoder_decode_u16(
    const FlipperCanbusFrame* frame,
    uint8_t offset,
    RusEfiCanValue* dst,
    int32_t mul,
    int32_t div,
    int32_t offset_value) {
    uint16_t raw = 0;
    if(rusefi_can_decoder_get_u16_le(frame, offset, &raw)) {
        rusefi_can_decoder_set(dst, ((int32_t)raw * mul) / div + offset_value);
    }
}

static void rusefi_can_decoder_decode_i16(
    const FlipperCanbusFrame* frame,
    uint8_t offset,
    RusEfiCanValue* dst,
    int32_t mul,
    int32_t div,
    int32_t offset_value) {
    int16_t raw = 0;
    if(rusefi_can_decoder_get_i16_le(frame, offset, &raw)) {
        rusefi_can_decoder_set(dst, ((int32_t)raw * mul) / div + offset_value);
    }
}

static void rusefi_can_decoder_decode_bit(
    const FlipperCanbusFrame* frame,
    uint8_t bit,
    RusEfiCanValue* dst) {
    bool value = false;
    if(rusefi_can_decoder_get_bit(frame, bit, &value)) {
        rusefi_can_decoder_set(dst, value ? 1 : 0);
    }
}

void rusefi_can_decoder_decode(FlipperCanbusWorker* worker, RusEfiCanDecoded* decoded) {
    furi_assert(worker);
    furi_assert(decoded);

    memset(decoded, 0, sizeof(RusEfiCanDecoded));

    FlipperCanbusFrame frame = {0};
    if(flipper_canbus_worker_get_frame(worker, 0x200, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->warning_counter, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 2, &decoded->last_error, 1, 1, 0);
        rusefi_can_decoder_decode_bit(&frame, 32, &decoded->rev_lim_act);
        rusefi_can_decoder_decode_bit(&frame, 33, &decoded->main_relay_act);
        rusefi_can_decoder_decode_bit(&frame, 34, &decoded->fuel_pump_act);
        rusefi_can_decoder_decode_bit(&frame, 35, &decoded->cel_act);
        rusefi_can_decoder_decode_bit(&frame, 36, &decoded->ego_heat_act);
        rusefi_can_decoder_decode_bit(&frame, 37, &decoded->lambda_protect_act);
        rusefi_can_decoder_decode_bit(&frame, 38, &decoded->fan);
        rusefi_can_decoder_decode_bit(&frame, 39, &decoded->fan2);
        rusefi_can_decoder_decode_u8(&frame, 5, &decoded->current_gear, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 6, &decoded->distance_deci_km, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x201, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->rpm, 1, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 2, &decoded->ignition_cdeg, 2, 1, 0);
        rusefi_can_decoder_decode_u8(&frame, 4, &decoded->inj_duty_cpercent, 50, 1, 0);
        rusefi_can_decoder_decode_u8(&frame, 5, &decoded->ign_duty_cpercent, 50, 1, 0);
        rusefi_can_decoder_decode_u8(&frame, 6, &decoded->vehicle_speed_kph, 1, 1, 0);
        rusefi_can_decoder_decode_u8(&frame, 7, &decoded->flex_percent, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x202, &frame)) {
        rusefi_can_decoder_decode_i16(&frame, 0, &decoded->pps_cpercent, 1, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 2, &decoded->tps1_cpercent, 1, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 4, &decoded->tps2_cpercent, 1, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 6, &decoded->wastegate_cpercent, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x203, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->map_deci_kpa, 1, 3, 0);
        rusefi_can_decoder_decode_u8(&frame, 2, &decoded->coolant_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 3, &decoded->intake_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 4, &decoded->aux1_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 5, &decoded->aux2_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 6, &decoded->mcu_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 7, &decoded->fuel_level_cpercent, 50, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x204, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 2, &decoded->oil_press_deci_kpa, 1, 3, 0);
        rusefi_can_decoder_decode_u8(&frame, 4, &decoded->oil_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u8(&frame, 5, &decoded->fuel_temp_c, 1, 1, -40);
        rusefi_can_decoder_decode_u16(&frame, 6, &decoded->batt_mv, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x205, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->cyl_airmass_mg, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 2, &decoded->est_maf_centi_kgh, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 4, &decoded->inj_pw_us, 10, 3, 0);
        rusefi_can_decoder_decode_u16(&frame, 6, &decoded->knock_count, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x206, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->fuel_used_g, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 2, &decoded->fuel_flow_mgps, 5, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 4, &decoded->fuel_trim1_cpercent, 1, 1, 0);
        rusefi_can_decoder_decode_i16(&frame, 6, &decoded->fuel_trim2_cpercent, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x207, &frame)) {
        rusefi_can_decoder_decode_u16(&frame, 0, &decoded->lambda1_ten_thousandth, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 2, &decoded->lambda2_ten_thousandth, 1, 1, 0);
        rusefi_can_decoder_decode_u16(&frame, 4, &decoded->fp_low_deci_kpa, 1, 3, 0);
        rusefi_can_decoder_decode_u16(&frame, 6, &decoded->fp_high_decibar, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x208, &frame)) {
        rusefi_can_decoder_decode_i8(&frame, 0, &decoded->cam1_i_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 1, &decoded->cam1_i_target_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 2, &decoded->cam1_e_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 3, &decoded->cam1_e_target_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 4, &decoded->cam2_i_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 5, &decoded->cam2_i_target_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 6, &decoded->cam2_e_deg, 1, 1, 0);
        rusefi_can_decoder_decode_i8(&frame, 7, &decoded->cam2_e_target_deg, 1, 1, 0);
    }

    if(flipper_canbus_worker_get_frame(worker, 0x209, &frame)) {
        for(uint8_t i = 0; i < 8U; i++) {
            rusefi_can_decoder_decode_u8(&frame, i, &decoded->egt_c[i], 5, 1, 0);
        }
    }

    if(flipper_canbus_worker_get_frame(worker, 0x20A, &frame)) {
        for(uint8_t i = 0; i < 3U; i++) {
            rusefi_can_decoder_decode_i8(&frame, i, &decoded->knock_db[i], 1, 1, 0);
        }
    }
}
