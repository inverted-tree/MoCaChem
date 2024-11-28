#include "monte-carlo.h"
#include "config.h"
#include "data.h"
#include <math.h>
#include <stdlib.h>

//******************************************************************************
// Helper Function Declarations
//******************************************************************************

static void mcc_monte_carlo_apply_boundary_condition(mcc_Particle_t *particle,
                                                     double box_length);

static mcc_Particle_t
mcc_monte_carlo_displace_particle(mcc_Particle_t *particle,
                                  double max_displacement, double box_length);

double mcc_lennard_jones_potential() { return 0.0; }

//******************************************************************************
// Interface Function Definitions
//******************************************************************************

void mcc_monte_carlo_move(mcc_Config_t *config) {
	mcc_Particle_Access_Functions_t functions = mcc_data_get_access_functions();

	size_t index = rand() % config->particle_count;
	mcc_Particle_t *old_particle = functions.get_particle(index, config);
	double old_energy = mcc_lennard_jones_potential();

	mcc_Particle_t new_particle = mcc_monte_carlo_displace_particle(
	    old_particle, config->max_displ, config->box_length);
	double new_energy = mcc_lennard_jones_potential();

	double chance = (double)rand() / RAND_MAX;
	if (chance < exp(-(new_energy - old_energy) / config->fluid_temp)) {
		functions.set_particle(index, new_particle, config);
	}
}

//******************************************************************************
// Helper Function Definitions
//******************************************************************************

static mcc_Particle_t
mcc_monte_carlo_displace_particle(mcc_Particle_t *particle,
                                  double max_displacement, double box_length) {
	mcc_Particle_t displaced_particle = {
	    .x = particle->x +
	         max_displacement * 2 * (0.5 - (double)rand() / RAND_MAX),
	    .y = particle->y +
	         max_displacement * 2 * (0.5 - (double)rand() / RAND_MAX),
	    .z = particle->z +
	         max_displacement * 2 * (0.5 - (double)rand() / RAND_MAX),
	};
	mcc_monte_carlo_apply_boundary_condition(&displaced_particle, box_length);
	return displaced_particle;
}

static void mcc_monte_carlo_apply_boundary_condition(mcc_Particle_t *particle,
                                                     double box_length) {
	if (particle->x > box_length) {
		particle->x -= box_length;
	} else if (particle->x < 0.0) {
		particle->x += box_length;
	}

	if (particle->y > box_length) {
		particle->y -= box_length;
	} else if (particle->y < 0.0) {
		particle->y += box_length;
	}

	if (particle->z > box_length) {
		particle->z -= box_length;
	} else if (particle->z < 0.0) {
		particle->z += box_length;
	}
}
