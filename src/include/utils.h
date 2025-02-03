#pragma once

#include "config.h"
#include <limits.h>
#include <stdbool.h>

//******************************************************************************
//  Interface Type Declarations
//******************************************************************************

#define STATUS_MSG_SIZE 256

typedef enum {
	SUCCESS = 0,
	FAILURE = 1,
} mcc_STATUS;

typedef struct {
	mcc_STATUS is;
	char msg[STATUS_MSG_SIZE];
} mcc_Status_t;

typedef struct {
	char config_file[PATH_MAX];
	char particle_file[PATH_MAX];
	bool minimal;
} mcc_CmdlArgs_t;

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

mcc_Status_t mcc_utils_status_success();

mcc_Status_t mcc_utils_status_failure(char const msg[static 1]);

mcc_Status_t mcc_utils_handle_args(mcc_CmdlArgs_t *args, int argc, char **argv);

mcc_Status_t mcc_utils_parse_config(mcc_Config_t *conf,
                                    mcc_CmdlArgs_t const *args);

mcc_Status_t mcc_utils_init_state_space(mcc_CmdlArgs_t const *args,
                                        mcc_Config_t const *conf);

//******************************************************************************
//  I/O Function Definitions
//******************************************************************************

void mcc_utils_print_title_image();

void mcc_utils_print_failure_msg();

void mcc_utils_print_progress_bar(int progress, int total);
