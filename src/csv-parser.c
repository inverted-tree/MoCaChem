#include "csv-parser.h"
#include "config.h"
#include "data.h"
#include "panic.h"
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

bool mcc_csv_validate_particle_data_config(char *header, mcc_Config_t *config);

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

bool mcc_csv_read_particle_configuration(char const *filename,
                                         mcc_Config_t *config) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		char err_msg[256];
		snprintf(err_msg, sizeof(err_msg),
		         "Cannot read partilce data file '%s'", filename);
		mcc_panic(MCC_ERR_FILE_NOT_FOUND, err_msg);
	}

	char line[1024];
	if (!fgets(line, sizeof(line), file))
		return false;
	if (!mcc_csv_validate_particle_data_config(line, config)) {
		char err_msg[256];
		snprintf(err_msg, sizeof(err_msg),
		         "The configuration does not match the parameters in the "
		         "particle file:\n Particle Count = %i\n System Volume = %f",
		         config->particle_count, config->box_volume);
		mcc_panic(MCC_ERR_IO, err_msg);
	}

	ValueParser parsers[] = {
	    mcc_csv_parse_int,
	    mcc_csv_parse_double,
	    mcc_csv_parse_double,
	    mcc_csv_parse_double,
	};

	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();

	bool success = true;
	while (fgets(line, sizeof(line), file)) {
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
				fprintf(stderr, "Failed to parse token '%s' in file %s.\n",
				        value, filename);
				success = false;
				break;
			}
			value = strtok(NULL, ",\n");
		}

		if (success)
			fs.set_particle(index, p, NULL);
		else
			break;
	}

	fclose(file);
	return success;
}

bool mcc_csv_write_particle_configuration(char const *filename,
                                          mcc_Config_t *config) {
	if (!filename || !config) {
		fprintf(stderr, "Failed to initialize CSV writer for file '%s'.\n",
		        filename);
		return false;
	}
	FILE *file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "Failed to open file '%s' for writing.\n", filename);
		return false;
	}

	fprintf(file, format, config->particle_count, config->box_volume);
	fprintf(file, "# Particle Configuration: Index, X, Y, Z Coordinates\n");

	mcc_Particle_Access_Functions_t fs = mcc_data_get_access_functions();
	for (int i = 0; i < config->particle_count; i++) {
		mcc_Particle_t *p = fs.get_particle(i, config);
		if (!p) {
			fprintf(stderr, "Failed to recieve particle with index '%i'.\n", i);
			fclose(file);
			return false;
		}
		fprintf(file, "%i, %16.8f, %16.8f, %16.8f,\n", i, p->x, p->y, p->z);
	}

	if (fclose(file))
		printf("The particle configuration has been successfully written to "
		       "'%s'.\n",
		       filename);

	return true;
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

bool mcc_csv_validate_particle_data_config(char *header, mcc_Config_t *config) {
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
