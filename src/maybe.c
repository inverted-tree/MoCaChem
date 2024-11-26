#include "maybe.h"
#include "panic.h"
#include "string.h"

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_Maybe_t mcc_just_int(int value) {
	mcc_Maybe_t m = {
	    .t = JUST_INT,
	    .int_value = value,
	};
	return m;
}

mcc_Maybe_t mcc_just_double(double value) {
	mcc_Maybe_t m = {
	    .t = JUST_DOUBLE,
	    .dbl_value = value,
	};
	return m;
}

mcc_Maybe_t mcc_just_string(char *value) {
	mcc_Maybe_t m = {
	    .t = JUST_STRING,
	    .str_value = strdup(value),
	};
	return m;
}

mcc_Maybe_t mcc_nothing() {
	mcc_Maybe_t m = {
	    .t = NOTHING,
	};
	return m;
}

double mcc_from_just_double(mcc_Maybe_t m) {
	if (m.t == JUST_DOUBLE)
		return m.dbl_value;
	mcc_panic(MCC_ERR_TYPE_MISMATCH, "Failed to unwrap double");
}

int mcc_from_just_int(mcc_Maybe_t m) {
	if (m.t == JUST_INT)
		return m.int_value;
	mcc_panic(MCC_ERR_TYPE_MISMATCH, "Failed to unwrap int");
}

char *mcc_from_just_string(mcc_Maybe_t m) {
	if (m.t == JUST_STRING)
		return m.str_value;
	mcc_panic(MCC_ERR_TYPE_MISMATCH, "Failed to unwrap string");
}

bool mcc_is_just_double(mcc_Maybe_t m) {
	return (m.t == JUST_DOUBLE) ? true : false;
}

bool mcc_is_just_int(mcc_Maybe_t m) { return (m.t == JUST_INT) ? true : false; }

bool mcc_is_just_string(mcc_Maybe_t m) {
	return (m.t == JUST_STRING) ? true : false;
}

bool mcc_is_nothing(mcc_Maybe_t m) { return (m.t == NOTHING) ? true : false; }
