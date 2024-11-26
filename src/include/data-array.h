#pragma once

#include "data.h"

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

mcc_Particle_Access_Functions_t mcc_data_array_get_access_functions();

mcc_Particle_Iterator_t mcc_data_array_get_iterator(int index, double cutoff);
