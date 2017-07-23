// Isaac Haseley - Nathan Mannes - 2017

/* Feel free to read from this struct's members, but don't write to them except 
through the accessor functions. */
typedef struct sceneNode sceneNode;
struct sceneNode {
	double *unif;
	texTexture **tex;
	meshMesh *mesh;
	sceneNode *firstChild, *nextSibling;
};





/* Initializes a sceneNode struct. Uniforms and texture pointers are copied 
into the node. Pointers to the mesh, first child, and next sibling are copied. 
The user must remember to call sceneDestroy or sceneDestroyRecursively when 
finished. Returns 0 if no error occurred. */
int sceneInitialize(sceneNode *node, renRenderer *ren, double unif[], 
		texTexture **tex, meshMesh *mesh, sceneNode *firstChild, 
		sceneNode *nextSibling) {
	node->unif = (double *)malloc(ren->unifDim * sizeof(double) + 
		ren->texNum * sizeof(texTexture *));
	if (node->unif != NULL) {
		node->tex = (texTexture **)&(node->unif[ren->unifDim]);
		int k;
		for (k = 0; k < ren->unifDim; k += 1)
			node->unif[k] = unif[k];
		for (k = 0; k < ren->texNum; k += 1)
			node->tex[k] = tex[k];
		node->mesh = mesh;
		node->firstChild = firstChild;
		node->nextSibling = nextSibling;
	}
	return (node->unif == NULL);
}

/* Adds a sibling to the given node. The sibling shows up as the youngest of 
its siblings. */
void sceneAddSibling(sceneNode *node, sceneNode *sibling) {
	if (node->nextSibling == NULL)
		node->nextSibling = sibling;
	else
		sceneAddSibling(node->nextSibling, sibling);
}

/* Adds a child to the given node. The child shows up as the youngest of its 
siblings. */
void sceneAddChild(sceneNode *node, sceneNode *child) {
	if (node->firstChild == NULL)
		node->firstChild = child;
	else
		sceneAddSibling(node->firstChild, child);
}

/* Copies the (ren->unifDim)-dimensional vector from unif into the node. */
void sceneSetUniform(sceneNode *node, renRenderer *ren, double unif[]) {
	int i;
	for (i = 0; i < ren->unifDim; i += 1)
		node->unif[i] = unif[i];
}

/* Sets the node's ith texture to the given one. */
void sceneSetTexture(sceneNode *node, renRenderer *ren, int i, texTexture *tex) 
{
	if (0 <= i && i < ren->texNum)
		node->tex[i] = tex;
}

/* Renders the node, its younger siblings, and their descendants. If the node 
has no parent, then unifParent is NULL. Otherwise, unifParent is the parent 
node's uniform vector. */
void sceneRender(sceneNode *node, renRenderer *ren, double *unifParent) {
    ren->updateUniform(ren, node->unif, unifParent);
    meshRender(node->mesh, ren, node->unif, node->tex);
    if (node->firstChild != NULL) {
        sceneRender(node->firstChild, ren, node->unif);
    }
    if (node->nextSibling != NULL) {
        sceneRender(node->nextSibling, ren, unifParent);
    }
}

/* Deallocates the resources backing this scene node. Does not destroy the 
resources backing the mesh or textures. */
void sceneDestroy(sceneNode *node) {
	free(node->unif);
}

/* Calls sceneDestroy recursively on the node's descendants and younger 
siblings, and then on the node itself. */
void sceneDestroyRecursively(sceneNode *node) {
	if (node->firstChild != NULL)
		sceneDestroyRecursively(node->firstChild);
	if (node->nextSibling != NULL)
		sceneDestroyRecursively(node->nextSibling);
	sceneDestroy(node);
}