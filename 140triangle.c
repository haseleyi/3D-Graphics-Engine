// Isaac Haseley - Nathan Mannes - 2017

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

// Inputs: triangle vertices, target pixel, vertex colors, chi vector
// Interpolates to find chi associated with target pixel
// Sets chi vector with appropriate values
void setChi(double a[], double b[], double c[], int varyDim, 
            double myInverseMatrix[2][2], double littleChi[2], 
            double littleA[2], double chi[]) { 
    chi[0] = littleChi[0];
    chi[1] = littleChi[1];
    double pAndQ[2];
    double xVec[2];
    vecSubtract(2, littleChi, littleA, xVec);
    mat221Multiply(myInverseMatrix, xVec, pAndQ);
    double p = pAndQ[0];
    double q = pAndQ[1];
    for (int i = 2; i < varyDim; i++) {
        chi[i] = a[i] + p * (b[i] - a[i]) + q * (c[i] - a[i]); 
    }
}  

// Inputs: three vertices of a triangle and an RGB color
// Assumption: vertices are in counter-clockwise order
// Calls pixSetRGB with the given RGB for each pixel in triangle
void triRender(renRenderer *ren, double unif[], texTexture *tex[], 
        double a[], double b[], double c[]) {
    
    // Find minimum, middle, and maximum x0-values
    // Find x1-value at each of these points
    double ar[6] = {a[renVARYX], a[renVARYY], b[renVARYX], 
                    b[renVARYY], c[renVARYX], c[renVARYY]};
    if (!(ar[0] <= ar[2] && ar[0] <= ar[4])) {
        if (ar[0] == a[renVARYX] && ar[1] == a[renVARYY] 
            && ar[2] == b[renVARYX] && ar[3] == b[renVARYY] 
            && ar[4] == c[renVARYX] && ar[5] == c[renVARYY]) {
            ar[0] = b[renVARYX]; 
            ar[1] = b[renVARYY];
            ar[2] = c[renVARYX];
            ar[3] = c[renVARYY];
            ar[4] = a[renVARYX];
            ar[5] = a[renVARYY];
        }
    }
    if (!(ar[0] <= ar[2] && ar[0] <= ar[4])) {
        ar[0] = c[renVARYX];
        ar[1] = c[renVARYY];
        ar[2] = a[renVARYX];
        ar[3] = a[renVARYY];
        ar[4] = b[renVARYX];
        ar[5] = b[renVARYY];
    }
    if (ar[4] < ar[2]) {
        double first = ar[2];
        double second = ar[3];
        double third = ar[4];
        double fourth = ar[5];
        ar[2] = third;
        ar[3] = fourth;
        ar[4] = first;
        ar[5] = second;
    }
    double xMin = ar[0];
    double xMid = ar[2];
    double xMax = ar[4];
    double x1Start = ar[1];
    double x1Center = ar[3];
    double x1End = ar[5]; 
    
    if (fabs(xMid - xMax) < 50 && xMid != xMax) {
        xMid = round(xMid);
        xMax = round(xMax);
    }
    if (fabs(xMid - xMin) < 50 && xMid != xMin) {
        xMid = round(xMid);
        xMin = round(xMin);
    }
    if (fabs(xMin - xMax) < 50 && xMin != xMax) {
        xMin = round(xMin);
        xMax = round(xMax);
    }
    
    // Calculate slopes now to avoid repeated division later
    double minMidSlope, midMaxSlope, minMaxSlope;
    if (xMid != xMin) {
        minMidSlope = (x1Center - x1Start)/(xMid - xMin);
    }
    if (xMid != xMax) {
        midMaxSlope = (x1End - x1Center)/(xMax - xMid);
    }
    if (xMax != xMin) {
        minMaxSlope = (x1End - x1Start)/(xMax - xMin);
    }
    
    // Assemble screen coordinate vectors
    // Calculate inverse matrix only once per triangle
    double chi[ren->varyDim], littleChi[2];
    double rgbz[4];
    double littleA[2] = {a[renVARYX], a[renVARYY]};
    double littleB[2] = {b[renVARYX], b[renVARYY]};
    double littleC[2] = {c[renVARYX], c[renVARYY]};
    double col0[2], col1[2];
    vecSubtract(2, littleB, littleA, col0);
    vecSubtract(2, littleC, littleA, col1);
    double myMatrix[2][2], myInverseMatrix[2][2];
    mat22Columns(col0, col1, myMatrix);
    if (mat22Invert(myMatrix, myInverseMatrix) <= 0) {
        return;
    }
    
    // Rasterize. At each pixel,
    //      1. setChi interpolates relevant varyibutes
    //      2. coorPixel returns pixel's color via rgb[3]
    //      3. pixSetRGB takes care of the rest
    for (int x0 = (int)ceil(xMin); x0 <= (int)floor(xMax); x0++) {
        
        int test1 = floor(xMid);
        // Find targets up to middle x0-value
        if (x0 <= floor(xMid)) {
            if (minMidSlope > minMaxSlope && xMid != xMin) {
                for (int x1 = (int)ceil(x1Start + minMaxSlope * (x0 - xMin)); 
                  x1 <= (int)floor(x1Start + minMidSlope 
                  * (x0 - xMin)); x1++) {
                    double littleChi[2] = {x0, x1};
                    setChi(a, b, c, ren->varyDim, myInverseMatrix, 
                           littleChi, littleA, chi);
                    ren->colorPixel(ren, unif, tex, chi, rgbz);
                    if (rgbz[3] > depthGetZ(ren->depth, x0, x1)) {
                        depthSetZ(ren->depth, x0, x1, rgbz[3]);
                        pixSetRGB(x0, x1, rgbz[0], rgbz[1], rgbz[2]);
                    } 
                }
            }
            else if (minMidSlope < minMaxSlope && xMid != xMin) {
                for (int x1 = (int)ceil(x1Start + minMidSlope * (x0 - xMin)); 
                  x1 <= (int)floor(x1Start + minMaxSlope 
                  * (x0 - xMin)); x1++) {
                    double littleChi[2] = {x0, x1};
                    setChi(a, b, c, ren->varyDim, myInverseMatrix, 
                           littleChi, littleA, chi);
                    ren->colorPixel(ren, unif, tex, chi, rgbz);
                    if (rgbz[3] > depthGetZ(ren->depth, x0, x1)) {
                        depthSetZ(ren->depth, x0, x1, rgbz[3]);
                        pixSetRGB(x0, x1, rgbz[0], rgbz[1], rgbz[2]);
                    } 
                }
            }
        } 
        // Find targets from middle to maximum x0-values
        if (x0 >= floor(xMid)) {
            if (midMaxSlope < minMaxSlope && xMid != xMax) {
                for (int x1 = (int)ceil(x1Start + minMaxSlope * (x0 - xMin)); 
                  x1 <= (int)floor(x1Center + midMaxSlope 
                  * (x0 - xMid)); x1++) {
                    double littleChi[2] = {x0, x1};
                    setChi(a, b, c, ren->varyDim, myInverseMatrix, 
                           littleChi, littleA, chi);
                    ren->colorPixel(ren, unif, tex, chi, rgbz);
                    if (rgbz[3] > depthGetZ(ren->depth, x0, x1)) {
                        depthSetZ(ren->depth, x0, x1, rgbz[3]);
                        pixSetRGB(x0, x1, rgbz[0], rgbz[1], rgbz[2]);
                    } 
                }
            }
            else if (midMaxSlope > minMaxSlope && xMid != xMax) {
                for (int x1 = (int)ceil(x1Center + midMaxSlope * (x0 - xMid));
                  x1 <= (int)floor(x1Start + minMaxSlope
                  * (x0 - xMin)); x1++) {
                    double littleChi[2] = {x0, x1};
                    setChi(a, b, c, ren->varyDim, myInverseMatrix, 
                           littleChi, littleA, chi);
                    ren->colorPixel(ren, unif, tex, chi, rgbz);
                    if (rgbz[3] > depthGetZ(ren->depth, x0, x1)) {
                        depthSetZ(ren->depth, x0, x1, rgbz[3]);
                        pixSetRGB(x0, x1, rgbz[0], rgbz[1], rgbz[2]);
                    }  
                }
            }
        }    
    }
}