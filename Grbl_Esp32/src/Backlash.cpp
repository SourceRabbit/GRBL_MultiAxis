
/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Backlash.h"

#define DIR_POSITIVE     0
#define DIR_NEGATIVE     1
#define DIR_NEUTRAL      2

static float previous_targets[MAX_N_AXIS] = {0.000};
uint8_t target_directions[MAX_N_AXIS] = {DIR_NEUTRAL};

void backlash_ini() {
    for (int i = 0; i < MAX_N_AXIS; i++) {
        previous_targets[i] = 0.000;
        target_directions[i] = DIR_NEUTRAL;
    }
}

void backlash_CompensateBacklashToTargetV2(float* target, plan_line_data_t* pl_data) {
    plan_line_data_t plan_backlash;
    plan_line_data_t* pl_backlash = &plan_backlash;

    pl_backlash->coolant = pl_data->coolant;
    pl_backlash->feed_rate = pl_data->feed_rate;
    pl_backlash->motion.backlash_motion = 1;
    pl_backlash->spindle = pl_data->spindle;
    pl_backlash->spindle_speed = pl_data->spindle_speed;

#ifdef USE_LINE_NUMBERS
    pl_backlash->line_number = pl_data->line_number;
#endif

    bool needsBacklashCompensation = false;

    for (int i = 0; i < MAX_N_AXIS; i++) {

        if (axis_settings[i]->backlash->get() > 0) {

            if (target[i] > previous_targets[i]) {
                // Axis is moving Positive compared to previous move
                if (target_directions[i] != DIR_POSITIVE) {
                    // The axis has changed direction...
                    target_directions[i] = DIR_POSITIVE;
                    previous_targets[i] = previous_targets[i] + axis_settings[i]->backlash->get();
                    needsBacklashCompensation = true;
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Backlash.cpp: Moving Positive");
                }

            } else if (target[i] < previous_targets[i]) {
                // Axis is moving Negative compared to previous move
                if (target_directions[i] != DIR_NEGATIVE) {
                    // The axis has changed direction...
                    target_directions[i] = DIR_NEGATIVE;
                    previous_targets[i] = previous_targets[i] - axis_settings[i]->backlash->get();
                    needsBacklashCompensation = true;
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Backlash.cpp: Moving Negative");
                }
            }
        }
    }

    if (needsBacklashCompensation) {
        // Perform backlash move if necessary
        plan_buffer_line(previous_targets, pl_backlash);
    }


    // If the buffer is full: good! That means we are well ahead of the robot.
    // Remain in this loop until there is room in the buffer.
    do {
        protocol_execute_realtime(); // Check for any run-time commands
        if (sys.abort) {
            return; // Bail, if system abort.
        }
        if (plan_check_full_buffer()) {
            protocol_auto_cycle_start(); // Auto-cycle start when buffer is full.
        } else {
            break;
        }
    } while (1);

    // Copy target to previous target
    for (int i = 0; i < MAX_N_AXIS; i++) {
        previous_targets[i] = target[i];
    }

}

float backlash_CompensateBacklashToTarget(int axis, float target) {
    // This method will run only if the axis has backlash setting set  >  0.
    if (axis_settings[axis]->backlash->get() > 0) {
        if (target > previous_targets[axis]) {
            // The Machine is moving "positive" compared to previous move
            // If the last move was "negative" add backlash compensation to the target
            if (target_directions[axis] == DIR_NEGATIVE) {
                target = target + axis_settings[axis]->backlash->get();
                //backlash_compensation_to_remove_from_mpos[axis] += axis_settings[axis]->backlash->get();
            }

            target_directions[axis] = DIR_POSITIVE;

        } else if (target < previous_targets[axis]) {
            // The Machine is moving "negative" compared to previous move
            // If the last move was "positive" remove backlash compensation from the target
            if (target_directions[axis] == DIR_POSITIVE) {
                target = target - axis_settings[axis]->backlash->get();
                //backlash_compensation_to_remove_from_mpos[axis] -= axis_settings[axis]->backlash->get();
            }

            target_directions[axis] = DIR_NEGATIVE;
        }

        // Update previous target to current target
        previous_targets[axis] = target;
    }

    return target;
}

