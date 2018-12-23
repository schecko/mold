#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

const char* DATA_DIR = "data";

enum FlagType {
	FLAG_TYPE_NONE,
	FLAG_TYPE_BOOL,
	FLAG_TYPE_INT,
	FLAG_TYPE_FLOAT,
	FLAG_TYPE_VEC3,
	FLAG_TYPE_VEC4
};

struct Flag {
	char* name;
	char* message;
	char terse;
	Flag* mods;
};

Flag flags[] = {
	{
		.name = "rotate",
		.message = "Rotate the object",
		.terse = 'r',
		.type = FLAG_TYPE_VEC3,
		.mods = {
			{
				.name = "local",
				.message = "Specify local rotation",
				.terse = 'l',
				.type = FLAG_TYPE_BOOL,
				.mods = NULL
			},
			{
				.name = "world",
				.message = "Specify a world rotation",
				.terse = 'w',
				.type = FLAG_TYPE_BOOL,
				.mods = NULL
			},
			NULL
		}
	},
	{
		.name = "scale",
		.message = "Scale the object",
		.terse = 's',
		.type = FLAG_TYPE_INT,
		.mods = NULL
	},
	{
		.name = "offset",
		.message = "Translate the object",
		.terse = 'o',
		.type = FLAG_TYPE_FLOAT,
		.mods = NULL
	},
	NULL
};

struct Value {
	FlagType type;
	union {
		bool b;
		int i;
		float f;
		float v3[3];
		float v4[4];
	}
};

Value parseValue(int argc, char** argv, FlagType expectedType) {
	switch(expectedType) {
		case FLAG_TYPE_BOOL:
			break;
		case FLAG_TYPE_INT:
			break;
		case FLAG_TYPE_FLOAT:
			break;
		case FLAG_TYPE_VEC3:
			break;
		case FLAG_TYPE_VEC4:
			break;
		default:
			break;
	}
}

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
			Flag* tempFlags = flags;
			while(*tempFlags) {
				if(strcmp(&arg[2], tempFlags.name) == 0) {
				}
				tempFlags++;
			}
		} else if(arg[0] == '-') {
			// this is a terse flag
		} else {
			// this is a model/shape or end of args
			break;	
		}
	}
}

void buildShape() {

	int sourceFD;
	int destFD;

	sprintf(buf, "%s%s.obj", DATA_DIR, arg);

	sourceFD = open(buf, O_RDONLY | O_SYNC);
	if(sourceFD == -1) {
		printf("Failed to find object called %s\n", arg);
		continue;
	}
	off_t sourceFileSize = lseek(sourceFD, 0, SEEK_END);
	char* sourceBuffer = malloc(sourceFileSize);
	read(sourceFD, sourceBuffer, sourceFileSize);




	sprintf(buf, "%s.obj", arg);
	destFD = open(buf, O_RDWR | O_CREAT | O_EXCL);
	if(destFD == -1) {
		printf("Failed to create new file %s\n", buf);
		continue;
	}

}

int main(int argc, char** argv) {
	char buf[256] = {};

	for(int i = 0; i < argc; i++) {
		char* arg = argv[i];
		int sourceFD;
		int destFD;

		sprintf(buf, "%s%s.obj", DATA_DIR, arg);

		sourceFD = open(buf, O_RDONLY | O_SYNC);
		if(sourceFD == -1) {
			printf("Failed to find object called %s\n", arg);
			continue;
		}
		off_t sourceFileSize = lseek(sourceFD, 0, SEEK_END);
		char* sourceBuffer = malloc(sourceFileSize);
		read(sourceFD, sourceBuffer, sourceFileSize);




		sprintf(buf, "%s.obj", arg);
		destFD = open(buf, O_RDWR | O_CREAT | O_EXCL);
		if(destFD == -1) {
			printf("Failed to create new file %s\n", buf);
			continue;
		}



	}
	printf("hello world\n");
}
