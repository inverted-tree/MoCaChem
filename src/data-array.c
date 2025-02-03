#include "data-array.h"
#include "config.h"
#include "data.h"
#include "include/data.h"
#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//******************************************************************************
//  Data Representation Definition
//******************************************************************************

// struct mcc_Index_t {
//	int idx;
// };

struct mcc_Data_Array_t {
	bool init;
	size_t length;
	mcc_Particle_t *particles;
};

struct mcc_Iter_State_t {
	int index;
	size_t current;
	double cutoff;
};

struct mcc_Data_Array_t data = {.init = false, .length = 0, .particles = NULL};

struct mcc_Iter_State_t state = {
    .index = 0,
    .current = 0,
    .cutoff = 0.0,
};

//******************************************************************************
//  Internal function declarations
//******************************************************************************

mcc_Status_t mcc_data_array_init(mcc_Config_t config[static 1]);

mcc_Status_t mcc_data_array_destroy();

mcc_Particle_t *mcc_data_array_get_particle(mcc_Index_t *index);

bool mcc_data_array_set_particle(mcc_Index_t *index, mcc_Particle_t *particle);

mcc_Particle_t *mcc_data_array_iterator_next();

bool mcc_data_array_iterator_is_done(mcc_Particle_t *result);

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_DAF_t mcc_data_array_get_access_fs() {
	mcc_DAF_t fs = {
	    .init = mcc_data_array_init,
	    .destroy = mcc_data_array_destroy,
	    .get_particle = mcc_data_array_get_particle,
	    .set_particle = mcc_data_array_set_particle,
	};
	return fs;
}

mcc_Particle_Iterator_t mcc_data_array_get_iterator(mcc_Index_t *index,
                                                    double cutoff) {
	state.index = index->idx;
	state.current = 0;
	state.cutoff = cutoff;

	mcc_Particle_Iterator_t iter = {
	    .next = mcc_data_array_iterator_next,
	    .is_done = mcc_data_array_iterator_is_done,
	};
	return iter;
}

mcc_Index_t mcc_data_array_gen_index(int idx) {
	mcc_Index_t index = {
	    .idx = idx,
	};
	return index;
}

//******************************************************************************
//  Internal function definitions
//******************************************************************************

mcc_Status_t mcc_data_array_init(mcc_Config_t config[static 1]) {
	if (data.init)
		return mcc_utils_status_failure(
		    "Array instance is already initialized");

	size_t number_of_particles = config->particle_count;
	srand(time(NULL));

	mcc_Particle_t *particle_positions =
	    malloc(number_of_particles * sizeof(mcc_Particle_t));
	if (particle_positions == NULL) {
		return mcc_utils_status_failure("Error in particle buffer allocation");
	}

	data.length = number_of_particles;
	data.particles = particle_positions;

	data.init = true;

	return mcc_utils_status_success();
}

mcc_Status_t mcc_data_array_destroy() {
	if (data.particles)
		free(data.particles);
	data.init = false;

	return mcc_utils_status_success();
}

mcc_Particle_t *mcc_data_array_get_particle(mcc_Index_t *index) {
	assert(data.init);

	if (index->idx < 0)
		index->idx = rand() % data.length;

	if ((size_t)index->idx < data.length)
		return &data.particles[index->idx];

	fprintf(stderr, "Particle index '%i' is out of bounds", index->idx);
	exit(1);
}

bool mcc_data_array_set_particle(mcc_Index_t *index, mcc_Particle_t *particle) {
	assert(data.init);
	assert(index->idx >= 0);
	assert((size_t)index->idx < data.length);

	data.particles[index->idx] = *particle;
	return true;
}

mcc_Particle_t *mcc_data_array_iterator_next() {
	if (state.current == (size_t)state.index)
		state.current++;
	return (state.current < data.length) ? &data.particles[state.current++]
	                                     : NULL;
}

bool mcc_data_array_iterator_is_done(mcc_Particle_t *result) {
	return result == NULL;
}
