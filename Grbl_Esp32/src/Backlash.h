#pragma once

/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Grbl.h"


// This array contains the backlash that has beed added to an axes in order
// to remove it later from the Report
//extern float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini();

float backlash_CompensateBacklashToTarget(int axis, float target);

void backlash_CompensateBacklashToTargetV2(float* target, plan_line_data_t* pl_data);

