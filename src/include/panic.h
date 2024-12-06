#pragma once

//******************************************************************************
//  Interface Datatype Declarations
//******************************************************************************

#if __STDC_VERSION__ >= 202311L
#define NORETURN [[noreturn]]
#elif __STDC_VERSION__ >= 201112L
#define NORETURN _Noreturn
#else
#define NORETURN
#endif

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

void NORETURN mcc_panic(MCC_ERR_TYPE err, char const *msg);
