// Isaac Haseley - Nathan Mannes - 2017

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "000pixel.h"
#include "GLFW/glfw3.h"
#include "100vector.c"
#include "130matrix.c"
#include "040texture.c"
#include "110depth.c"
#include "130renderer.c"

#define renVARYDIMBOUND 16
#define renVERTNUMBOUND 10000

#define renATTRX 0
#define renATTRY 1
#define renATTRZ 2
#define renATTRS 3
#define renATTRT 4
#define renATTRN 5
#define renATTRO 6
#define renATTRP 7

#define renVARYX 0
#define renVARYY 1
#define renVARYZ 2
#define renVARYW 3
#define renVARYS 4
#define renVARYT 5
#define renVARYR 6
#define renVARYG 7
#define renVARYB 8

#define renTEXR 0
#define renTEXG 1
#define renTEXB 2

#define renUNIFR 0
#define renUNIFG 1
#define renUNIFB 2
#define renUNIFTRANSX 3
#define renUNIFTRANSY 4
#define renUNIFTRANSZ 5
#define renUNIFTHETA 6
#define renUNIFISOMETRY 7
#define renUNIFCAMERA 23

/* If unifParent is NULL, then sets the uniform matrix to the 
rotation-translation M described by the other uniforms. If unifParent is not 
NULL, but instead contains a rotation-translation P, then sets the uniform 
matrix to the matrix product P * M. */
void updateUniform(renRenderer *ren, double unif[], double unifParent[]) {      
    double trans[3] = {unif[renUNIFTRANSX], unif[renUNIFTRANSY],
        unif[renUNIFTRANSZ]};
    double rot[3][3] = {
        {cos(unif[renUNIFTHETA]), -sin(unif[renUNIFTHETA]), 0},
        {sin(unif[renUNIFTHETA]), cos(unif[renUNIFTHETA]), 0},
        {0, 0, 1}
    };  
    if (unifParent == NULL)
        /* The 16 uniforms for storing the matrix start at index 
        renUNIFISOMETRY. So &unif[renUNIFISOMETRY] is an array containing those 
        16 numbers. We use '(double(*)[4])' to cast it to a 4x4 matrix. */
        mat44Isometry(rot, trans, (double(*)[4])(&unif[renUNIFISOMETRY]));
    else {
        double m[4][4];
        mat44Isometry(rot, trans, m);
        mat444Multiply((double(*)[4])(&unifParent[renUNIFISOMETRY]), m, 
            (double(*)[4])(&unif[renUNIFISOMETRY]));
    }
    // Updates the camera matrix
    int index = renUNIFCAMERA;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            unif[index] = ren->viewing[i][j];
            index++;
        }
    } 
}

/* Writes the vary vector, based on the other parameters. */
void transformVertex(renRenderer *ren, double unif[], double attr[], 
        double vary[]) {
    int i = renUNIFISOMETRY;
    double initial[4] = {attr[renATTRX], attr[renATTRY], attr[renATTRZ], 1};
    double worldXYZ1[4], eyeXYZ1[4], clipXYZ1[4], screenXYZ1[4];
    mat441Multiply((double(*)[4])(&unif[renUNIFISOMETRY]), initial, 
        worldXYZ1);
    mat441Multiply((double(*)[4])(&unif[renUNIFCAMERA]), worldXYZ1, eyeXYZ1);
    mat441Multiply(ren->viewing, eyeXYZ1, clipXYZ1);
    vary[renVARYX] = clipXYZ1[0]; 
    vary[renVARYY] = clipXYZ1[1]; 
    vary[renVARYZ] = clipXYZ1[2];
    vary[renVARYW] = clipXYZ1[3];
    vary[renVARYS] = attr[renATTRS];
    vary[renVARYT] = attr[renATTRT];
}

/* Sets rgb, based on the other parameters, which are unaltered. vary is an 
interpolated varying vector. */
void colorPixel(renRenderer *ren, double unif[], texTexture *tex[], 
        double vary[], double rgbz[]) {   
    texSample(tex[0], vary[renVARYS], vary[renVARYT]);
    rgbz[0] = tex[0]->sample[renTEXR];
    rgbz[1] = tex[0]->sample[renTEXG];
    rgbz[2] = tex[0]->sample[renTEXB];
    rgbz[3] = vary[renVARYZ];
}

#include "110triangle.c"
#include "140clipping.c"
#include "140mesh.c"
#include "100scene.c"

sceneNode *rootPointer;
renRenderer renderer;
renRenderer *ren = &renderer; 
depthBuffer myBuffer;
depthBuffer *buf = &myBuffer;
texTexture *tex[4];

double turnAngle = 0;

// If arrow key, strafes camera in that direction
// Else, rotates meshes
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    pixClearRGB(0, 1, 1);
    depthClearZs(buf, -10000000);
    rootPointer->unif[renUNIFTRANSZ] += 10;
    renSetProjectionType(ren, 1);
    double proj[6] = {-3, 3, -3, 3, -1000, -1};       
    renSetProjection(ren, proj);
    sceneRender(rootPointer, ren, NULL);
}

int main(void) {
	
    // Fill in renderer values
    ren->unifDim = 3 + 1 + 3 + 16 + 16;
    ren->texNum = 1;
    ren->attrDim = 8;
    ren->varyDim = 9;
    ren->colorPixel = colorPixel;
    ren->updateUniform = updateUniform;
    ren->transformVertex = transformVertex;
    ren->depth = buf;
    
    // Initialize depth buffer
    if (depthInitialize(buf, 512, 512) != 0) {
        return 1;
    }
    
//    void renSetFrustum(renRenderer *ren, int projType, double fovy, double focal, double ratio) {
    
    /* Sets the projection type and the six projection parameters, based on the 
width and height and three other parameters. The camera looks down the center 
of the viewing volume. For perspective projection, fovy is the full 
(not half) vertical angle of the field of vision, in radians. focal > 0 is 
the distance from the camera to the 'focal' plane (where 'focus' is used in
the sense of attention, not optics). ratio expresses the far and near
clipping planes relative to focal: far = -focal * ratio and near = -focal /
ratio. Reasonable values are fovy = M_PI / 6.0, focal = 10.0, and ratio =
10.0, so that far = -100.0 and near = -1.0. For orthographic projection, the
projection parameters are set to produce the orthographic projection that, at
the focal plane, is most similar to the perspective projection just
described. */
    
    ren->cameraRotation[0][0] = 1;
    ren->cameraRotation[0][1] = 0;
    ren->cameraRotation[0][2] = 0;
    ren->cameraRotation[1][0] = 0;
    ren->cameraRotation[1][1] = 1;
    ren->cameraRotation[1][2] = 0;
    ren->cameraRotation[2][0] = 0;
    ren->cameraRotation[2][1] = 0;
    ren->cameraRotation[2][2] = 1;
    ren->cameraTranslation[0] = 0;
    ren->cameraTranslation[1] = 0;
    ren->cameraTranslation[2] = 0;
    renUpdateViewing(ren);
    
    /* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pantheon") != 0) {
		return 1;
    }
    else {
        
        // Initialize textures
        texTexture obamaTex;
        tex[0] = &obamaTex;
        texTexture batmanTex;
        tex[1] = &batmanTex;
        texTexture supermanTex;
        tex[2] = &supermanTex;
        texTexture spidermanTex;
        tex[3] = &spidermanTex;
        if (texInitializeFile(tex[0], "0000obama.jpg") != 0) {
            return 1;
        }
        if (texInitializeFile(tex[1], "0000batman.jpg") != 0) {
            return 1;
        }
        if (texInitializeFile(tex[2], "0000superman.jpg") != 0) {
            return 1;
        }
        if (texInitializeFile(tex[3], "0000spiderman.jpg") != 0) {
            return 1;
        }
        
        // Initialize meshes
        meshMesh obamaMesh, batmanMesh, supermanMesh, spidermanMesh;
        if (meshInitializeBox(&obamaMesh, -100, 100, -100, 100, -100, 100) 
            != 0) {
            return 1;
        }
        if (meshInitializeBox(&batmanMesh, -75, 75, -75, 75, -75, 75) 
            != 0) {
            return 1;
        }
        if (meshInitializeBox(&supermanMesh, -75, 75, -75, 75, -75, 75) 
            != 0) {
            return 1;
        }
        if (meshInitializeBox(&spidermanMesh, -60, 60, -60, 60, -60, 60) 
            != 0) {
            return 1;
        }

        // Initialize unifs
        double unif0[39] = {
            1, 1, 1, // rgb
            0, 200, -250, // xyz translation 
            0, // theta
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        double unif1[39] = {
            1, 1, 1, // rgb
            -100, -150, 30, // xyz translation 
            0, // theta
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        double unif2[39] = {
            1, 1, 1, // rgb
            100, -150, 30, // xyz translation 
            0, // theta
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        double unif3[39] = {
            1, 1, 1, // rgb
            100, -100, 30, // xyz translation 
            0, // theta
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        
        // Initialize scene nodes
        sceneNode root, batmanNode, supermanNode, spidermanNode;
        rootPointer = &root;
        sceneNode *batmanNodePointer = &batmanNode;
        sceneNode *supermanNodePointer = &supermanNode;
        sceneNode *spidermanNodePointer = &spidermanNode;
        // (Fourth args: pointers to arrays of tex pointers of len 1)
        if (sceneInitialize(rootPointer, ren, unif0, &tex[0], 
            &obamaMesh, batmanNodePointer, NULL) != 0) {
            return 1;
        }
        if (sceneInitialize(batmanNodePointer, ren, unif1 , &tex[1], 
            &batmanMesh, spidermanNodePointer, 
            supermanNodePointer) != 0) {
            return 1;
        }
        if (sceneInitialize(supermanNodePointer, ren, unif2, &tex[2], 
            &supermanMesh, NULL, NULL) != 0) {
            return 1;
        }
        if (sceneInitialize(spidermanNodePointer, ren, unif3, &tex[3], 
            &spidermanMesh, NULL, NULL) != 0) {
            return 1;
        }
        
        pixSetKeyUpHandler(handleKeyUp);
    
        renSetProjectionType(ren, 1);
        double proj[6] = {-3, 3, -3, 3, -1000, -1};      
        renSetProjection(ren, proj);   
        
        renUpdateViewing(ren);
        
        // Render everything 
        sceneRender(rootPointer, ren, NULL);
        
        pixRun();
        
        // Destroy everything
        for (int i = 0; i < ren->texNum; i++) {
            texDestroy(tex[i]);
        }
        meshDestroy(&obamaMesh);
        meshDestroy(&batmanMesh);
        meshDestroy(&supermanMesh);
        meshDestroy(&spidermanMesh);
        sceneDestroyRecursively(rootPointer);
        depthDestroy(buf);
        
        return 0;
    } 
}