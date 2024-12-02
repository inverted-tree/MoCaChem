#include "config.h"
#include "csv-parser.h"
#include "data.h"
#include "lennard-jones.h"
#include "monte-carlo.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts_t args = mcc_utils_handle_command_line_args(argc, argv);
	mcc_utils_print_title_image();
	mcc_Config_t config = mcc_utils_init_config(args);

	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();
	bool success = true;
	success &= fs.initialize(&config);
	success &= mcc_utils_init_particles(&args, &config);

	mcc_Energy_t global_energy = mcc_lennard_jones_system_potential(&config);
	fprintf(stdout,
	        "   ┌──────────────────┬──────────────────┬──────────────────┐   \n"
	        "   │    < Energy >    │   < Pressure >   │ Acceptance Ratio │   \n"
	        "   ├──────────────────┼──────────────────┼──────────────────┤   \n"
	        "   │   %12.6f   │   %12.6f   │   %12.6f   │\n\n\n",
	        global_energy.lennard_jones / config.particle_count,
	        global_energy.virial / config.particle_count, 1.0);

	size_t accepted_moves = 0;
	for (int i = 0; i < config.monte_carlo_steps; i++) {
		accepted_moves += mcc_monte_carlo_move(&global_energy, &config);
		if (i > 0 && i % (int)(config.monte_carlo_steps / 1000) == 0) {
			printf("\033[A\033[A\033[A\r   │   %12.6f   │   %12.6f   │   "
			       "%12.6f   │  "
			       " \n",
			       global_energy.lennard_jones / config.particle_count,
			       global_energy.virial / config.particle_count,
			       (double)accepted_moves / i);
			printf("   "
			       "└──────────────────┴──────────────────┴──────────────────┘ "
			       "  ");
			mcc_utils_print_progress_bar(i, config.monte_carlo_steps);
			fflush(stdout);
		}
	}
	printf("\033[A\033[A");
	mcc_utils_print_progress_bar(config.monte_carlo_steps,
	                             config.monte_carlo_steps);
	printf("\n");
	mcc_csv_write_particle_configuration("../../../../final.csv", &config);

	success &= fs.finalize();

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
