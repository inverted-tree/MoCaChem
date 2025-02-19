#pragma once

#include "data.h"

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

mcc_DAF_t mcc_data_array_get_access_fs();

mcc_Particle_Iterator_t mcc_data_array_get_iterator(mcc_Index_t *index,
                                                    double cutoff);

mcc_Index_t mcc_data_array_gen_index(int idx);
