#include "toml-parser.h"
#include "maybe.h"
#include "panic.h"
#include "toml.h"
#include <stdio.h>

//******************************************************************************
//  Helper function declarations
//******************************************************************************

mcc_Maybe_t mcc_toml_get_double_with_key(toml_table_t *table, char *key);

mcc_Maybe_t mcc_toml_get_int_with_key(toml_table_t *table, char *key);

mcc_Maybe_t mcc_toml_get_string_with_key(toml_table_t *table, char *key);

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_Config_t mcc_toml_parse_config(char *config_path) {
	config_path = "/Users/lukas/Projects/MoCaChem/config.toml";
	size_t const ebuf_size = 256;
	FILE *file = fopen(config_path, "r");
	if (!file) {
		char err_msg[ebuf_size];
		snprintf(err_msg, ebuf_size, "Cannot read file '%s'", config_path);
		mcc_panic(MCC_ERR_IO, err_msg);
	}

	char toml_err_msg[ebuf_size];
	toml_table_t *config_table = toml_parse_file(file, toml_err_msg, ebuf_size);
	fclose(file);

	if (!config_table) {
		char err_msg[ebuf_size];
		snprintf(err_msg, ebuf_size, "Cannot parse file '%s'", config_path);
		mcc_panic(MCC_ERR_IO, err_msg);
	}

	mcc_Config_t config = {};
	// TODO: parse all values
	config.monte_carlo_steps = mcc_from_just_int(
	    mcc_toml_get_int_with_key(config_table, "monte-carlo-steps"));
	config.equilibrium_steps = mcc_from_just_int(
	    mcc_toml_get_int_with_key(config_table, "equilibrium-steps"));
	config.particle_count = mcc_from_just_int(
	    mcc_toml_get_int_with_key(config_table, "particle-count"));
	config.fluid_density = mcc_from_just_double(
	    mcc_toml_get_double_with_key(config_table, "fluid-density"));
	config.fluid_temp = mcc_from_just_double(
	    mcc_toml_get_double_with_key(config_table, "fluid-temperature"));
	config.max_displ = mcc_from_just_double(
	    mcc_toml_get_double_with_key(config_table, "maximum-displacement"));
	config.cutoff_dist = mcc_from_just_double(
	    mcc_toml_get_double_with_key(config_table, "cutoff-distance"));

	toml_free(config_table);

	return config;
}

//******************************************************************************
//  Helper function definitions
//******************************************************************************

mcc_Maybe_t mcc_toml_get_double_with_key(toml_table_t *table, char *key) {
	toml_datum_t value = toml_double_in(table, key);

	if (value.ok)
		return mcc_just_double(value.u.d);

	return mcc_nothing();
}

mcc_Maybe_t mcc_toml_get_int_with_key(toml_table_t *table, char *key) {
	toml_datum_t value = toml_int_in(table, key);

	if (value.ok)
		return mcc_just_int(value.u.i);

	return mcc_nothing();
}

mcc_Maybe_t mcc_toml_get_string_with_key(toml_table_t *table, char *key) {
	toml_datum_t value = toml_string_in(table, key);

	if (value.ok)
		return mcc_just_string(value.u.s);

	return mcc_nothing();
}
