// Isaac Haseley - Nathan Mannes - 2017

/*** Public: For header file ***/

/* These are constants, that are used to configure how the texture behaves. */
#define texQUADRATIC 0
#define texNEAREST 1
#define texREPEAT 2
#define texCLAMP 3

typedef struct texTexture texTexture;
/* Feel free to read from this struct's members, but don't write to them. */
struct texTexture {
	int width, height;	/* do not have to be powers of 2 */
	int texelDim;		/* e.g. 3 for RGB textures */
	int filtering;		/* texQUADRATIC or texNEAREST */
	int topBottom;		/* texREPEAT or texCLAMP */
	int leftRight;		/* texREPEAT or texCLAMP */
	double *data;		/* width * height * texelDim doubles, row-major order */
	double *aux;		/* texelDim doubles, for use as scratch space */
	double *sample;		/* texelDim doubles, where samples are returned */
};



/*** Private ***/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_FAILURE_USERMSG



/*** Public: Basics ***/

/* Sets all texels within the texture. Assumes that the texture has already 
been initialized. Assumes that texel has the same texel dimension as the 
texture. */
void texClearTexels(texTexture *tex, double texel[]) {
	int index, bound, k;
	bound = tex->texelDim * tex->width * tex->height;
	for (index = 0; index < bound; index += tex->texelDim)
		for (k = 0; k < tex->texelDim; k += 1)
			tex->data[index + k] = texel[k];
}

/* Initializes a texTexture struct to a given width and height and a solid 
color. The width and height do not have to be powers of 2. Returns 0 if no 
error occurred. The user must remember to call texDestroy when finished with 
the texture. */
int texInitializeSolid(texTexture *tex, int width, int height, int texelDim, 
		double texel[]) {
	tex->width = width;
	tex->height = height;
	tex->texelDim = texelDim;
	tex->data = (double *)malloc((width * height + 2) * texelDim * 
		sizeof(double));
	if (tex->data != NULL) {
		tex->aux = &(tex->data[tex->width * tex->height * tex->texelDim]);
		tex->sample = &(tex->aux[tex->texelDim]);
		texClearTexels(tex, texel);
	}
	return (tex->data == NULL);
}

/* Initializes a texTexture struct by loading an image from a file. Many image 
types are supported (using the public-domain STB Image library). The width and 
height do not have to be powers of 2. Returns 0 if no error occurred. The user 
must remember to call texDestroy when finished with the texture. */
/* WARNING: Currently there is a weird behavior, in which some image files show 
up with their rows and columns switched, so that their width and height are 
flipped. If that's happening with your image, then use a different image. */
int texInitializeFile(texTexture *tex, const char *path) {
	// Use the STB image library to load the file as unsigned chars.
	unsigned char *rawData;
	int x, y, z, newInd, oldInd;
	rawData = stbi_load(path, &(tex->width), &(tex->height), &(tex->texelDim), 
		0);
    if (rawData == NULL) {
		fprintf(stderr, "error: texInitializeFile: failed to load image %s\n", 
			path);
		fprintf(stderr, "with STB Image reason: %s\n", stbi_failure_reason());
		return 1;
	} else {
		/* STB Image starts in the upper-left, while I want the lower-left. */
		tex->data = (double *)malloc((tex->width * tex->height + 2) * 
			tex->texelDim * sizeof(double));
		if (tex->data != NULL) {
			tex->aux = &(tex->data[tex->width * tex->height * tex->texelDim]);
			tex->sample = &(tex->aux[tex->texelDim]);
			for (x = 0; x < tex->width; x += 1)
				for (y = 0; y < tex->height; y += 1) {
					newInd = tex->texelDim * (x + tex->width * y);
					oldInd = tex->texelDim * 
						(x + tex->width * (tex->height - 1 - y));
					for (z = 0; z < tex->texelDim; z += 1)
						tex->data[newInd + z] = rawData[oldInd + z] / 255.0;
				}
		}
		stbi_image_free(rawData);
		return (tex->data == NULL);
	}
}

/*
For image files with their rows and columns switched, we must use this code 
instead. I'm not sure how to detect this case. So only the other case is 
handled in the code above.
	rawData = stbi_load(path, &(tex->height), &(tex->width), &(tex->texelDim), 0);
	...
	newInd = tex->texelDim * (x + tex->width * y);
	oldInd = tex->texelDim * (tex->height * (tex->width - x + 1) - y);
*/

/* Sets the texture filtering, to either texNEAREST (effectively the default) 
or texQUADRATIC. */
void texSetFiltering(texTexture *tex, int filtering) {
	tex->filtering = filtering;
}

/* Sets the texture wrapping for the top and bottom edges, to either texCLAMP 
(effectively the default) or texREPEAT. */
void texSetTopBottom(texTexture *tex, int topBottom) {
	tex->topBottom = topBottom;
}

/* Sets the texture wrapping for the left and right edges, to either texCLAMP 
(effectively the default) or texREPEAT. */
void texSetLeftRight(texTexture *tex, int leftRight) {
	tex->leftRight = leftRight;
}

/* Gets a single texel within the texture. Assumes that the texture has already 
been initialized. Assumes that texel has the same texel dimension as the 
texture. */
void texGetTexel(texTexture *tex, int s, int t, double texel[]) {
	int k;
	for (k = 0; k < tex->texelDim; k += 1)
		texel[k] = tex->data[(s + tex->width * t) * tex->texelDim + k];
}

/* Sets a single texel within the texture. Assumes that the texture has already 
been initialized. Assumes that texel has the same texel dimension as the 
texture. */
void texSetTexel(texTexture *tex, int x, int y, double texel[]) {
	if (0 <= x && x < tex->width && 0 <= y && y < tex->height
			&& tex->data != NULL) {
		int index, k;
		index = tex->texelDim * (x + tex->width * y);
		for (k = 0; k < tex->texelDim; k += 1)
			tex->data[index + k] = texel[k];
	}
}

/* Deallocates the resources backing the texture. This function must be called 
when the user is finished using the texture. */
void texDestroy(texTexture *tex) {
	free(tex->data);
}



/*** Public: Higher-level sampling ***/

/* Samples from the texture, taking into account wrapping and filtering. The s 
and t parameters are texture coordinates. The texture itself is assumed to have 
texture coordinates [0, 1] x [0, 1]. Assumes that the texture has already been 
initialized. The result is placed in tex->sample for reading by the user. */
void texSample(texTexture *tex, double s, double t) {
	/* Handle clamping vs. repeating. */
	if (tex->leftRight == texREPEAT)
		s = s - floor(s);
	else {
		if (s < 0.0)
			s = 0.0;
		else if (s > 1.0)
			s = 1.0;
	}
	if (tex->topBottom == texREPEAT)
		t = t - floor(t);
	else {
		if (t < 0.0)
			t = 0.0;
		else if (t > 1.0)
			t = 1.0;
	}
	/* Handle nearest-neighbor vs. quadratic filtering. */
	double u, v;
	u = s * (tex->width - 1.0);
	v = t * (tex->height - 1.0);
    // Quadratic filtering implementation
    if (tex->filtering == texQUADRATIC) {
        double uFrac, vFrac;
        uFrac = u - floor(u);
        vFrac = v - floor(v);
        
        texGetTexel(tex, ceil(u), ceil(v), tex->aux);
        vecScale(tex->texelDim, uFrac * vFrac, tex->aux, tex->sample);
        
        texGetTexel(tex, ceil(u), floor(v), tex->aux);
        vecScale(tex->texelDim, uFrac * (1 - vFrac), tex->aux, tex->aux);
        vecAdd(tex->texelDim, tex->aux, tex->sample, tex->sample);
        
        texGetTexel(tex, floor(u), ceil(v), tex->aux);
        vecScale(tex->texelDim, (1 - uFrac) * vFrac, tex->aux, tex->aux);
        vecAdd(tex->texelDim, tex->aux, tex->sample, tex->sample);
        
        texGetTexel(tex, floor(u), floor(v), tex->aux);
        vecScale(tex->texelDim, (1 - uFrac) * (1 - vFrac), tex->aux, tex->aux);
        vecAdd(tex->texelDim, tex->aux, tex->sample, tex->sample);
	} 
    // Nearest-neighbor filtering implementation
    else {
		texGetTexel(tex, (int)round(u), (int)round(v), tex->sample);
    }
}