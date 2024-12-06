#include "monte-carlo.h"
#include "config.h"
#include "data.h"
#include "lennard-jones.h"
#include <math.h>
#include <stdlib.h>

//******************************************************************************
// Helper Function Declarations
//******************************************************************************

static void mcc_monte_carlo_apply_boundary_condition(mcc_Particle_t *particle,
                                                     double box_length);

static mcc_Particle_t
mcc_monte_carlo_displace_particle(mcc_Particle_t const *particle,
                                  double max_displacement, double box_length);

//******************************************************************************
// Interface Function Definitions
//******************************************************************************

bool mcc_monte_carlo_move(mcc_Energy_t *energy, mcc_Config_t *config) {
	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();

	int index = rand() % config->particle_count;
	mcc_Particle_t *old_particle = fs.get_particle(index);
	mcc_Energy_t old_energy =
	    mcc_lennard_jones_particle_potential(index, old_particle, config);

	mcc_Particle_t new_particle = mcc_monte_carlo_displace_particle(
	    old_particle, config->max_displ, config->box_length);

	mcc_Energy_t new_energy =
	    mcc_lennard_jones_particle_potential(index, &new_particle, config);

	double chance = (double)rand() / RAND_MAX;
	if (chance < exp(-(new_energy.lennard_jones - old_energy.lennard_jones) /
	                 config->fluid_temp)) {
		fs.set_particle(index, &new_particle);
		energy->lennard_jones +=
		    new_energy.lennard_jones - old_energy.lennard_jones;
		energy->virial += new_energy.virial - old_energy.virial;

		return true;
	}
	return false;
}

//******************************************************************************
// Helper Function Definitions
//******************************************************************************

static mcc_Particle_t
mcc_monte_carlo_displace_particle(mcc_Particle_t const *particle,
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
