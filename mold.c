#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

const char* DATA_DIR = "data/";
typedef enum FlagType {
	FLAG_TYPE_NONE,
	FLAG_TYPE_BOOL,
	FLAG_TYPE_INT,
	FLAG_TYPE_FLOAT,
	FLAG_TYPE_VEC3,
	FLAG_TYPE_VEC4
} FlagType;

typedef struct v3 {
	float a[3];
} v3;


typedef struct Transform {
	v3 offset;
	v3 rotation;
	v3 scale;
} Transform;

Transform initTransform() {
	Transform ret = {};
	ret.scale.a[0] = 1.0;
	ret.scale.a[1] = 1.0;
	ret.scale.a[2] = 1.0;
	return ret;
}

void printTransform(Transform* transform) {
	printf("offset: %f %f %f\n", transform->offset.a[0], transform->offset.a[1], transform->offset.a[2]); 
	printf("scale: %f %f %f\n", transform->scale.a[0], transform->scale.a[1], transform->scale.a[2]); 
	printf("rotation: %f %f %f\n", transform->rotation.a[0], transform->rotation.a[1], transform->rotation.a[2]); 
}

typedef int(*FlagOperation)(Transform* inoutTransform, const char** params);

typedef struct Flag {
	char* name;
	char* message;
	char terse;
	FlagType type;
	FlagOperation op;
	struct Flag* mods;
} Flag;

bool checkNumeric(const char* str) {
	while(*str) {
		if(!((*str >= '0' && *str <= '9') || (*str == '.'))) {
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
	v3 rotation = {};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			rotation.a[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous rotation i guess...
	inoutTransform->rotation = rotation;
	return current;
}

int parseScale(Transform* inoutTransform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	v3 scale = {{ 1, 1, 1 }};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			scale.a[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// scale all axis' if x,y arent set.
	if(current == 1) {
		scale.a[1] = scale.a[0];
		scale.a[2] = scale.a[0];
	}
	// overwrite any previous scale i guess...
	inoutTransform->scale = scale;
	return current;
}	

int parseOffset(Transform* inoutTransform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	v3 offset = {};
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			offset.a[current++] = temp;
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous offset i guess...
	inoutTransform->offset = offset;
	return current;
}	

Flag flags[] = {
	{
		.name = "rotate",
		.message = "Rotate the object",
		.terse = 'r',
		.type = FLAG_TYPE_VEC3,
		.op = parseRotate,
		/*mods = {
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
		}*/
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

typedef struct Value {
	FlagType type;
	union {
		bool b;
		int i;
		float f;
		float v3[3];
		float v4[4];
	};
} Value;

char* findEndOfArg(char* arg) {
	while(!isspace(*arg)) { arg++; }
	return arg;
}

typedef struct ArgSet {
	int argc;
	char** argv;
} ArgSet;

Transform parseArgSet(ArgSet* arg, int* outNumParsed) {
	Transform transform = initTransform();
	int i = 0;
	
	for(; i < arg->argc; i++) {
		const char* currentArg = arg->argv[i];
		// todo mod flags
		if(currentArg[0] == '-' && currentArg[1] == '-') {
			// this is a full name flag
			Flag* flag = flags;
			while(flag) {
				if(strcmp(&currentArg[2], flag->name) == 0) {
					int used = flag->op(&transform, (const char**)&arg->argv[i + 1]);
					i += used;
					break;
				}
				flag++;
			}
		} else if(currentArg[0] == '-') {
			// this is a terse flag
			Flag* flag = flags;
			while(flag) {
				if(currentArg[1] == flag->terse) {
					int used = flag->op(&transform, (const char**)&arg->argv[i + 1]);
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
	if(outNumParsed) *outNumParsed = i;

	return transform;
}

bool buildShape(Transform globalTransform, Transform modelTransform, const char* shape) {
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
	int numParsed = 0;
	int totalNumParsed = 0;
	ArgSet set = {};
	set.argc = argc;
	set.argv = argv;
	// the first argset is the global transform
	Transform globalTransform = parseArgSet(&set, &numParsed); 
	totalNumParsed += numParsed;

	printf("global transform is:\n");
	printTransform(&globalTransform);
	printf("\n");

	for(;totalNumParsed < argc;) {
		const char* arg = argv[totalNumParsed++];
		Transform localTransform = parseArgSet(&set, &numParsed);
		totalNumParsed += numParsed;
		buildShape(globalTransform, localTransform, arg);

		printf("building shape %s\n", arg);
		printTransform(&localTransform);
	}

	printf("\n");
}
