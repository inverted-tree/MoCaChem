#pragma once

#include "config.h"
#include <stdbool.h>

//******************************************************************************
//  Interface datatype declarations
//******************************************************************************

typedef struct {
	double x, y, z;
} mcc_Particle_t;

typedef struct {
	void *state;
	mcc_Particle_t *(*next)();
	bool (*is_done)(mcc_Particle_t *result);
} mcc_Particle_Iterator_t;

typedef struct {
	bool (*initialize)(mcc_Config_t *config);
	bool (*finalize)();
	mcc_Particle_t *(*get_particle)(int index);
	bool (*set_particle)(int index, mcc_Particle_t *particle);
} mcc_Particle_Access_Functions_t;

//******************************************************************************
//  Interface function declarations
//******************************************************************************

mcc_Particle_Access_Functions_t mcc_data_get_access_functions();

mcc_Particle_Iterator_t mcc_data_get_iterator(int index, mcc_Config_t *config);
