#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

const char* DATA_DIR = "data/";
enum FlagType {
	FLAG_TYPE_NONE,
	FLAG_TYPE_BOOL,
	FLAG_TYPE_INT,
	FLAG_TYPE_FLOAT,
	FLAG_TYPE_VEC3,
	FLAG_TYPE_VEC4
};

typedef int(*FlagOperation)(Transform* inoutTransform, const char** params);

struct Flag {
	char* name;
	char* message;
	char terse;
	FlagOperation op;
	Flag* mods;
};

bool checkNumeric(char* str) {
	while(*str) {
		if(!((str >= '0' && str <= '9') || (str == '.'))) {
			return false;
		}
		str++;
	}
	return true;
}

int parseRotate(Transform* inoutTransform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	float rotation[3] = {};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			rotation[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous rotation i guess...
	*inoutTransform.rotation = rotation;
	return current;
}

int parseScale(Transform* transform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	float scale[3] = {};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			scale[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous scale i guess...
	*inoutTransform.scale = scale;
	return current;
}	

int parseOffset(Transform* transform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	float offset[3] = {};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			offset[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous offset i guess...
	*inoutTransform.offset = offset;
	return current;
}	

Flag flags[] = {
	{
		.name = "rotate",
		.message = "Rotate the object",
		.terse = 'r',
		.type = FLAG_TYPE_VEC3,
		.op = parseRotate,
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
		.op = parseScale,
		.mods = NULL
	},
	{
		.name = "offset",
		.message = "Translate the object",
		.terse = 'o',
		.type = FLAG_TYPE_FLOAT,
		.op = parseOffset,
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


Transform parseTransform() {
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

struct Transform {
	float offset[3];
	float rotation[3];
	float scale[3];
}

Transform parseArgSet(int argc, const char** argv) {
	Transform transform = {};
	int i = 0;
	for(; i < argc; i++) {
		char* arg = argv[i];
		// todo mod flags
		if(arg[0] == '-' && arg[1] == '-') {
			// this is a full name flag
			Flag* flag = flags;
			while(*flag) {
				if(strcmp(&arg[2], flag.name) == 0) {
					int used = flag.op(&transform, argv[i + 1]);
					i += used;
					break;
				}
				flag++;
			}
		} else if(arg[0] == '-') {
			// this is a terse flag
			Flag* flag = flags;
			while(*flag) {
				if(arg[1] == flag.terse) {
					int used = flag.op(&transform, argv[i + 1]);
					i += used;
					break;
				}
				flag++;
			}
		} else {
			// this is a model/shape or end of args
			break;	
		}
	}
	return transform;
}

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
	if (sourceBuffer) free(sourceBuffer);
closeSource:
	close(sourceFD);
	
	return ret;
}

int main(int argc, char** argv) {
	char buf[256] = {};
	Transform globalTransform = parseArgSet(argc, argv); 
	for(int i = 1; i < argc; i++) {
		char* arg = argv[i];
		buildShape(NULL, arg);
	}
	printf("hello world\n");
}
