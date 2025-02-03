#include "csv-parser.h"
#include "config.h"
#include "data.h"
#include "include/data.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//******************************************************************************
//  Internal Datatype Definitions
//******************************************************************************

const char *format = "Particle Count = %i, System Volume = %lf\n";

typedef bool (*ValueParser)(char *value, void *dest);

//******************************************************************************
//  Internal Function Declarations
//******************************************************************************

static bool mcc_csv_parse_double(char *value, void *field);

static bool mcc_csv_parse_int(char *value, void *field);

static bool mcc_csv_validate_particle_data_config(char *header,
                                                  mcc_Config_t *config);

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

mcc_Status_t mcc_csv_load_state_space(char const *filename,
                                      mcc_Config_t const *config) {
	mcc_Status_t s = mcc_utils_status_success();
	char err_msg[STATUS_MSG_SIZE];
	memset(err_msg, 0, sizeof(err_msg));

	FILE *file = fopen(filename, "r");
	if (!file) {
		snprintf(err_msg, sizeof(err_msg),
		         "Cannot read partilce data file '%s'.", filename);
		goto ERROR;
	}

	char line[1024];
	if (!fgets(line, sizeof(line), file)) {
		snprintf(err_msg, sizeof(err_msg),
		         "Cannot read line from data file '%s'.", filename);
		goto ERROR;
	}

	if (!mcc_csv_validate_particle_data_config(line, config)) {
		snprintf(err_msg, sizeof(err_msg),
		         "The configuration does not match the parameters in the "
		         "particle file:\n Particle Count = %i\n System Volume = %f",
		         config->particle_count, config->box_volume);
		goto ERROR;
	}

	ValueParser parsers[] = {
	    mcc_csv_parse_int,
	    mcc_csv_parse_double,
	    mcc_csv_parse_double,
	    mcc_csv_parse_double,
	};

	mcc_DAF_t fs = mcc_data_get_access_fs();

	size_t line_count = 0;
	while (fgets(line, sizeof(line), file)) {
		line_count++;
		if (line[0] == '\0' || line[0] == '#' || line[0] == '\n')
			continue;

		int index;
		mcc_Particle_t p = {};
		void *fields[] = {
		    &index,
		    &p.x,
		    &p.y,
		    &p.z,
		};

		char *value = strtok(line, ", ");
		for (size_t i = 0; i < 4; i++) {
			if (!parsers[i](value, fields[i])) {
				snprintf(
				    err_msg, sizeof(err_msg),
				    "Failed to parse token %zu '%s' in line %zu in file %s.", i,
				    value, line_count, filename);
				goto ERROR;
			}
			value = strtok(NULL, ",\n");
		}
		mcc_Index_t idx = {.idx = index};
		fs.set_particle(&idx, &p);
	}

	fclose(file);
	return s;

ERROR:
	s.is = FAILURE;
	strncpy(s.msg, err_msg, sizeof(STATUS_MSG_SIZE));
	return s;
}

mcc_Status_t mcc_csv_write_state_space(char const *filename,
                                       mcc_Config_t const *config) {
	mcc_Status_t s = mcc_utils_status_success();
	char err_msg[STATUS_MSG_SIZE];
	memset(err_msg, 0, sizeof(err_msg));

	if (!filename || !config) {
		snprintf(err_msg, sizeof(err_msg),
		         "Failed to initialize CSV writer for file '%s'.", filename);
		goto ERROR;
	}

	FILE *file = fopen(filename, "w");
	if (!file) {
		snprintf(err_msg, sizeof(err_msg),
		         "Failed to open file '%s' for writing.", filename);
		goto ERROR;
	}

	fprintf(file, format, config->particle_count, config->box_volume);
	fprintf(file, "# Particle Configuration: Index, X, Y, Z Coordinates\n");

	mcc_DAF_t fs = mcc_data_get_access_fs();
	for (int i = 0; i < config->particle_count; i++) {
		mcc_Index_t idx = {.idx = i};
		mcc_Particle_t *p = fs.get_particle(&idx);
		if (!p) {
			snprintf(err_msg, sizeof(err_msg),
			         "Failed to recieve particle with index '%i'.", i);
			fclose(file);
			goto ERROR;
		}
		fprintf(file, "%i, %16.8f, %16.8f, %16.8f,\n", i, p->x, p->y, p->z);
	}

	if (fclose(file))
		printf("The particle configuration has been successfully written to "
		       "'%s'.\n",
		       filename);

	return s;

ERROR:
	s.is = FAILURE;
	strncpy(s.msg, err_msg, sizeof(STATUS_MSG_SIZE));
	return s;
}

//******************************************************************************
//  Internal function definitions
//******************************************************************************

static bool mcc_csv_parse_int(char *value, void *field) {
	char *end;
	long token = strtol(value, &end, 10);
	if (end == value || *end != '\0')
		return false;
	*(int *)field = (int)token;
	return true;
}

static bool mcc_csv_parse_double(char *value, void *field) {
	char *end;
	double token = strtod(value, &end);
	if (end == value || *end != '\0')
		return false;
	*(double *)field = token;
	return true;
}

static bool mcc_csv_validate_particle_data_config(char *header,
                                                  mcc_Config_t *config) {
	if (!header || !config)
		return false;

	int count;
	double volume;
	if (sscanf(header, format, &count, &volume) == 2)
		return count == config->particle_count &&
		       fabs(volume - config->box_volume) < 1E-6;
	else
		return false;
}
