#pragma once

#include "config.h"
#include "maybe.h"

//******************************************************************************
//  Interface Datatype Declarations
//******************************************************************************

typedef struct {
	mcc_Maybe_t config;
	mcc_Maybe_t particles;
} mcc_CmdlOpts_t;

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

void mcc_utils_destroy_command_line_args(mcc_CmdlOpts_t *options);

mcc_CmdlOpts_t mcc_utils_handle_command_line_args(int argc, char **argv);

mcc_Config_t mcc_utils_init_config(mcc_CmdlOpts_t args);

bool mcc_utils_init_particles(mcc_CmdlOpts_t *args, mcc_Config_t *config);

void mcc_utils_print_title_image();
