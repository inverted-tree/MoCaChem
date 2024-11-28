#include "data-array.h"
#include "config.h"
#include "data.h"
#include "panic.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//******************************************************************************
//  Data Representation Definition
//******************************************************************************

struct mcc_Data_Array_t {
	bool is_initialized;
	size_t number_of_particles;
	mcc_Particle_t *particle_positions;
};

struct mcc_Particle_Iterator_State_t {
	size_t current;
	double cutoff;
};

struct mcc_Data_Array_t data = {.is_initialized = false,
                                .number_of_particles = 0,
                                .particle_positions = NULL};

struct mcc_Particle_Iterator_State_t state = {
    .current = 0,
    .cutoff = 0.0,
};

//******************************************************************************
//  Internal function declarations
//******************************************************************************

bool mcc_data_array_initalize(mcc_Config_t *config);

bool mcc_data_array_finalize();

mcc_Particle_t *mcc_data_array_get_particle(int index, mcc_Config_t *config);

bool mcc_data_array_set_particle(int index, mcc_Particle_t particle,
                                 mcc_Config_t *config);

mcc_Particle_t *mcc_data_array_iterator_next();

bool mcc_data_array_iterator_is_done(mcc_Particle_t *result);

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_Particle_Access_Functions_t mcc_data_array_get_access_functions() {
	mcc_Particle_Access_Functions_t functions = {
	    .initialize = mcc_data_array_initalize,
	    .finalize = mcc_data_array_finalize,
	    .get_particle = mcc_data_array_get_particle,
	    .set_particle = mcc_data_array_set_particle,
	};
	return functions;
}

mcc_Particle_Iterator_t mcc_data_array_get_iterator(int index, double cutoff) {
	state.current = index;
	state.cutoff = cutoff;

	mcc_Particle_Iterator_t iterator = {
	    .next = mcc_data_array_iterator_next,
	    .is_done = mcc_data_array_iterator_is_done,
	};
	return iterator;
}

//******************************************************************************
//  Internal function definitions
//******************************************************************************

bool mcc_data_array_initalize(mcc_Config_t *config) {
	if (data.is_initialized)
		return false;

	size_t number_of_particles = config->particle_count;
	srand(time(NULL));

	mcc_Particle_t *particle_positions =
	    malloc(number_of_particles * sizeof(mcc_Particle_t));
	if (particle_positions == NULL) {
		puts("Error in particle buffer allocation");
		return false;
	}

	data.number_of_particles = number_of_particles;
	data.particle_positions = particle_positions;

	data.is_initialized = true;

	return data.is_initialized;
}

bool mcc_data_array_finalize() {
	free(data.particle_positions);
	data.is_initialized = false;
	return true;
}

mcc_Particle_t *mcc_data_array_get_particle(int index, mcc_Config_t *config) {
	(void)config;
	if (index < 0)
		index = rand() % data.number_of_particles;

	if ((size_t)index < data.number_of_particles)
		return &data.particle_positions[index];

	char err_msg[256];
	snprintf(err_msg, 256, "Particle index '%i' is out of bounds", index);
	mcc_panic(MCC_ERR_INDEX_OUT_OF_BOUNDS, err_msg);
}

bool mcc_data_array_set_particle(int index, mcc_Particle_t particle,
                                 mcc_Config_t *config) {
	(void)config;
	if (index < 0 || (size_t)index >= data.number_of_particles) {
		fprintf(stderr, "Particle index '%i' is out of bounds (%zu).\n", index,
		        data.number_of_particles);
		return false;
	}

	data.particle_positions[index] = particle;
	return true;
}

mcc_Particle_t *mcc_data_array_iterator_next() {
	return (state.current <= data.number_of_particles - 1)
	           ? &data.particle_positions[state.current++]
	           : NULL;
}

bool mcc_data_array_iterator_is_done(mcc_Particle_t *result) {
	return result == NULL;
}
