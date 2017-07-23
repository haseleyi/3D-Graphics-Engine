/*** Creating and destroying ***/

/* Feel free to read the struct's members, but don't write them, except through 
the accessors below such as meshSetTriangle, meshSetVertex. */
typedef struct meshMesh meshMesh;
struct meshMesh {
	int triNum, vertNum, attrDim;
	int *tri;						/* triNum * 3 ints */
	double *vert;					/* vertNum * attrDim doubles */
};

/* Initializes a mesh with enough memory to hold its triangles and vertices. 
Does not actually fill in those triangles or vertices with useful data. When 
you are finished with the mesh, you must call meshDestroy to deallocate its 
backing resources. */
int meshInitialize(meshMesh *mesh, int triNum, int vertNum, int attrDim) {
	mesh->tri = (int *)malloc(triNum * 3 * sizeof(int) +
		vertNum * attrDim * sizeof(double));
	if (mesh->tri != NULL) {
		mesh->vert = (double *)&(mesh->tri[triNum * 3]);
		mesh->triNum = triNum;
		mesh->vertNum = vertNum;
		mesh->attrDim = attrDim;
	}
	return (mesh->tri == NULL);
}

/* Sets the trith triangle to have vertex indices i, j, k. */
void meshSetTriangle(meshMesh *mesh, int tri, int i, int j, int k) {
	if (0 <= tri && tri < mesh->triNum) {
		mesh->tri[3 * tri] = i;
		mesh->tri[3 * tri + 1] = j;
		mesh->tri[3 * tri + 2] = k;
	}
}

/* Returns a pointer to the trith triangle. For example:
	int *triangle13 = meshGetTrianglePointer(&mesh, 13);
	printf("%d, %d, %d\n", triangle13[0], triangle13[1], triangle13[2]); */
int *meshGetTrianglePointer(meshMesh *mesh, int tri) {
	if (0 <= tri && tri < mesh->triNum)
		return &mesh->tri[tri * 3];
	else
		return NULL;
}

/* Sets the vertth vertex to have attributes attr. */
void meshSetVertex(meshMesh *mesh, int vert, double attr[]) {
	int k;
	if (0 <= vert && vert < mesh->vertNum)
		for (k = 0; k < mesh->attrDim; k += 1)
			mesh->vert[mesh->attrDim * vert + k] = attr[k];
}

/* Returns a pointer to the vertth vertex. For example:
	double *vertex13 = meshGetVertexPointer(&mesh, 13);
	printf("x = %f, y = %f\n", vertex13[0], vertex13[1]); */
double *meshGetVertexPointer(meshMesh *mesh, int vert) {
	if (0 <= vert && vert < mesh->vertNum)
		return &mesh->vert[vert * mesh->attrDim];
	else
		return NULL;
}

/* Deallocates the resources backing the mesh. This function must be called 
when you are finished using a mesh. */
void meshDestroy(meshMesh *mesh) {
	// Should test whether pointer NULL. If so, free and set to NULL.!!
	free(mesh->tri);
}



/*** Rendering ***/

double meshTransformedVertices[renVERTNUMBOUND * renVARYDIMBOUND];

double *meshGetTransformedVertexPointer(meshMesh *mesh, renRenderer *ren, 
		int vert) {
	if (0 <= vert && vert < mesh->vertNum)
		return &meshTransformedVertices[vert * ren->varyDim];
	else
		return NULL;
}

/* Renders the mesh. If the mesh and the renderer have differing values for 
attrDim, then prints an error message and does not render anything. */
void meshRender(meshMesh *mesh, renRenderer *ren, double unif[], 
		texTexture *tex[]) {
	if(mesh->attrDim != ren->attrDim) {
        printf("Error: Mesh and renderer have different attrDims.\n");
        fflush(stdout);
    }
    else {
        int i, j;
        double attr[ren->attrDim];
        double vary[ren->varyDim];
        // For each vertex...
        for(i = 0; i < mesh->vertNum; i++) {
            // Create an attribute vector from vert
            for (j = 0; j < ren->attrDim; j++) {
                attr[j] = mesh->vert[i * ren->attrDim + j];
            }
            // Transform said vector
            ren->transformVertex(ren, unif, attr, vary);
            
            // Add it to global 
            for (j = 0; j < ren->varyDim; j++) {
                meshTransformedVertices[i * ren->varyDim + j] = vary[j];
            }
        }        
        double a[ren->varyDim], b[ren->varyDim], c[ren->varyDim];
        int *pointerAtTriangle;
        
        // For each triangle...
        for (int i = 0; i < mesh->triNum; i++) {
            pointerAtTriangle = meshGetTrianglePointer(mesh, i);     
            // Grab its three vertices' vary vectors and triRender
            for(j = 0; j < ren->varyDim; j++) {
                a[j] = meshTransformedVertices[pointerAtTriangle[0] 
                    * ren->varyDim + j];
                b[j] = meshTransformedVertices[pointerAtTriangle[1] 
                    * ren->varyDim + j];
                c[j] = meshTransformedVertices[pointerAtTriangle[2] 
                    * ren->varyDim + j];      
            }
            clipRender(ren, unif, tex, a, b, c);
        }
    }
}



/*** Convenience initializers: 2D ***/

/* Initializes a mesh to two triangles forming a rectangle of the given sides. 
The four attributes are X, Y, S, T. Do not call meshInitialize separately; it 
is called inside this function. Don't forget to call meshDestroy when done. */
int meshInitializeRectangle(meshMesh *mesh, double left, double right, 
		double bottom, double top) {
	int error = meshInitialize(mesh, 2, 4, 2 + 2);
	if (error == 0) {
		meshSetTriangle(mesh, 0, 0, 1, 2);
		meshSetTriangle(mesh, 1, 0, 2, 3);
		double attr[4];
		vecSet(4, attr, left, bottom, 0.0, 0.0);
		meshSetVertex(mesh, 0, attr);
		vecSet(4, attr, right, bottom, 1.0, 0.0);
		meshSetVertex(mesh, 1, attr);
		vecSet(4, attr, right, top, 1.0, 1.0);
		meshSetVertex(mesh, 2, attr);
		vecSet(4, attr, left, top, 0.0, 1.0);
		meshSetVertex(mesh, 3, attr);
	}
	return error;
}

/* Initializes a mesh to sideNum triangles forming an ellipse of the given 
center (x, y) and radii rx, ry. The four attributes are X, Y, S, T. Do not call 
meshInitialize separately; it is called inside this function. Don't forget to 
call meshDestroy when done. */
int meshInitializeEllipse(meshMesh *mesh, double x, double y, double rx, 
		double ry, int sideNum) {
	int i, error;
	double theta, cosTheta, sinTheta, attr[4] = {x, y, 0.5, 0.5};
	error = meshInitialize(mesh, sideNum, sideNum + 1, 2 + 2);
	if (error == 0) {
		meshSetVertex(mesh, 0, attr);
		for (i = 0; i < sideNum; i += 1) {
			meshSetTriangle(mesh, i, 0, i + 1, (i + 1) % sideNum + 1);
			theta = i * 2.0 * M_PI / sideNum;
			cosTheta = cos(theta);
			sinTheta = sin(theta);
			vecSet(4, attr, x + rx * cosTheta, y + ry * sinTheta, 
				0.5 * cosTheta + 0.5, 0.5 * sinTheta + 0.5);
			meshSetVertex(mesh, i + 1, attr);
		}
	}
	return error;
}



/*** Convenience initializers: 3D ***/

/* Assumes that attributes 0, 1, 2 are XYZ. Assumes that the vertices of the 
triangle are in counter-clockwise order when viewed from 'outside' the 
triangle. Computes the outward-pointing unit normal vector for the triangle. */
void meshTrueNormal(double a[], double b[], double c[], double normal[3]) {
	double bMinusA[3], cMinusA[3];
	vecSubtract(3, b, a, bMinusA);
	vecSubtract(3, c, a, cMinusA);
	vec3Cross(bMinusA, cMinusA, normal);
	vecUnit(3, normal, normal);
}

/* Assumes that attributes 0, 1, 2 are XYZ. Sets attributes n, n + 1, n + 2 to 
flat-shaded normals. If a vertex belongs to more than triangle, then some 
unspecified triangle's normal wins. */
void meshFlatNormals(meshMesh *mesh, int n) {
	int i, *tri;
	double *a, *b, *c, normal[3];
	for (i = 0; i < mesh->triNum; i += 1) {
		tri = meshGetTrianglePointer(mesh, i);
		a = meshGetVertexPointer(mesh, tri[0]);
		b = meshGetVertexPointer(mesh, tri[1]);
		c = meshGetVertexPointer(mesh, tri[2]);
		meshTrueNormal(a, b, c, normal);
		vecCopy(3, normal, &a[n]);
		vecCopy(3, normal, &b[n]);
		vecCopy(3, normal, &c[n]);
	}
}

/* Assumes that attributes 0, 1, 2 are XYZ. Sets attributes n, n + 1, n + 2 to 
smooth-shaded normals. Does not do anything special to handle multiple vertices 
with the same coordinates. */
void meshSmoothNormals(meshMesh *mesh, int n) {
	int i, *tri;
	double *a, *b, *c, normal[3] = {0.0, 0.0, 0.0};
	/* Zero the normals. */
	for (i = 0; i < mesh->vertNum; i += 1) {
		a = meshGetVertexPointer(mesh, i);
		vecCopy(3, normal, &a[n]);
	}
	/* For each triangle, add onto the normal at each of its vertices. */
	for (i = 0; i < mesh->triNum; i += 1) {
		tri = meshGetTrianglePointer(mesh, i);
		a = meshGetVertexPointer(mesh, tri[0]);
		b = meshGetVertexPointer(mesh, tri[1]);
		c = meshGetVertexPointer(mesh, tri[2]);
		meshTrueNormal(a, b, c, normal);
		vecAdd(3, normal, &a[n], &a[n]);
		vecAdd(3, normal, &b[n], &b[n]);
		vecAdd(3, normal, &c[n], &c[n]);
	}
	/* Normalize the normals. */
	for (i = 0; i < mesh->vertNum; i += 1) {
		a = meshGetVertexPointer(mesh, i);
		vecUnit(3, &a[n], &a[n]);
	}
}

/* Builds a mesh for a parallelepiped (box) of the given size. The attributes 
are XYZ position, ST texture, and NOP unit normal vector. The normals are 
discontinuous at the edges (flat shading, not smooth). To facilitate this, some 
vertices have equal XYZ but different NOP, for 24 vertices in all. Don't forget 
to meshDestroy when finished. */
int meshInitializeBox(meshMesh *mesh, double left, double right, double bottom, 
		double top, double base, double lid) {
	int error = meshInitialize(mesh, 12, 24, 3 + 2 + 3);
	if (error == 0) {
		/* Make the triangles. */
		meshSetTriangle(mesh, 0, 0, 2, 1);
		meshSetTriangle(mesh, 1, 0, 3, 2);
		meshSetTriangle(mesh, 2, 4, 5, 6);
		meshSetTriangle(mesh, 3, 4, 6, 7);
		meshSetTriangle(mesh, 4, 8, 10, 9);
		meshSetTriangle(mesh, 5, 8, 11, 10);
		meshSetTriangle(mesh, 6, 12, 13, 14);
		meshSetTriangle(mesh, 7, 12, 14, 15);
		meshSetTriangle(mesh, 8, 16, 18, 17);
		meshSetTriangle(mesh, 9, 16, 19, 18);
		meshSetTriangle(mesh, 10, 20, 21, 22);
		meshSetTriangle(mesh, 11, 20, 22, 23);
		/* Make the vertices after 0, using vertex 0 as temporary storage. */
		double *v = mesh->vert;
		vecSet(8, v, right, bottom, base, 1.0, 0.0, 0.0, 0.0, -1.0);
		meshSetVertex(mesh, 1, v);
		vecSet(8, v, right, top, base, 1.0, 1.0, 0.0, 0.0, -1.0);
		meshSetVertex(mesh, 2, v);
		vecSet(8, v, left, top, base, 0.0, 1.0, 0.0, 0.0, -1.0);
		meshSetVertex(mesh, 3, v);
		vecSet(8, v, left, bottom, lid, 0.0, 0.0, 0.0, 0.0, 1.0);
		meshSetVertex(mesh, 4, v);
		vecSet(8, v, right, bottom, lid, 1.0, 0.0, 0.0, 0.0, 1.0);
		meshSetVertex(mesh, 5, v);
		vecSet(8, v, right, top, lid, 1.0, 1.0, 0.0, 0.0, 1.0);
		meshSetVertex(mesh, 6, v);
		vecSet(8, v, left, top, lid, 0.0, 1.0, 0.0, 0.0, 1.0);
		meshSetVertex(mesh, 7, v);
		vecSet(8, v, left, top, base, 0.0, 1.0, 0.0, 1.0, 0.0);
		meshSetVertex(mesh, 8, v);
		vecSet(8, v, right, top, base, 1.0, 1.0, 0.0, 1.0, 0.0);
		meshSetVertex(mesh, 9, v);
		vecSet(8, v, right, top, lid, 1.0, 1.0, 0.0, 1.0, 0.0);
		meshSetVertex(mesh, 10, v);
		vecSet(8, v, left, top, lid, 0.0, 1.0, 0.0, 1.0, 0.0);
		meshSetVertex(mesh, 11, v);
		vecSet(8, v, left, bottom, base, 0.0, 0.0, 0.0, -1.0, 0.0);
		meshSetVertex(mesh, 12, v);
		vecSet(8, v, right, bottom, base, 1.0, 0.0, 0.0, -1.0, 0.0);
		meshSetVertex(mesh, 13, v);
		vecSet(8, v, right, bottom, lid, 1.0, 0.0, 0.0, -1.0, 0.0);
		meshSetVertex(mesh, 14, v);
		vecSet(8, v, left, bottom, lid, 0.0, 0.0, 0.0, -1.0, 0.0);
		meshSetVertex(mesh, 15, v);
		vecSet(8, v, right, top, base, 1.0, 1.0, 1.0, 0.0, 0.0);
		meshSetVertex(mesh, 16, v);
		vecSet(8, v, right, bottom, base, 1.0, 0.0, 1.0, 0.0, 0.0);
		meshSetVertex(mesh, 17, v);
		vecSet(8, v, right, bottom, lid, 1.0, 0.0, 1.0, 0.0, 0.0);
		meshSetVertex(mesh, 18, v);
		vecSet(8, v, right, top, lid, 1.0, 1.0, 1.0, 0.0, 0.0);
		meshSetVertex(mesh, 19, v);
		vecSet(8, v, left, top, base, 0.0, 1.0, -1.0, 0.0, 0.0);
		meshSetVertex(mesh, 20, v);
		vecSet(8, v, left, bottom, base, 0.0, 0.0, -1.0, 0.0, 0.0);
		meshSetVertex(mesh, 21, v);
		vecSet(8, v, left, bottom, lid, 0.0, 0.0, -1.0, 0.0, 0.0);
		meshSetVertex(mesh, 22, v);
		vecSet(8, v, left, top, lid, 0.0, 1.0, -1.0, 0.0, 0.0);
		meshSetVertex(mesh, 23, v);
		/* Now make vertex 0 for realsies. */
		vecSet(8, v, left, bottom, base, 0.0, 0.0, 0.0, 0.0, -1.0);
	}
	return error;
}

/* Rotates a 2-dimensional vector through an angle. The input can safely alias 
the output. */
void meshRotateVector(double theta, double v[2], double vRot[2]) {
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);
	double vRot0 = cosTheta * v[0] - sinTheta * v[1];
	vRot[1] = sinTheta * v[0] + cosTheta * v[1];
	vRot[0] = vRot0;
}

/* Rotate a curve about the Z-axis. Can be used to make a sphere, spheroid, 
capsule, circular cone, circular cylinder, box, etc. The z-values should be in 
ascending order --- or at least the first z should be less than the last. The 
first and last r-values should be 0.0, and no others. Probably the t-values 
should be in ascending or descending order. The sideNum parameter controls the 
fineness of the mesh. The attributes are XYZ position, ST texture, and NOP unit 
normal vector. The normals are smooth. Don't forget to meshDestroy when 
finished. */
int meshInitializeRevolution(meshMesh *mesh, int zNum, double z[], 
		double r[], double t[], int sideNum) {
	int i, j, error;
	error = meshInitialize(mesh, (zNum - 2) * sideNum * 2, 
		(zNum - 2) * (sideNum + 1) + 2, 3 + 2 + 3);
	if (error == 0) {
		/* Make the bottom triangles. */
		for (i = 0; i < sideNum; i += 1)
			meshSetTriangle(mesh, i, 0, i + 2, i + 1);
		/* Make the top triangles. */
		for (i = 0; i < sideNum; i += 1)
			meshSetTriangle(mesh, sideNum + i, mesh->vertNum - 1, 
				mesh->vertNum - 1 - (sideNum + 1) + i, 
				mesh->vertNum - 1 - (sideNum + 1) + i + 1);
		/* Make the middle triangles. */
		for (j = 1; j <= zNum - 3; j += 1)
			for (i = 0; i < sideNum; i += 1) {
				meshSetTriangle(mesh, 2 * sideNum * j + 2 * i,
					(j - 1) * (sideNum + 1) + 1 + i, 
					j * (sideNum + 1) + 1 + i + 1, 
					j * (sideNum + 1) + 1 + i);
				meshSetTriangle(mesh, 2 * sideNum * j + 2 * i + 1,
					(j - 1) * (sideNum + 1) + 1 + i, 
					(j - 1) * (sideNum + 1) + 1 + i + 1, 
					j * (sideNum + 1) + 1 + i + 1);
			}
		/* Make the vertices, using vertex 0 as temporary storage. */
		double *v = mesh->vert;
		double p[3], q[3], o[3];
		for (j = 1; j <= zNum - 2; j += 1) {
			// Form the sideNum + 1 vertices in the jth layer.
			vecSet(3, p, z[j + 1] - z[j], 0.0, r[j] - r[j + 1]);
			vecUnit(3, p, p);
			vecSet(3, q, z[j] - z[j - 1], 0.0, r[j - 1] - r[j]);
			vecUnit(3, q, q);
			vecAdd(3, p, q, o);
			vecUnit(3, o, o);
			vecSet(8, v, r[j], 0.0, z[j], 1.0, t[j], o[0], o[1], o[2]);
			meshSetVertex(mesh, j * (sideNum + 1), v);
			v[3] = 0.0;
			meshSetVertex(mesh, (j - 1) * (sideNum + 1) + 1, v);
			for (i = 1; i < sideNum; i += 1) {
				meshRotateVector(2 * M_PI / sideNum, v, v);
				v[3] += 1.0 / sideNum;
				meshRotateVector(2 * M_PI / sideNum, &v[5], &v[5]);
				meshSetVertex(mesh, (j - 1) * (sideNum + 1) + 1 + i, v);
			}
		}
		/* Form the top vertex. */
		vecSet(8, v, 0.0, 0.0, z[zNum - 1], 0.0, 0.0, 0.0, 0.0, 1.0);
		meshSetVertex(mesh, mesh->vertNum - 1, v);
		/* Finally form the bottom vertex. */
		vecSet(8, v, 0.0, 0.0, z[0], 0.0, 0.0, 0.0, 0.0, -1.0);
	}
	return error;
}

/* Builds a mesh for a sphere, centered at the origin, of radius r. The sideNum 
and layerNum parameters control the fineness of the mesh. The attributes are 
XYZ position, ST texture, and NOP unit normal vector. The normals are smooth. 
Don't forget to meshDestroy when finished. */
int meshInitializeSphere(meshMesh *mesh, double r, int layerNum, int sideNum) {
	int error, i;
	double *ts = (double *)malloc((layerNum + 1) * 3 * sizeof(double));
	if (ts == NULL)
		return 1;
	else {
		double *zs = &ts[layerNum + 1];
		double *rs = &ts[2 * layerNum + 2];
		for (i = 0; i <= layerNum; i += 1) {
			ts[i] = (double)i / layerNum;
			zs[i] = -r * cos(ts[i] * M_PI);
			rs[i] = r * sin(ts[i] * M_PI);
		}
		error = meshInitializeRevolution(mesh, layerNum + 1, zs, rs, ts, 
			sideNum);
		free(ts);
		return error;
	}
}

/* Builds a non-closed 'landscape' mesh based on a grid of Z-values. There are 
width * height Z-values, which arrive in the data parameter. The mesh is made 
of (width - 1) * (height - 1) squares, each made of two triangles. The spacing 
parameter controls the spacing of the X- and Y-coordinates of the vertices. The 
attributes are XYZ position, ST texture, and NOP unit normal vector. Don't 
forget to call meshDestroy when finished with the mesh. To understand the exact 
layout of the data, try this example code:
double zs[3][4] = {
	{10.0, 9.0, 7.0, 6.0}, 
	{6.0, 5.0, 3.0, 1.0}, 
	{4.0, 3.0, -1.0, -2.0}};
int error = meshInitializeLandscape(&mesh, 3, 4, 20.0, (double *)zs); */
int meshInitializeLandscape(meshMesh *mesh, int width, int height, 
		double spacing, double *data) {
	int i, j, error;
	int a, b, c, d;
	double *vert;
	error = meshInitialize(mesh, 2 * (width - 1) * (height - 1), 
		width * height, 3 + 2 + 3);
	if (error == 0) {
		/* Build the vertices with normals set to 0. */
		for (i = 0; i < width; i += 1)
			for (j = 0; j < height; j += 1) {
				vert = meshGetVertexPointer(mesh, i * height + j);
				vecSet(3 + 2 + 3, vert, i * spacing, j * spacing, 
					data[i * height + j], i / (width - 1.0), 
					j / (height - 1.0), 0.0, 0.0, 0.0);
				/*vecSet(3 + 2 + 3, vert, i * spacing, j * spacing, 
					data[i * height + j], (double)i, (double)j, 0.0, 0.0, 0.0);*/
			}
		/* Build the triangles. */
		for (i = 0; i < width - 1; i += 1)
			for (j = 0; j < height - 1; j += 1) {
				int index = 2 * (i * (height - 1) + j);
				a = i * height + j;
				b = (i + 1) * height + j;
				c = (i + 1) * height + (j + 1);
				d = i * height + (j + 1);
				meshSetTriangle(mesh, index, a, b, c);
				meshSetTriangle(mesh, index + 1, a, c, d);
			}
		/* Set the normals. */
		meshSmoothNormals(mesh, 5);
	}
	return error;
}