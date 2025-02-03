#include "toml-parser.h"
#include "config.h"
#include "include/utils.h"
#include "toml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//******************************************************************************
//  Helper Function Declarations
//******************************************************************************

mcc_Status_t mcc_toml_get_double_with_key(double value[static 1],
                                          toml_table_t const *table,
                                          char const key[static 1]);

mcc_Status_t mcc_toml_get_int_with_key(int value[static 1],
                                       toml_table_t const *table,
                                       char const key[static 1]);

mcc_Status_t mcc_toml_get_string_with_key(char value[static 1],
                                          toml_table_t const *table,
                                          char const key[static 1]);

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

mcc_Status_t mcc_toml_parse_config(mcc_Config_t config[static 1],
                                   char const config_path[static 1]) {
	FILE *file = fopen(config_path, "r");
	if (!file) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "Cannot read file '%s'.", config_path);
		return mcc_utils_status_failure(msg);
	}

	char toml_err_msg[200];
	toml_table_t *table =
	    toml_parse_file(file, toml_err_msg, sizeof(toml_err_msg));
	fclose(file);

	if (!table) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg), "Cannot parse file '%s'.", config_path);
		return mcc_utils_status_failure(msg);
	}

	mcc_Status_t s;
	s = mcc_toml_get_int_with_key(&config->monte_carlo_steps, table,
	                              "monte-carlo-steps");
	if (s.is == SUCCESS)
		s = mcc_toml_get_int_with_key(&config->equilibrium_steps, table,
		                              "equilibrium-steps");
	if (s.is == SUCCESS)
		s = mcc_toml_get_int_with_key(&config->particle_count, table,
		                              "particle-count");
	if (s.is == SUCCESS)
		s = mcc_toml_get_double_with_key(&config->fluid_density, table,
		                                 "fluid-density");
	if (s.is == SUCCESS)
		s = mcc_toml_get_double_with_key(&config->fluid_temp, table,
		                                 "fluid-temperature");
	if (s.is == SUCCESS)
		s = mcc_toml_get_double_with_key(&config->max_displ, table,
		                                 "maximum-displacement");
	if (s.is == SUCCESS)
		s = mcc_toml_get_double_with_key(&config->cutoff_dist, table,
		                                 "cutoff-distance");

	toml_free(table);

	return s;
}

mcc_Status_t mcc_toml_generate_config_file(char const config_path[static 1]) {
	(void)config_path;
	return mcc_utils_status_failure(
	    "The config file generator is not yet implemented!");
}

//******************************************************************************
//  Helper Function Definitions
//******************************************************************************

mcc_Status_t mcc_toml_get_double_with_key(double value[static 1],
                                          toml_table_t const *table,
                                          char const key[static 1]) {
	toml_datum_t v = toml_double_in(table, key);

	if (!v.ok) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Failed to read [double] value for key '%s'.", key);
		return mcc_utils_status_failure(msg);
	}

	*value = v.u.d;

	return mcc_utils_status_success();
}

mcc_Status_t mcc_toml_get_int_with_key(int value[static 1],
                                       toml_table_t const *table,
                                       char const key[static 1]) {
	toml_datum_t v = toml_int_in(table, key);

	if (!v.ok) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Failed to read [integer] value for key '%s'.", key);
		return mcc_utils_status_failure(msg);
	}

	*value = v.u.i;

	return mcc_utils_status_success();
}

mcc_Status_t mcc_toml_get_string_with_key(char *value,
                                          toml_table_t const *table,
                                          char const key[static 1]) {
	toml_datum_t v = toml_string_in(table, key);

	if (!v.ok) {
		char msg[STATUS_MSG_SIZE];
		snprintf(msg, sizeof(msg),
		         "Failed to read [string] value for key '%s'.", key);
		return mcc_utils_status_failure(msg);
	}

	value = v.u.s;

	return mcc_utils_status_success();
}
