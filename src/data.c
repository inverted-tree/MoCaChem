#include "data.h"
#include "data-array.h"
#include "include/data-array.h"
#include "include/data.h"
// #include "data-octree.h"

//******************************************************************************
//  Interface function assignments
//******************************************************************************

mcc_DAF_t mcc_data_get_access_fs() { return mcc_data_array_get_access_fs(); }

mcc_Particle_Iterator_t mcc_data_get_iterator(mcc_Index_t *index,
                                              mcc_Config_t *config) {
	return mcc_data_array_get_iterator(index, config->cutoff_dist);
}

mcc_Index_t mcc_data_gen_index(int idx) {
	return mcc_data_array_gen_index(idx);
}
