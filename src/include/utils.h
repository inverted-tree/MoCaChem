#pragma once

#include "maybe.h"

//******************************************************************************
//  Interface datatype declarations
//******************************************************************************

typedef struct {
	mcc_Maybe root_dir;
	mcc_Maybe config;
} mcc_CmdlOpts;

//******************************************************************************
//  Interface function declarations
//******************************************************************************

mcc_CmdlOpts mcc_utils_handle_command_line_args(int argc, char **argv);

void mcc_utils_print_title_image();
