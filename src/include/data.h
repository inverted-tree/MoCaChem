#pragma once

#include "config.h"
#include "utils.h"
#include <stdbool.h>

//******************************************************************************
//  Interface datatype declarations
//******************************************************************************

typedef struct mcc_Index_t mcc_Index_t;
struct mcc_Index_t {
	int idx;
};

typedef struct {
	double x, y, z;
} mcc_Particle_t;

typedef struct {
	void *state;
	mcc_Particle_t *(*next)();
	bool (*is_done)(mcc_Particle_t *result);
} mcc_Particle_Iterator_t;

typedef struct {
	mcc_Status_t (*init)(mcc_Config_t *config);
	mcc_Status_t (*destroy)();
	mcc_Particle_t *(*get_particle)(mcc_Index_t *index);
	bool (*set_particle)(mcc_Index_t *index, mcc_Particle_t *particle);
} mcc_DAF_t;

//******************************************************************************
//  Interface function declarations
//******************************************************************************

mcc_DAF_t mcc_data_get_access_fs();

mcc_Particle_Iterator_t mcc_data_get_iterator(mcc_Index_t *index,
                                              mcc_Config_t *config);

mcc_Index_t mcc_data_gen_index(int idx);
