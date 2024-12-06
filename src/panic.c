#include "panic.h"
#include <stdio.h>
#include <stdlib.h>

//******************************************************************************
//  Interface Function Definitions
//******************************************************************************

void NORETURN mcc_panic(MCC_ERR_TYPE err, char const *msg) {
	(void)err;
	puts(msg);
	exit(EXIT_FAILURE);
}
