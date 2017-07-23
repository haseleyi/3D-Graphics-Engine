// Isaac Haseley - Nathan Mannes - 2017

// Finds the vector d that lies on the line 
// between a and b on the clip plane where a 
// is clipped and b is not
void clipHelp(double a[], double b[], int vecDim, double d[]) {
    double t = (a[3] - a[2]) / (a[3] - a[2] + b[2] - b[3]);
    vecSubtract(vecDim, b, a, d);
    vecScale(vecDim, t, d, d);
    vecAdd(vecDim, a, d, d);
}

// Performs homogenous division and viewport transformation
// Calls triRender
void renderHelp(renRenderer *ren, double unif[], texTexture *tex[], 
        double a[], double b[], double c[]) {
    double newA[4], newB[4], newC[4];
    vecScale(4, 1 / a[3], a, newA);
    vecScale(4, 1 / b[3], b, newB);
    vecScale(4, 1 / c[3], c, newC);
    mat441Multiply(ren->viewport, newA, a);
    mat441Multiply(ren->viewport, newB, b);
    mat441Multiply(ren->viewport, newC, c);
    triRender(ren, unif, tex, a, b, c);   
}

// Clips triangles at the near plane
void clipRender(renRenderer *ren, double unif[], texTexture *tex[], 
        double a[], double b[], double c[]) {
    int aIsClipped, bIsClipped, cIsClipped, numClipped;
    aIsClipped = 0;
    bIsClipped = 0;
    cIsClipped = 0;
    numClipped = 0;
    // Determines which vertices need to be clipped
    if (a[2] < a[3] || a[3] <= 0) {
        aIsClipped = 1;
        numClipped++;
    }
    if (b[2] < b[3] || b[3] <= 0) {
        bIsClipped = 1;
        numClipped++;
    }
    if (c[2] < c[3] || c[3] <= 0) {
        cIsClipped = 1;
        numClipped++;        
    }
    if (numClipped == 3) {
        return;   
    }
    else if (numClipped == 0) {
        renderHelp(ren, unif, tex, a, b, c);   
    }
    else {
        double aC[renVARYDIMBOUND], bC[renVARYDIMBOUND];
        double *copyOfA = a;
        double *copyOfB = b;
        double *copyOfC = c;
        if(numClipped == 2) {
            printf("Clipping two\n");
            fflush(stdout);
            if (cIsClipped + bIsClipped == 2) {
                copyOfC = a;
                copyOfB = c;
                copyOfA = b;
            }
            else if (aIsClipped + cIsClipped == 2) {
                copyOfC = b;
                copyOfB = a;
                copyOfA = c;
            } 
            clipHelp(copyOfA, copyOfC, ren->varyDim, aC);
            clipHelp(copyOfB, copyOfC, ren->varyDim, bC);
            renderHelp(ren, unif, tex, aC, bC, copyOfC);
        }
        else if (numClipped == 1) {
            printf("Clipping one\n");
            fflush(stdout);
            if (bIsClipped) {
                copyOfA = b;
                copyOfB = c;
                copyOfC = a;
            } 
            else if (cIsClipped) {
                copyOfA = c;
                copyOfB = a;
                copyOfC = b;
            } 
            clipHelp(copyOfA, copyOfC, ren->varyDim, aC);
            clipHelp(copyOfA, copyOfB, ren->varyDim, bC);
            renderHelp(ren, unif, tex, aC, b, copyOfC);
            renderHelp(ren, unif, tex, aC, bC, copyOfB);
        }
    }
}