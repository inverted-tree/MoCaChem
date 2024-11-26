#pragma once

#include "config.h"
#include "maybe.h"

//******************************************************************************
//  Interface Datatype Declarations
//******************************************************************************

typedef struct {
	mcc_Maybe_t root_dir;
	mcc_Maybe_t config;
} mcc_CmdlOpts_t;

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

void mcc_utils_destroy_command_line_options(mcc_CmdlOpts_t options);

mcc_CmdlOpts_t mcc_utils_handle_command_line_args(int argc, char **argv);

mcc_Config_t mcc_utils_init_config(mcc_CmdlOpts_t args);

void mcc_utils_print_title_image();
