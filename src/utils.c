#include "utils.h"
#include "csv-parser.h"
#include "data.h"
#include "maybe.h"
#include "panic.h"
#include "toml-parser.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//******************************************************************************
//  Helper Datatype Declarations
//******************************************************************************

enum MCC_UTILS_ARG_TYPE {
	CONFIG_PATH,
	HELP,
	PARTICLE_POSITIONS,
	NONE,
};

struct mcc_CmdlOptsTracker_t {
	bool config_path;
	bool particle_config;
};

//******************************************************************************
//  Helper Function Declarations
//******************************************************************************

mcc_Maybe_t mcc_utils_extract_filepath(int arg, int argc, char **argv);

static bool mcc_utils_file_exists(mcc_Maybe_t path);

static char *mcc_utils_get_default_config_path();

static char *mcc_utils_get_project_root_dir();

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg);

static void mcc_utils_print_help_message();

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

void mcc_utils_destroy_command_line_args(mcc_CmdlOpts_t *options) {
	free(options->config.str_value);
	free(options->particles.str_value);
}

mcc_Maybe_t mcc_utils_extract_filepath(int arg, int argc, char **argv) {
	if (arg + 1 >= argc && mcc_utils_map_arg_to_enum(argv[arg + 1]) != NONE) {
		char err_msg[256];
		snprintf(err_msg, 256, "No file path provided after argument '%s'",
		         argv[arg]);
		mcc_panic(MCC_ERR_UNKNOWN_ARGUMENT, err_msg);
	}

	char real_path[PATH_MAX];
	realpath(argv[arg + 1], real_path);
	mcc_Maybe_t path = mcc_just_string(real_path);

	if (mcc_utils_file_exists(path))
		return path;

	char *root = mcc_utils_get_project_root_dir();
	char argstr[PATH_MAX];
	snprintf(argstr, PATH_MAX, "%s/%s", root, argv[arg + 1]);
	free(root);
	realpath(argstr, real_path);
	path = mcc_just_string(real_path);

	if (mcc_utils_file_exists(path))
		return path;
	else
		return mcc_nothing();
}

mcc_CmdlOpts_t mcc_utils_handle_command_line_args(int argc, char **argv) {
	struct mcc_CmdlOptsTracker_t options_tracker = {};
	mcc_Maybe_t config = mcc_nothing();
	mcc_Maybe_t particles = mcc_nothing();

	for (size_t arg = 1; arg < (size_t)argc; arg++) {
		char err_msg[256];
		switch (mcc_utils_map_arg_to_enum(argv[arg])) {
		case CONFIG_PATH:
			if (options_tracker.config_path)
				mcc_panic(MCC_ERR_IO, "Config Path option already set");

			config = mcc_utils_extract_filepath(arg, argc, argv);
			if (mcc_is_nothing(config)) {
				snprintf(err_msg, sizeof(err_msg), "Config file '%s' not found",
				         argv[arg + 1]);
				mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
			}

			options_tracker.config_path = true;
			arg++;
			break;
		case HELP:
			mcc_utils_print_help_message();
			exit(EXIT_SUCCESS);
		case PARTICLE_POSITIONS:
			if (options_tracker.particle_config)
				mcc_panic(MCC_ERR_IO, "Particle position option already set");

			particles = mcc_utils_extract_filepath(arg, argc, argv);
			if (mcc_is_nothing(particles)) {
				snprintf(err_msg, sizeof(err_msg), "CSV file '%s' not found",
				         argv[arg + 1]);
				mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
			}

			options_tracker.particle_config = true;
			arg++;
			break;
		case NONE:
			mcc_utils_print_help_message();
			puts("");
			snprintf(err_msg, sizeof(err_msg), "Unknown argument '%s'",
			         argv[arg]);
			mcc_panic(MCC_ERR_UNKNOWN_ARGUMENT, err_msg);
		}
	}

	if (mcc_is_nothing(config)) {
		char *config_path = mcc_utils_get_default_config_path();
		config = mcc_just_string(config_path);
		free(config_path);
	}

	mcc_CmdlOpts_t result = {
	    .config = config,
	    .particles = particles,
	};
	return result;
}

mcc_Config_t mcc_utils_init_config(mcc_CmdlOpts_t args) {
	assert(!mcc_is_nothing(args.config));
	mcc_Config_t config =
	    mcc_toml_parse_config(mcc_from_just_string(args.config));

	config.box_volume = config.particle_count / config.fluid_density;
	config.box_length = pow(config.box_volume, 1.0 / 3);

	double const rr3 = 1.0 / pow(config.cutoff_dist, 3.0);
	config.lj_error =
	    8.0 * M_PI * config.fluid_density * (pow(rr3, 3.0) / 9 - rr3 / 3);
	config.p_error = 16.0 / 3 * M_PI * pow(config.fluid_density, 2.0) *
	                 (2.0 / 3 * pow(rr3, 3.0) - rr3);
	config.lj_at_cutoff = 4 * (1.0 / pow(config.cutoff_dist, 12) -
	                           1.0 / pow(config.cutoff_dist, 6));

	return config;
}

bool mcc_utils_init_particles(mcc_CmdlOpts_t *args, mcc_Config_t *config) {
	bool success = true;
	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();
	if (mcc_is_nothing(args->particles)) {
		size_t res = 2; /* The initial particle lattice resolution */
		while (res * res * res < (size_t)config->particle_count)
			res++;

		double dl_cell =
		    config->box_length / res; /* The length of a lattice cell */
		for (size_t i = 0; i < (size_t)config->particle_count; i++) {
			size_t x = i % res;
			size_t y = (i / res) % res;
			size_t z = i / (res * res);

			mcc_Particle_t particle = {
			    (x + 0.5) * dl_cell,
			    (y + 0.5) * dl_cell,
			    (z + 0.5) * dl_cell,
			};
			success &= fs.set_particle(i, particle, config);
		}
	} else {
		mcc_csv_read_particle_configuration(
		    mcc_from_just_string(args->particles), config);
	}

	mcc_utils_destroy_command_line_args(args);
	return success;
}

void mcc_utils_print_progress_bar(int progress, int total) {
	int percentage = (progress * 100) / total;
	int bar_width = 58;
	int progress_chars = (progress * bar_width) / total;

	printf("\n\n   ");
	for (int i = 0; i < bar_width; i++) {
		if (i < progress_chars) {
			printf("▓"); // ●");
		} else {
			printf("░"); // ○");
		}
	}
	printf(" %d%%", percentage);

	fflush(stdout);
}
void mcc_utils_print_title_image() {
	char *title =
	    " ______  ___     _________      ______________                  \n"
	    " ___   |/  /_______  ____/_____ __  ____/__  /_____________ ___   \n"
	    " __  /|_/ /_  __ \\  /    _  __ `/  /    __  __ \\  _ \\_  __ `__ \\ "
	    "\n"
	    " _  /  / / / /_/ / /___  / /_/ // /___  _  / / /  __/  / / / / / \n"
	    " /_/  /_/  \\____/\\____/  \\__,_/ \\____/  /_/ /_/\\___//_/ /_/ /_/\n"
	    "———————————————————————————————————————————————————————————————\n"
	    " Version: none\n"
	    " Copyright: Lukas Schröder 2024\n"
	    " License: MIT\n";
	puts(title);
}

//******************************************************************************
//  Helper Function Definitions
//******************************************************************************

static bool mcc_utils_file_exists(mcc_Maybe_t path) {
	return access(mcc_from_just_string(path), F_OK) ? false : true;
}

static char *mcc_utils_get_default_config_path() {
	char *root_dir = mcc_utils_get_project_root_dir();
	char *config_path = malloc(PATH_MAX * sizeof(char));
	if (!config_path)
		mcc_panic(MCC_ERR_MALLOC, "Memory allocation for string failed");

	snprintf(config_path, PATH_MAX, "%s/config.toml", root_dir);
	return config_path;
}

static char *mcc_utils_get_project_root_dir() {
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		mcc_panic(MCC_ERR_IO,
		          "Could not determine the current working directory");

	char messy_base[PATH_MAX];
	snprintf(messy_base, sizeof(messy_base), "%s/../../../..", cwd);
	char *base = realpath(messy_base, NULL);

	return base;
}

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg) {
	if (!strcmp(arg, "-c") || !strcmp(arg, "--config")) {
		return CONFIG_PATH;
	} else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
		return HELP;
	} else if (!strcmp(arg, "-p") || !strcmp(arg, "--particles")) {
		return PARTICLE_POSITIONS;
	} else {
		return NONE;
	}
}

static void mcc_utils_print_help_message() {
	printf(" Usage: MoCaChem [options]\n\n"
	       " Description:\n"
	       "  MoCaChem is a Monte Carlo simulation to estimate the chemical "
	       "potential of a fluid using Lennard-Jones interactions via particle "
	       "fluctuations in the system's volume.\n\n"
	       " Options:\n"
	       "  -c / --config		Pass a non-standrad config file to the "
	       "simulation.\n"
	       "  -h / --help    	Show this help message and exit.\n"
	       "  -p / --particles	Pass a particle position configuration in CSV "
	       "format to initialize the data.\n\n"
	       " Example:\n"
	       "  MoCaChem\n"
	       "  MoCaChem --config ./configs/config-1.toml\n\n"
	       " Please report bugs to the issue tracker at "
	       "https://github.com/inverted-tree/MoCaChem/issues\n");
}
