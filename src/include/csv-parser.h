#pragma once

#include "config.h"
#include <stdbool.h>

//******************************************************************************
//  Interface function declarations
//******************************************************************************

bool mcc_csv_read_particle_configuration(char const *filename,
                                         mcc_Config_t *config);

bool mcc_csv_write_particle_configuration(char const *filename,
                                          mcc_Config_t *config);
