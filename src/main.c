#include "data.h"
#include "monte-carlo.h"
#include "utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts_t args = mcc_utils_handle_command_line_args(argc, argv);
	mcc_utils_print_title_image();
	mcc_Config_t config = mcc_utils_init_config(args);

	mcc_Particle_Access_Functions_t functions = mcc_data_get_access_functions();

	bool no_error = true;
	no_error &= functions.initialize(&config);

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
		no_error &= functions.set_particle(i, particle, &config);
	}

	mcc_Particle_t *particle = NULL;
	mcc_Particle_Iterator_t iterator = mcc_data_get_iterator(0, &config);
	size_t counter = 0;
	while (!iterator.is_done(particle = iterator.next())) {
		printf("The particle at position [%zu] has value: (%f, %f, %f))\n",
		       counter, particle->x, particle->y, particle->z);
		counter++;
	}

	no_error &= functions.finalize();
	mcc_utils_destroy_command_line_options(args);

	return !no_error;
}
