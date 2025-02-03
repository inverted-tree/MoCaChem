#pragma once

#include "config.h"
#include "lennard-jones.h"

//******************************************************************************
// Interface Function Declarations
//******************************************************************************

bool mcc_monte_carlo_move(mcc_Energy_t *energy, mcc_Config_t config[static 1]);
