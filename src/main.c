#include "config.h"
#include "csv-parser.h"
#include "data.h"
#include "lennard-jones.h"
#include "monte-carlo.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
// #include <notcurses/notcurses.h>

int main(int argc, char **argv) {
	mcc_CmdlArgs_t arguments = {0};
	mcc_Status_t status = mcc_utils_handle_args(&arguments, argc, argv);
	if (status.is == FAILURE)
		goto EXIT_ON_FAILURE;

	if (!arguments.minimal)
		mcc_utils_print_title_image();

	mcc_Config_t config = {0};
	status = mcc_utils_parse_config(&config, &arguments);
	if (status.is == FAILURE)
		goto EXIT_ON_FAILURE;

	mcc_DAF_t fs = mcc_data_get_access_fs();

	status = fs.init(&config);
	if (status.is == FAILURE)
		goto EXIT_ON_FAILURE;

	status = mcc_utils_init_state_space(&arguments, &config);
	if (status.is == FAILURE)
		goto EXIT_ON_FAILURE;

	// TODO: Experiment with the tui library notcurses:
	/**
	struct notcurses *nc;
	struct notcurses_options nc_opts = {.loglevel = NCLOGLEVEL_ERROR};
	nc = notcurses_init(&nc_opts, stdout);
	if (nc == NULL) {
	    fprintf(stderr, "Error initializing Notcurses\n");
	    return 1;
	}
	notcurses_clear(nc);
	// Wait for user input
	getchar(); // This will keep the terminal open until a key is pressed
	notcurses_stop(nc);
	*/

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
	mcc_csv_write_state_space("../../../../final.csv", &config);

	status = fs.destroy();
	if (status.is == FAILURE)
		goto EXIT_ON_FAILURE;

	return EXIT_SUCCESS;

EXIT_ON_FAILURE:
	puts(status.msg);
	mcc_utils_print_failure_msg();
	return EXIT_FAILURE;
}
