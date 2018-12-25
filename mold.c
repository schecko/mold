#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

const char* DATA_DIR = "data/";
#if 0
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
#endif

typedef struct {
	int x;
} ShapeModifier;


bool buildShape(ShapeModifier* modifier, char* shape) {
	bool ret = true;
	int sourceFD;
	int destFD;
	char buf[256] = {};
	sprintf(buf, "%s%s.obj", DATA_DIR, shape);

	sourceFD = open(buf, O_RDONLY | O_SYNC);
	if(sourceFD == -1) {
		printf("Failed to find object called %s\n", shape);
		ret = false;
		goto closeSource;
	}
	off_t sourceFileSize = lseek(sourceFD, 0, SEEK_END);
	char* sourceBuffer = malloc(sourceFileSize);
	if(!sourceBuffer) {
		ret = false;
		goto freeSourceBuffer;
	}
	read(sourceFD, sourceBuffer, sourceFileSize);

	sprintf(buf, "%s.obj", shape);
	destFD = open(buf, O_RDWR | O_CREAT | O_EXCL);
	if(destFD == -1) {
		printf("Failed to create new file %s\n", buf);
		ret = false;
		goto closeDest;
	}

closeDest:
	close(destFD);
freeSourceBuffer:
	free(sourceBuffer);
closeSource:
	close(sourceFD);
	
	return ret;
}

int main(int argc, char** argv) {
	char buf[256] = {};

	for(int i = 1; i < argc; i++) {
		char* arg = argv[i];
		buildShape(NULL, arg);
	}
	printf("hello world\n");
}
