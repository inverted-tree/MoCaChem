#include "utils.h"
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
	NONE,
};

struct mcc_CmdlOptsTracker_t {
	bool config_path;
};

//******************************************************************************
//  Helper Function Declarations
//******************************************************************************

static bool mcc_utils_file_exists(char const *path);

static char *mcc_utils_get_default_config_path();

static char *mcc_utils_get_project_root_dir();

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg);

static void mcc_utils_print_help_message();

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

void mcc_utils_destroy_command_line_options(mcc_CmdlOpts_t options) {
	free(options.root_dir.str_value);
	free(options.config.str_value);
}

mcc_CmdlOpts_t mcc_utils_handle_command_line_args(int argc, char **argv) {
	char *root = mcc_utils_get_project_root_dir();
	mcc_Maybe_t root_dir = mcc_just_string(root);
	free(root);

	struct mcc_CmdlOptsTracker_t options_tracker = {};
	mcc_Maybe_t config = mcc_nothing();
	for (size_t arg = 1; arg < (size_t)argc; arg++) {
		char err_msg[256];
		switch (mcc_utils_map_arg_to_enum(argv[arg])) {
		case CONFIG_PATH:
			if (options_tracker.config_path)
				mcc_panic(MCC_ERR_IO, "Config Path option already set");

			if ((int)arg + 1 >= argc) {
				snprintf(err_msg, sizeof(err_msg),
				         "No file provided after argument '%s'", argv[arg]);
				mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
			}

			char argstr[PATH_MAX];
			strcpy(argstr, argv[arg + 1]);
			if (mcc_utils_file_exists(argstr)) {
				config = mcc_just_string(argstr);
				options_tracker.config_path = true;
				arg++;
				continue;
			}

			char appended_argstr[PATH_MAX];
			snprintf(appended_argstr, PATH_MAX, "%s/%s",
			         mcc_from_just_string(root_dir), argstr);
			if (mcc_utils_file_exists(appended_argstr)) {
				char *clean_argstr = realpath(appended_argstr, NULL);
				if (!clean_argstr) {
					snprintf(err_msg, sizeof(err_msg),
					         "Failed to resolve realpath for '%s'",
					         appended_argstr);
					mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
				}
				config = mcc_just_string(clean_argstr);
				free(clean_argstr);
				options_tracker.config_path = true;
				arg++;
				continue;
			}

			snprintf(err_msg, sizeof(err_msg), "No such config file '%s'",
			         argstr);
			mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
			break;
		case HELP:
			mcc_utils_print_help_message();
			exit(EXIT_SUCCESS);
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
	    .root_dir = root_dir,
	    .config = config,
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
	config.virial_corr =
	    8.0 * M_PI * config.fluid_density * (pow(rr3, 3.0) / 9 - rr3 / 3);
	config.lj_pot_corr = 16.0 / 3 * M_PI * pow(config.fluid_density, 2.0) *
	                     (2.0 / 3 * pow(rr3, 3.0) - rr3);

	return config;
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

static bool mcc_utils_file_exists(char const *path) {
	return access(path, F_OK) ? false : true;
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
	       "  -h / --help    	Show this help message and exit.\n\n"
	       " Example:\n"
	       "  MoCaChem\n"
	       "  MoCaChem --config ./configs/config-1.toml\n\n"
	       " Please report bugs to the issue tracker at "
	       "https://github.com/inverted-tree/MoCaChem/issues\n");
}
