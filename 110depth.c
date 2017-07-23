// Isaac Haseley - Nathan Mannes - 2017


/*** Creating and destroying ***/

/* Feel free to read the struct's members, but don't write them, except through 
the accessors below such as depthSetZ, etc. */
typedef struct depthBuffer depthBuffer;
struct depthBuffer {
	int width, height;
	double *z;			/* width * height doubles */
};

/* Initializes a depth buffer. When you are finished with the buffer, you must 
call depthDestroy to deallocate its backing resources. */
int depthInitialize(depthBuffer *buf, int width, int height) {
	buf->z = (double *)malloc(width * height * sizeof(double));
	if (buf->z != NULL) {
		buf->width = width;
		buf->height = height;
	}
	return (buf->z == NULL);
}

/* Sets every Z-value to the given z. Typically you use this function at the 
start of each frame, passing a large negative value for z. */
void depthClearZs(depthBuffer *buf, double z) {
	int i, j;
	for (i = 0; i < buf->width; i += 1)
		for (j = 0; j < buf->height; j += 1)
			buf->z[i + buf->width * j] = z;
}

/* Sets the Z-value at pixel (i, j) to the given z. */
void depthSetZ(depthBuffer *buf, int i, int j, double z) {
	if (0 <= i && i < buf->width && 0 <= j && j < buf->height)
		buf->z[i + buf->width * j] = z;
}

/* Returns the Z-value at pixel (i, j). */
double depthGetZ(depthBuffer *buf, int i, int j) {
	if (0 <= i && i < buf->width && 0 <= j && j < buf->height)
		return buf->z[i + buf->width * j];
	else
		/* There's no right answer, but we have to return something. */
		return 0.0;
}

/* Deallocates the resources backing the buffer. This function must be called 
when you are finished using a buffer. */
void depthDestroy(depthBuffer *buf) {
	if (buf->z != NULL)
		free(buf->z);
	buf->z = NULL;
}