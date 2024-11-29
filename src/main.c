#include "data.h"
#include "lennard-jones.h"
#include "monte-carlo.h"
#include "utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts_t args = mcc_utils_handle_command_line_args(argc, argv);
	mcc_utils_print_title_image();
	mcc_Config_t config = mcc_utils_init_config(args);

	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();

	bool no_error = true;
	no_error &= fs.initialize(&config);

	size_t res = 2; /* The initial particle lattice resolution */
	while (res * res * res < (size_t)config.particle_count)
		res++;

	double dl_cell = config.box_length / res; /* The length of a lattice cell */
	for (size_t i = 0; i < (size_t)config.particle_count; i++) {
		size_t x = i % res;
		size_t y = (i / res) % res;
		size_t z = i / (res * res);

		mcc_Particle_t particle = {
		    (x + 0.5) * dl_cell,
		    (y + 0.5) * dl_cell,
		    (z + 0.5) * dl_cell,
		};
		no_error &= fs.set_particle(i, particle, &config);
	}

	mcc_Energy_t global_energy = mcc_lennard_jones_system_potential(&config);
	fprintf(stdout, "The initial chemical potential is %f\n",
	        global_energy.lennard_jones / config.particle_count);

	size_t accepted_moves = 0;
	for (int i = 0; i < config.monte_carlo_steps; i++) {
		accepted_moves += mcc_monte_carlo_move(&global_energy, &config);
		if (i % 50000 == 0)
			printf("Move [%i]: LJ-Potential = %f\n", i,
			       global_energy.lennard_jones / config.particle_count);
	}

	no_error &= fs.finalize();
	mcc_utils_destroy_command_line_options(args);

	return !no_error;
}
