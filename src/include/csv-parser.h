#pragma once

#include "config.h"
#include "utils.h"

//******************************************************************************
//  Interface function declarations
//******************************************************************************

mcc_Status_t mcc_csv_load_state_space(char const filename[static 1],
                                      mcc_Config_t const config[static 1]);

mcc_Status_t mcc_csv_write_state_space(char const filename[static 1],
                                       mcc_Config_t const config[static 1]);
