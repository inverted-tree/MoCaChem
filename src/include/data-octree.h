#pragma once

#include "data.h"

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

mcc_DAF_t mcc_data_octree_get_access_fs();

mcc_Particle_Iterator_t mcc_data_octree_get_iterator(mcc_Index_t *index,
                                                     double cutoff);
