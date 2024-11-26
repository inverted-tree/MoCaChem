#include "data.h"
#include "utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts_t args = mcc_utils_handle_command_line_args(argc, argv);
	mcc_utils_print_title_image();
	mcc_Config_t config = mcc_utils_init_config(args);

	mcc_Particle_Access_Functions_t functions = mcc_data_get_access_functions();

	bool no_error = true;
	no_error &= functions.initialize(config);
	for (size_t i = 0; i < config.particle_count; i++) {
		mcc_Particle_t test_particle = {
		    (int)i,
		};
		no_error &= functions.set_particle(i, test_particle, config);
	}

	mcc_Particle_t *particle = NULL;
	mcc_Particle_Iterator_t iterator = mcc_data_get_iterator(0, config);
	size_t counter = 0;
	while (!iterator.is_done(particle = iterator.next())) {
		printf("The particle at position [%zu] has value: %i\n", counter,
		       *particle);
		counter++;
	}

	no_error &= functions.finalize();
	mcc_utils_destroy_command_line_options(args);

	return !no_error;
}
