#pragma once

#include "config.h"
#include "utils.h"

//*****************************************************************************
//  Interface Function Declarations
//******************************************************************************

mcc_Status_t mcc_toml_parse_config(mcc_Config_t *config,
                                   char const config_path[static 1]);

mcc_Status_t mcc_toml_generate_config_file(char const config_path[static 1]);
