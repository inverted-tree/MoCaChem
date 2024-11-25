#pragma once

#include <stdbool.h>

//******************************************************************************
//  Interface datatype declarations
//******************************************************************************

typedef enum {
	JUST_DOUBLE,
	JUST_INT,
	JUST_STRING,
	NOTHING,
} mcc_MaybeType;

typedef struct {
	mcc_MaybeType t;
	union {
		double dbl_value;
		int int_value;
		char *str_value;
	};
} mcc_Maybe;

//******************************************************************************
//  Interface function declarations
//******************************************************************************

double mcc_from_just_double(mcc_Maybe m);

int mcc_from_just_int(mcc_Maybe m);

char *mcc_from_just_string(mcc_Maybe m);

bool mcc_is_just_double(mcc_Maybe m);

bool mcc_is_just_int(mcc_Maybe m);

bool mcc_is_just_string(mcc_Maybe m);

bool mcc_is_nothing(mcc_Maybe m);

mcc_Maybe mcc_just_double(double value);

mcc_Maybe mcc_just_int(int value);

mcc_Maybe mcc_just_string(char *value);

mcc_Maybe mcc_nothing();
