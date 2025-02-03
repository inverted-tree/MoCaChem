#include "utils.h"
#include "config.h"
#include "csv-parser.h"
#include "data.h"
#include "include/data.h"
#include "include/utils.h"
#include "toml-parser.h"
#include <assert.h>
#include <dirent.h>
#include <math.h>
#include <stddef.h>
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
	NO_TUI,
	NONE,
};

struct mcc_CmdlOptsTracker_t {
	bool config_path;
	bool no_tui;
	bool particle_config;
};

//******************************************************************************
//  Helper Function Declarations
//******************************************************************************

mcc_Status_t mcc_utils_extract_filepath(char path[static 1], size_t path_length,
                                        int arg, int argc, char **argv);

static bool mcc_utils_file_exists(char const path[static 1]);

static mcc_Status_t mcc_utils_get_default_config_path(char path[static 1],
                                                      size_t path_length);

static mcc_Status_t mcc_utils_get_project_root_dir(char root[static 1],
                                                   size_t root_length);

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg);

static void mcc_utils_print_help_message();

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

mcc_Status_t mcc_utils_status_success() {
	mcc_Status_t s = {
	    .is = SUCCESS,
	    .msg = "",
	};
	return s;
}

mcc_Status_t mcc_utils_status_failure(char const msg[static 1]) {
	mcc_Status_t s = {
	    .is = FAILURE,
	};
	strncpy(s.msg, msg, STATUS_MSG_SIZE);
	return s;
}

mcc_Status_t mcc_utils_extract_filepath(char path[static 1], size_t path_length,
                                        int arg, int argc, char **argv) {
	if (arg + 1 >= argc || mcc_utils_map_arg_to_enum(argv[arg + 1]) != NONE) {
		static char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "No file path provided after argument '%s'.",
		         argv[arg]);
		return mcc_utils_status_failure(msg);
	}

	static char real_path[PATH_MAX];
	realpath(argv[arg + 1], real_path);

	if (mcc_utils_file_exists(real_path)) {
		int i = snprintf(path, path_length, "%s", real_path);
		if ((size_t)i >= path_length) {
			static char msg[STATUS_MSG_SIZE];
			snprintf(msg, sizeof(msg),
			         "Buffer length of '%zu B' exceeded by '%zu B'. File "
			         "path string truncated.",
			         path_length, i - path_length);
			return mcc_utils_status_failure(msg);
		} else if (i < 0) {
			return mcc_utils_status_failure(
			    "Formatting of the file path string failed.");
		}
		return mcc_utils_status_success();
	} else {
		static char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "File '%s' does not exist.", real_path);
		return mcc_utils_status_failure(msg);
	}

	static char root[PATH_MAX];
	static char argstr[PATH_MAX];
	mcc_Status_t s = mcc_utils_get_project_root_dir(root, sizeof(root));
	if (s.is == FAILURE)
		return s;
	int i = snprintf(argstr, sizeof(argstr), "%s/%s", root, argv[arg + 1]);
	if (i >= sizeof(argstr)) {
		static char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of '%zu B' exceeded by '%zu B'. File "
		         "path string truncated.",
		         sizeof(argstr), i - sizeof(argstr));
		return mcc_utils_status_failure(msg);
	} else if (i < 0) {
		return mcc_utils_status_failure(
		    "Formatting of the file path string failed.");
	}

	if (!realpath(argstr, real_path))
		return mcc_utils_status_failure(
		    "Real path for '%s' could not be evaluated.");

	if (!mcc_utils_file_exists(real_path)) {
		static char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "File '%s' does not exist.", real_path);
		return mcc_utils_status_failure(msg);
	}

	snprintf(path, path_length, "%s", real_path);
	if (i >= path_length) {
		static char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of '%zu B' exceeded by '%zu B'. File "
		         "path string truncated.",
		         path_length, i - path_length);
		return mcc_utils_status_failure(msg);
	} else if (i < 0) {
		return mcc_utils_status_failure(
		    "Formatting of the file path string failed.");
	}

	return mcc_utils_status_success();
}

mcc_Status_t mcc_utils_handle_args(mcc_CmdlArgs_t *args, int argc,
                                   char **argv) {
	struct mcc_CmdlOptsTracker_t options_tracker = {};
	static char conf_file[PATH_MAX];
	static char particle_file[PATH_MAX];
	mcc_Status_t s;

	for (size_t arg = 1; arg < (size_t)argc; arg++) {
		switch (mcc_utils_map_arg_to_enum(argv[arg])) {
		case CONFIG_PATH:
			if (options_tracker.config_path) {
				static char msg[STATUS_MSG_SIZE];
				snprintf(msg, sizeof(msg),
				         "Config Path option already set to %s", conf_file);
				return mcc_utils_status_failure(msg);
			}

			s = mcc_utils_extract_filepath(conf_file, sizeof(conf_file), arg,
			                               argc, argv);
			if (s.is == FAILURE)
				return s;

			options_tracker.config_path = true;
			arg++;
			break;

		case HELP:
			mcc_utils_print_help_message();
			exit(EXIT_SUCCESS);

		case NO_TUI:
			/* This does nothing as of now since the TUI implementation is
			 * not yet complete. */
			break;

		case PARTICLE_POSITIONS:
			if (options_tracker.particle_config) {
				static char msg[STATUS_MSG_SIZE];
				snprintf(msg, sizeof(msg),
				         "Particle position option already set to %s",
				         particle_file);
				return mcc_utils_status_failure(msg);
			}

			s = mcc_utils_extract_filepath(particle_file, sizeof(particle_file),
			                               arg, argc, argv);
			if (s.is == FAILURE)
				return s;

			options_tracker.particle_config = true;
			arg++;
			break;

		case NONE:
			mcc_utils_print_help_message();
			puts("");
			fprintf(stderr, "Unknown argument '%s'", argv[arg]);
			exit(EXIT_FAILURE);
		}
	}

	if (conf_file[0] == '\0') {
		static char path[PATH_MAX];
		s = mcc_utils_get_default_config_path(path, sizeof(path));
		if (s.is == FAILURE)
			return s;
		snprintf(conf_file, sizeof(conf_file), "%s", path);
	}

	int i =
	    snprintf(args->config_file, sizeof(args->config_file), "%s", conf_file);
	if ((size_t)i >= sizeof(args->config_file)) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of '%zu B' exceeded by '%zu B'. Config file "
		         "path string truncated.",
		         sizeof(args->config_file), i - sizeof(args->config_file));
		return mcc_utils_status_failure(msg);
	}

	i = snprintf(args->particle_file, sizeof(args->particle_file), "%s",
	             particle_file);
	if ((size_t)i >= sizeof(args->config_file)) {

		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of '%zu B' exceeded by '%zu B'. Particle data "
		         "file path string truncated.",
		         sizeof(args->config_file), i - sizeof(args->config_file));
		return mcc_utils_status_failure(msg);
	}

	return s;
}

mcc_Status_t mcc_utils_parse_config(mcc_Config_t config[static 1],
                                    mcc_CmdlArgs_t const args[static 1]) {
	mcc_Status_t s = mcc_toml_parse_config(config, args->config_file);

	config->box_volume = config->particle_count / config->fluid_density;
	config->box_length = pow(config->box_volume, 1.0 / 3);

	double const rr3 = 1.0 / pow(config->cutoff_dist, 3.0);
	config->lj_error =
	    8.0 * M_PI * config->fluid_density * (pow(rr3, 3.0) / 9 - rr3 / 3);
	config->p_error = 16.0 / 3 * M_PI * pow(config->fluid_density, 2.0) *
	                  (2.0 / 3 * pow(rr3, 3.0) - rr3);
	config->lj_at_cutoff = 4 * (1.0 / pow(config->cutoff_dist, 12) -
	                            1.0 / pow(config->cutoff_dist, 6));

	return s;
}

mcc_Status_t mcc_utils_init_state_space(mcc_CmdlArgs_t const *args,
                                        mcc_Config_t const *conf) {
	mcc_DAF_t fs = mcc_data_get_access_fs();

	if (args->particle_file[0] == '\0') {
		/*
		 * initialize the state space as a regular grid distribution of
		 * particles
		 */
		size_t res = 2; /* initial lattice resolution */
		while (res * res * res < (size_t)conf->particle_count)
			res++;

		double dl_cell = conf->box_length / res; /* length of a lattice cell */
		for (size_t i = 0; i < (size_t)conf->particle_count; i++) {
			size_t x = i % res;
			size_t y = (i / res) % res;
			size_t z = i / (res * res);

			mcc_Particle_t particle = {
			    (x + 0.5) * dl_cell,
			    (y + 0.5) * dl_cell,
			    (z + 0.5) * dl_cell,
			};
			mcc_Index_t idx = {.idx = i};
			bool test = fs.set_particle(&idx, &particle);
			assert(test);
		}
	} else {
		return mcc_csv_load_state_space(args->particle_file, conf);
	}

	return mcc_utils_status_success();
}

//******************************************************************************
//  Helper Function Definitions
//******************************************************************************

static bool mcc_utils_file_exists(char const path[static 1]) {
	return access(path, F_OK) ? false : true;
}

static mcc_Status_t mcc_utils_get_default_config_path(char path[static 1],
                                                      size_t path_length) {
	char root[PATH_MAX];
	memset(path, 0, path_length);
	memset(root, 0, PATH_MAX);

	mcc_Status_t s = mcc_utils_get_project_root_dir(root, sizeof(root));
	if (s.is == FAILURE)
		return s;

	int i = snprintf(path, path_length, "%s/config.toml", root);
	if ((size_t)i >= path_length) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of '%zu B' exceeded by '%zu B'. Path string "
		         "truncated.",
		         path_length, i - path_length);
		return mcc_utils_status_failure(msg);
	} else if (i < 0) {
		return mcc_utils_status_failure(
		    "Formatting of the path string failed.");
	}

	if (mcc_utils_file_exists(path)) {
		return mcc_utils_status_success();
	} else {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "Default config file '%s' does not exist.",
		         path);
		return mcc_utils_status_failure(msg);
	}
}

static mcc_Status_t mcc_utils_get_project_root_dir(char root[static 1],
                                                   size_t root_length) {
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		return mcc_utils_status_failure(
		    "Could not determine the current working directory.");

	// TODO: It would be better to implement this as a recursive descent of
	// max 4 directories and check each time if the root dir has been
	// reached. This way we do not rely on the xmake dir structure.
	char xmake_root[PATH_MAX];
	snprintf(xmake_root, sizeof(xmake_root), "%s/../../../..", cwd);
	char real_xmake_root[PATH_MAX];

	if (realpath(xmake_root, real_xmake_root) == NULL) {
		return mcc_utils_status_failure(
		    "Formatting of the project root path string failed.");
	}

	int i = snprintf(root, root_length, "%s", real_xmake_root);
	if ((size_t)i >= root_length) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Buffer length of 'PATH_MAX=%i B' exceeded by '%i B'. Root "
		         "path string truncated.",
		         PATH_MAX, i - PATH_MAX);
		return mcc_utils_status_failure(msg);
	} else if (i < 0) {
		return mcc_utils_status_failure(
		    "Formatting of the project root path string failed.");
	}

	DIR *dir = opendir(root);
	if (dir) {
		closedir(dir);
		return mcc_utils_status_success();
	} else {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Formatted project root directory '%s' does not exist.", root);
		return mcc_utils_status_failure(msg);
	}
}

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg) {
	if (!strcmp(arg, "-c") || !strcmp(arg, "--config")) {
		return CONFIG_PATH;
	} else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
		return HELP;
	} else if (!strcmp(arg, "-p") || !strcmp(arg, "--particles")) {
		return PARTICLE_POSITIONS;
	} else if (!strcmp(arg, "-t") || !strcmp(arg, "--no-tui")) {
		return NO_TUI;
	} else {
		return NONE;
	}
}

//******************************************************************************
//  I/O Function Definitions
//******************************************************************************

void mcc_utils_print_title_image() {
	char *title = "\033[34m ______  ___     _________      ______________  "
	              "              "
	              "  \n"
	              " ___   |/  /_______  ____/_____ __  ____/__  "
	              "/_____________ ___   \n"
	              " __  /|_/ /_  __ \\  /    _  __ `/  /    __  __ \\  _ "
	              "\\_  __ `__ \\ "
	              "\n"
	              " _  /  / / / /_/ / /___  / /_/ // /___  _  / / /  __/  "
	              "/ / / / / \n"
	              " /_/  /_/  \\____/\\____/  \\__,_/ \\____/  /_/ "
	              "/_/\\___//_/ /_/ /_/\n"
	              "————————————————————————————————————————————————————————"
	              "———————\033["
	              "33m\n"
	              " Version: none\n"
	              " Copyright: Lukas Schröder 2025\n"
	              " License: MIT\n\033[0m";
	puts(title);
}

void mcc_utils_print_progress_bar(int progress, int total) {
	int percentage = (progress * 100) / total;
	int bar_width = 58;
	int progress_chars = (progress * bar_width) / total;

	if (progress == total)
		printf("\033[32m");
	else
		printf("\033[33m");
	printf("\n\n   ");
	for (int i = 0; i < bar_width; i++) {
		if (i < progress_chars) {
			printf("▓");
		} else {
			printf("░");
		}
	}
	printf(" %d%%\033[0m", percentage);

	fflush(stdout);
}

void mcc_utils_print_failure_msg() {
	puts("MoCaChem encountered a runtime error. If you think this is a bug, "
	     "please upload your configuration file with a description of the "
	     "error to the issue tracker at "
	     "https://github.com/inverted-tree/MoCaChem/issues");
}

static void mcc_utils_print_help_message() {
	printf(" Usage: MoCaChem [options]\n\n"
	       " Description:\n"
	       "  MoCaChem is a Monte Carlo simulation to estimate the chemical "
	       "potential of a fluid using Lennard-Jones interactions via "
	       "particle "
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
