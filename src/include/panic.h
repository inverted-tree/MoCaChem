#pragma once

//******************************************************************************
//  Interface Datatype Declarations
//******************************************************************************

typedef enum {
	MCC_ERR_IO,
	MCC_ERR_FILE_NOT_FOUND,
	MCC_ERR_UNKNOWN_ARGUMENT,
	MCC_ERR_TYPE_MISMATCH,
	MCC_ERR_MALLOC,
	MCC_ERR_INDEX_OUT_OF_BOUNDS,
} MCC_ERR_TYPE;

//******************************************************************************
//  Interface Function Declarations
//******************************************************************************

void mcc_panic(MCC_ERR_TYPE err, char const *msg);
