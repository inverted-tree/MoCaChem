#include "toml-parser.h"
#include "utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts_t args = mcc_utils_handle_command_line_args(argc, argv);

	mcc_utils_print_title_image();

	mcc_Config_t config = mcc_utils_init_config(args);

	printf("The project root is at %s\n", mcc_from_just_string(args.root_dir));
	printf("The config file is %s\n", mcc_from_just_string(args.config));

	printf("mc_steps:..... %i\n", config.monte_carlo_steps);
	printf("eq_steps:..... %i\n", config.equilibrium_steps);
	printf("num_particles: %i\n", config.particle_count);
	printf("rho:.......... %f\n", config.fluid_density);
	printf("temp:......... %f\n", config.fluid_temp);
	printf("dr_max:....... %f\n", config.max_displ);
	printf("dr_cut:....... %f\n", config.cutoff_dist);
	printf("volume:....... %f\n", config.box_volume);
	printf("dl_box:....... %f\n", config.box_length);
	printf("v_corr:....... %f\n", config.virial_corr);
	printf("p_corr:....... %f\n", config.lj_pot_corr);

	return 0;
}
