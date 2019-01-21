#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gsl/gsl_vector.h>

const char* DATA_DIR = "data/";
typedef enum FlagType {
	FLAG_TYPE_NONE,
	FLAG_TYPE_BOOL,
	FLAG_TYPE_INT,
	FLAG_TYPE_FLOAT,
	FLAG_TYPE_VEC3,
	FLAG_TYPE_VEC4
} FlagType;

typedef struct Transform {
	gsl_vector* offset;
	gsl_vector* rotation;
	gsl_vector* scale;
} Transform;

Transform initTransform() {
	Transform ret = {};
	ret.offset = gsl_vector_calloc(3);
	ret.rotation = gsl_vector_calloc(3);
	ret.scale = gsl_vector_alloc(3);
	gsl_vector_set_all(ret.scale, 1.0f);
	return ret;
}

void printTransform(Transform* transform) {
	gsl_vector_fprintf(stdout, transform->offset, "%f");
	gsl_vector_fprintf(stdout, transform->scale, "%f");
	gsl_vector_fprintf(stdout, transform->rotation, "%f");
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
	gsl_vector* rotation = gsl_vector_calloc(3);
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			gsl_vector_set(rotation, current++, temp);
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous rotation i guess...
	gsl_vector_memcpy(inoutTransform->rotation, rotation);
	gsl_vector_free(rotation);
	return current;
}

int parseScale(Transform* inoutTransform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	gsl_vector* scale = gsl_vector_calloc(3);
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			gsl_vector_set(scale, current++, temp);
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// scale all axis' if x,y arent set.
	if(current == 1) {
		gsl_vector_set(scale, 1, gsl_vector_get(scale, 0));
		gsl_vector_set(scale, 2, gsl_vector_get(scale, 0));
	}
	// overwrite any previous scale i guess...
	gsl_vector_memcpy(inoutTransform->scale, scale);
	gsl_vector_free(scale);
	return current;
}	

int parseOffset(Transform* inoutTransform, const char** params) {
	if(!params) return 0;
	if(!params[0]) return 0;
	
	int current = 0;
	gsl_vector* offset = gsl_vector_calloc(3);
	for(int i = 0; i < 3; i++) {
		if(checkNumeric(*params)) {
			float temp = atof(*params);
			gsl_vector_set(offset, current++, temp);
		} else {
			break;
		}

		if(current >= 3) break;
	}
	// overwrite any previous offset i guess...
	gsl_vector_memcpy(inoutTransform->offset, offset);
	gsl_vector_free(offset);
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
	set.argc = argc - 1; // ignore the name of the binary..
	set.argv = argv + 1;
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
