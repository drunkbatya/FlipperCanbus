#pragma once

#include <workers/flipper_canbus_worker.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool valid;
    int32_t value;
} RusEfiCanValue;

typedef struct {
    RusEfiCanValue warning_counter;
    RusEfiCanValue last_error;
    RusEfiCanValue rev_lim_act;
    RusEfiCanValue main_relay_act;
    RusEfiCanValue fuel_pump_act;
    RusEfiCanValue cel_act;
    RusEfiCanValue ego_heat_act;
    RusEfiCanValue lambda_protect_act;
    RusEfiCanValue fan;
    RusEfiCanValue fan2;
    RusEfiCanValue current_gear;
    RusEfiCanValue distance_deci_km;

    RusEfiCanValue rpm;
    RusEfiCanValue ignition_cdeg;
    RusEfiCanValue inj_duty_cpercent;
    RusEfiCanValue ign_duty_cpercent;
    RusEfiCanValue vehicle_speed_kph;
    RusEfiCanValue flex_percent;

    RusEfiCanValue pps_cpercent;
    RusEfiCanValue tps1_cpercent;
    RusEfiCanValue tps2_cpercent;
    RusEfiCanValue wastegate_cpercent;

    RusEfiCanValue map_deci_kpa;
    RusEfiCanValue coolant_temp_c;
    RusEfiCanValue intake_temp_c;
    RusEfiCanValue aux1_temp_c;
    RusEfiCanValue aux2_temp_c;
    RusEfiCanValue mcu_temp_c;
    RusEfiCanValue fuel_level_cpercent;

    RusEfiCanValue oil_press_deci_kpa;
    RusEfiCanValue oil_temp_c;
    RusEfiCanValue fuel_temp_c;
    RusEfiCanValue batt_mv;

    RusEfiCanValue cyl_airmass_mg;
    RusEfiCanValue est_maf_centi_kgh;
    RusEfiCanValue inj_pw_us;
    RusEfiCanValue knock_count;

    RusEfiCanValue fuel_used_g;
    RusEfiCanValue fuel_flow_mgps;
    RusEfiCanValue fuel_trim1_cpercent;
    RusEfiCanValue fuel_trim2_cpercent;

    RusEfiCanValue lambda1_ten_thousandth;
    RusEfiCanValue lambda2_ten_thousandth;
    RusEfiCanValue fp_low_deci_kpa;
    RusEfiCanValue fp_high_decibar;

    RusEfiCanValue cam1_i_deg;
    RusEfiCanValue cam1_i_target_deg;
    RusEfiCanValue cam1_e_deg;
    RusEfiCanValue cam1_e_target_deg;
    RusEfiCanValue cam2_i_deg;
    RusEfiCanValue cam2_i_target_deg;
    RusEfiCanValue cam2_e_deg;
    RusEfiCanValue cam2_e_target_deg;

    RusEfiCanValue egt_c[8];
    RusEfiCanValue knock_db[3];
} RusEfiCanDecoded;

void rusefi_can_decoder_decode(FlipperCanbusWorker* worker, RusEfiCanDecoded* decoded);
