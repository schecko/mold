#include "argo.h"
#include <stdio.h>


struct Flag {
	char* name;
	char* message;
	char terse;
	Flag* mods;
}

Flag flags[] = {
	{
		.name = "rotate",
		.message = "Rotate the object",
		.terse = 'r',
		.mods = {
			{
				.name = "local",
				.message = "Specify local rotation",
				.terse = 'l',
				.mods = NULL
			},
			{
				.name = "world",
				.message = "Specify a world rotation",
				.terse = 'w',
				.mods = NULL
			},
			NULL
		}
	},
	{
		.name = "scale",
		.message = "Scale the object",
		.terse = 's',
		.mods = NULL
	},
	{
		.name = "offset",
		.message = "Translate the object",
		.terse = 'o',
		.mods = NULL
	},
	NULL
};

char* findEndOfArg(char* arg) {
	while(!isspace(arg)) { arg++; };
	return arg;
}

struct ArgSet {
	int numArgs;
	char* shape;
}

int parseArgSet(int argc, const char** argv) {
	int i = 0;
	for(; i < argc; i++) {
		char* arg = argv[i];
		if(arg[0] == '-' && arg[1] == '-') {
			// this arg is full name
		} else if(arg[0] == '-') {
			// this is a terse flag
		} else {
			// this is a model/shape or end of args
			break;	
		}
	}
}


int main(int argc, char** argv) {

	for(int i = 0; i < argc; i++) {

	}
	printf("hello world\n");
}
