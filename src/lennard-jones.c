#include "lennard-jones.h"
#include <math.h>
#include <stdlib.h>

//******************************************************************************
// Helper Function Declarations
//******************************************************************************

static mcc_Energy_t mcc_lennard_jones_potential(mcc_Particle_t *p1,
                                                mcc_Particle_t *p2,
                                                mcc_Config_t *config);

//******************************************************************************
// Interface Function Definitions
//******************************************************************************

mcc_Energy_t mcc_lennard_jones_particle_potential(int index,
                                                  mcc_Particle_t *particle,
                                                  mcc_Config_t *config) {
	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();
	mcc_Energy_t energy = {
	    .lennard_jones = 0.0,
	    .virial = 0.0,
	};

	for (int i = 0; i < config->particle_count; i++) {
		if (i == index)
			continue;
		mcc_Energy_t contribution = mcc_lennard_jones_potential(
		    particle, fs.get_particle(i, config), config);
		energy.lennard_jones += contribution.lennard_jones;
		energy.virial += contribution.virial;
	}

	return energy;
}

mcc_Energy_t mcc_lennard_jones_system_potential(mcc_Config_t *config) {
	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();
	mcc_Energy_t energy = {
	    .lennard_jones = 0.0,
	    .virial = 0.0,
	};

	for (int i = 0; i < config->particle_count - 1; i++) {
		for (int j = i + 1; j < config->particle_count; j++) {
			mcc_Energy_t contribution = mcc_lennard_jones_potential(
			    fs.get_particle(i, config), fs.get_particle(j, config), config);
			energy.lennard_jones += contribution.lennard_jones;
			energy.virial += contribution.virial;
		}
	}

	energy.lennard_jones += config->particle_count * config->lj_error;
	return energy;
}

//******************************************************************************
// Helper Function Definitions
//******************************************************************************

static mcc_Energy_t mcc_lennard_jones_potential(mcc_Particle_t *p1,
                                                mcc_Particle_t *p2,
                                                mcc_Config_t *config) {
	mcc_Particle_t r = {
	    .x = p1->x - p2->x,
	    .y = p1->y - p2->y,
	    .z = p1->z - p2->z,
	};

	r.x -= config->box_length * round(r.x / config->box_length);
	r.y -= config->box_length * round(r.y / config->box_length);
	r.z -= config->box_length * round(r.z / config->box_length);

	double distance_2 = r.x * r.x + r.y * r.y + r.z * r.z;

	double const eps = 1E-9;
	distance_2 = fmax(distance_2, eps);
	mcc_Energy_t energy = {
	    .lennard_jones = 0.0,
	    .virial = 0.0,
	};
	if (distance_2 > config->cutoff_dist * config->cutoff_dist)
		return energy;

	double oo_distance_6 = 1.0 / (distance_2 * distance_2 * distance_2);
	double oo_distance_12 = oo_distance_6 * oo_distance_6;

	energy.lennard_jones =
	    4 * (oo_distance_12 - oo_distance_6) - config->lj_at_cutoff;
	energy.virial = 48 * (oo_distance_12 - 0.5 * oo_distance_6);

	return energy;
}
