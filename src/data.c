#include "data.h"
#include "data-array.h"

//******************************************************************************
//  Interface function assignments
//******************************************************************************

mcc_Particle_Access_Functions_t mcc_data_get_access_functions() {
	return mcc_data_array_get_access_functions();
}

mcc_Particle_Iterator_t mcc_data_get_iterator(int index, mcc_Config_t *config) {
	return mcc_data_array_get_iterator(index, config->cutoff_dist);
}
