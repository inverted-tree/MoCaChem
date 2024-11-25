#include "utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
	mcc_CmdlOpts args = mcc_utils_handle_command_line_args(argc, argv);
	printf("The project root is at %s\n", mcc_from_just_string(args.root_dir));
	printf("The config file is %s\n", mcc_from_just_string(args.config));
	return 0;
}
