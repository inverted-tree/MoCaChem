#pragma once

//*****************************************************************************
//  Global Datatype Declaration
//******************************************************************************

typedef struct {
	int monte_carlo_steps;
	int equilibrium_steps;
	int particle_count;

	double fluid_density;
	double fluid_temp;

	double max_displ;
	double cutoff_dist;
	double box_volume;
	double box_length;

	double lj_error;
	double p_error;
	double lj_at_cutoff;
} mcc_Config_t;
