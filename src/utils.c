#include "utils.h"
#include "maybe.h"
#include "panic.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//******************************************************************************
//  Helper datatype declarations
//******************************************************************************

enum MCC_UTILS_ARG_TYPE {
	CONFIG_PATH,
	NONE,
};

//******************************************************************************
//  Helper function declarations
//******************************************************************************

static bool mcc_utils_file_exists(char const *path);

static mcc_Maybe mcc_utils_find_file_in_subdirs(char *const base_dir,
                                                char const *file,
                                                unsigned depth);

static char *mcc_utils_get_default_config_path();

static char *mcc_utils_get_project_root_dir();

static enum MCC_UTILS_ARG_TYPE mcc_utils_map_arg_to_enum(char const *arg);

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_CmdlOpts mcc_utils_handle_command_line_args(int argc, char **argv) {
	char *root = mcc_utils_get_project_root_dir();
	mcc_Maybe root_dir = mcc_just_string(root);
	free(root);

	mcc_Maybe config = mcc_nothing();
	for (size_t arg = 1; (const int)arg < argc; arg++) {
		char err_msg[256];
		switch (mcc_utils_map_arg_to_enum(argv[arg])) {
		case CONFIG_PATH:
			if (arg + 1 >= argc)
				mcc_panic(
				    MCC_ERR_FILE_NOT_FOUND,
				    "No file provided after custom config option was set");

			char argstr[PATH_MAX];
			strcpy(argstr, argv[arg + 1]);
			if (mcc_utils_file_exists(argstr)) {
				config = mcc_just_string(argstr);
				arg++;
				continue;
			}

			char appended_argstr[PATH_MAX];
			snprintf(appended_argstr, PATH_MAX, "%s/%s",
			         mcc_from_just_string(root_dir), argstr);
			if (mcc_utils_file_exists(appended_argstr)) {
				char *clean_argstr = realpath(appended_argstr, NULL);
				config = mcc_just_string(clean_argstr);
				free(clean_argstr);
				arg++;
				continue;
			}

			snprintf(err_msg, sizeof(err_msg), "No such config file '%s'",
			         argstr);
			mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
			break;
		case NONE:
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

	mcc_CmdlOpts result = {
	    .root_dir = root_dir,
	    .config = config,
	};
	return result;
}

//******************************************************************************
//  Helper function definitions
//******************************************************************************

static mcc_Maybe mcc_utils_find_file_in_subdirs(char *const base_dir,
                                                char const *file,
                                                unsigned depth) {
	char *current_dir = realpath(base_dir, NULL);
	if (!current_dir)
		return mcc_nothing();

	while (current_dir && depth > 0) {
		char test_file_path[PATH_MAX];
		snprintf(test_file_path, sizeof(test_file_path), "%s/%s", current_dir,
		         file);

		if (mcc_utils_file_exists(test_file_path)) {
			free(current_dir);
			return mcc_just_string(test_file_path);
		}

		char *last_divider = strrchr(current_dir, '/');
		if (last_divider) {
			*last_divider = '\0';
		} else {
			free(current_dir);
			return mcc_nothing();
		}

		depth--;
	}

	return mcc_nothing();
}

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
	} else {
		return NONE;
	}
}
