#include "panic.h"
#include <stdio.h>
#include <stdlib.h>

//******************************************************************************
//  Interface function definitions
//******************************************************************************

void mcc_panic(MCC_ERR_TYPE err, char const *msg) {
	puts(msg);
	exit(EXIT_FAILURE);
}
