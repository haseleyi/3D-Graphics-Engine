// Isaac Haseley - Nathan Mannes - 2017

/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(double m[2][2]) {
	for (int i = 0; i < 2; i++) {
			printf("[ %f  ,  %f ]", m[i][0], m[i][1]);
	}
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. */
double mat22Invert(double m[2][2], double mInv[2][2]) {
	double det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
	if (det != 0) {
        double detFrac = 1 / det;
        mInv[0][0] = detFrac * m[1][1];
        mInv[0][1] = -detFrac * m[0][1];
        mInv[1][0] = -detFrac * m[1][0];
        mInv[1][1] = detFrac * m[0][0];
	}
	return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. */
void mat221Multiply(double m[2][2], double v[2], double mTimesV[2]) {
	mTimesV[0] = m[0][0] * v[0] + m[0][1] * v[1];
	mTimesV[1] = m[1][0] * v[0] + m[1][1] * v[1];
}

/* Fills the matrix m from its two columns. */
void mat22Columns(double col0[2], double col1[2], double m[2][2]) {
	m[0][0] = col0[0];
    m[0][1] = col1[0];
    m[1][0] = col0[1];
    m[1][1] = col1[1];
}

// Scales the 3x3 matrix
void mat33Scale(double c, double m[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m[i][j] = c * m[i][j];
        }
    }
}

// Adds the two 3x3 matrices
void mat33Add(double m[3][3], double n[3][3], double mPlusN[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            mPlusN[i][j] = m[i][j] + n[i][j];
        }
    }
}

/* Fills the matrix m from its three columns. */
void mat33Columns(double col0[3], double col1[3], double col2[3], 
    double m[3][3]) {
	m[0][0] = col0[0];
    m[0][1] = col1[0];
    m[0][2] = col2[0];
    m[1][0] = col0[1];
    m[1][1] = col1[1];
    m[1][2] = col2[1];
    m[2][0] = col0[2];
    m[2][1] = col1[2];
    m[2][2] = col2[2];
}

// Transposes the matrix m
void mat33Transpose(double m[3][3], double mT[3][3]) {
    mT[0][0] = m[0][0];
    mT[0][1] = m[1][0];
    mT[0][2] = m[2][0];
    mT[1][0] = m[0][1];
    mT[1][1] = m[1][1];
    mT[1][2] = m[2][1];
    mT[2][0] = m[0][2];
    mT[2][1] = m[1][2];
    mT[2][2] = m[2][2];
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. */
void mat333Multiply(double m[3][3], double n[3][3], double mTimesN[3][3]) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            mTimesN[r][c] = 
                m[r][0] * n[0][c] + m[r][1] * n[1][c] + m[r][2] * n[2][c];
        }
    }
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. */
void mat331Multiply(double m[3][3], double v[3], double mTimesV[3]) {
    for (int i = 0; i < 3; i++) {
        mTimesV[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2];      
    }
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector (x, y). 
*/
void mat33Isometry(double theta, double x, double y, double isom[3][3]) {
    isom[0][0] = cos(theta);
    isom[0][1] = -sin(theta);
    isom[0][2] = x;
    isom[1][0] = sin(theta);
    isom[1][1] = cos(theta);
    isom[1][2] = y;
    isom[2][0] = 0;
    isom[2][1] = 0;
    isom[2][2] = 1;
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the 
rotation matrix for the rotation about that axis through that angle. Based on 
Rodrigues' rotation formula R = I + (sin theta) U + (1 - cos theta) U^2. */
void mat33AngleAxisRotation(double theta, double axis[3], double rot[3][3]) {
    double i[3][3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    double u[3][3] = {
        {0, -axis[2], axis[1]},
        {axis[2], 0, -axis[0]},
        {-axis[1], axis[0], 0}
    };
    double uSquared[3][3];
    mat333Multiply(u, u, uSquared);
    mat33Scale(sin(theta), u);
    mat33Scale(1 - cos(theta), uSquared);
    mat33Add(i, u, i);
    mat33Add(i, uSquared, rot);    
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other. 
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds 
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(double u[3], double v[3], double a[3], double b[3], 
        double rot[3][3]) {
    double r[3][3], s[3][3], rT[3][3];
    double uCrossV[3], aCrossB[3];
    vec3Cross(u, v, uCrossV);
    vec3Cross(a, b, aCrossB);
    mat33Columns(u, v, uCrossV, r);
    mat33Columns(a, b, aCrossB, s);
    mat33Transpose(r, rT);
    mat333Multiply(s, rT, rot);
}

/* Multiplies m by n, placing the answer in mTimesN. */
void mat444Multiply(double m[4][4], double n[4][4], double mTimesN[4][4]) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            mTimesN[r][c] = m[r][0] * n[0][c] + m[r][1] 
                * n[1][c] + m[r][2] * n[2][c] + m[r][3]
                * n[3][c];
        }
    }
}

/* Multiplies m by v, placing the answer in mTimesV. */
void mat441Multiply(double m[4][4], double v[4], double mTimesV[4]) {
    for (int i = 0; i < 4; i++) {
        mTimesV[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2]
            + m[i][3] * v[3];      
    }
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix 
representing the rotation followed in time by the translation. */
void mat44Isometry(double rot[3][3], double trans[3], double isom[4][4]) {
    isom[0][0] = rot[0][0];
    isom[0][1] = rot[0][1];
    isom[0][2] = rot[0][2];
    isom[0][3] = trans[0];
    isom[1][0] = rot[1][0];
    isom[1][1] = rot[1][1];
    isom[1][2] = rot[1][2];
    isom[1][3] = trans[1];
    isom[2][0] = rot[2][0];
    isom[2][1] = rot[2][1];
    isom[2][2] = rot[2][2];
    isom[2][3] = trans[2];
    isom[3][0] = 0;
    isom[3][1] = 0;
    isom[3][2] = 0;
    isom[3][3] = 1;
}

/* Given a rotation and translation, forms the 4x4 homogeneous matrix 
representing the inverse translation followed in time by the inverse
rotation. That is, the isom produced by this function is the inverse to the
isom produced by mat44Isometry on the same inputs. */
void mat44InverseIsometry(double rot[3][3], double trans[3], 
        double isom[4][4]){
    double temp[3][3], temp1[3];
    mat33Transpose(rot, temp);
    vecScale(3, -1, trans, temp1);
    mat44Isometry(temp, temp1, isom);
}

/* Builds a 4x4 matrix representing orthographic projection with a boxy
viewing volume [left, right] x [bottom, top] x [far, near]. That is, on the
near plane the box is the rectangle R = [left, right] x [bottom, top], and on
the far plane the box is the same rectangle R. Keep in mind that 0 > near >
far. Maps the viewing volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Orthographic(double left, double right, double bottom, double top, 
        double far, double near, double proj[4][4]) {
    proj[0][0] = 2 / (right - left);
    proj[0][1] = 0;
    proj[0][2] = 0;
    proj[0][3] = (-right - left) / (right - left);
    
    proj[1][0] = 0;
    proj[1][1] = 2 / (top - bottom);
    proj[1][2] = 0;
    proj[1][3] = (-top - bottom) / (top - bottom);
    
    proj[2][0] = 0;
    proj[2][1] = 0;
    proj[2][2] = 2 / (near - far);
    proj[2][3] = (-near - far) / (near - far);
    
    proj[3][0] = 0;
    proj[3][1] = 0;
    proj[3][2] = 0;
    proj[3][3] = 1;   
}

/* Builds a 4x4 matrix that maps a projected viewing volume 
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w - 1] x [0, h - 1] x [-1, 1]. */
void mat44Viewport(double width, double height, double view[4][4]) {
    view[0][0] = (width - 1) / 2;
    view[0][1] = 0;
    view[0][2] = 0;
    view[0][3] = view[0][0];
    
    view[1][0] = 0;
    view[1][1] = (height - 1) / 2;
    view[1][2] = 0;
    view[1][3] = view[1][1];
    
    view[2][0] = 0;
    view[2][1] = 0;
    view[2][2] = 1;
    view[2][3] = 0;
    
    view[3][0] = 0;
    view[3][1] = 0;
    view[3][2] = 0;
    view[3][3] = 1;    
}

/* Builds a 4x4 matrix representing perspective projection. The viewing
frustum is contained between the near and far planes, with 0 > near > far. 
On the near plane, the frustum is the rectangle R = [left, right] x [bottom,
top]. On the far plane, the frustum is the rectangle (far / near) * R. Maps
the viewing volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Perspective(double left, double right, double bottom, double top, 
        double far, double near, double proj[4][4]) {
    proj[0][0] = (-2 * near) / (right - left);
    proj[0][1] = 0;
    proj[0][2] = (right + left) / (right - left);
    proj[0][3] = 0;
    
    proj[1][0] = 0;
    proj[1][1] = (-2 * near) / (top - bottom);
    proj[1][2] = (top + bottom) / (top - bottom);
    proj[1][3] = 0;
    
    proj[2][0] = 0;
    proj[2][1] = 0;
    proj[2][2] = (-near - far) / ( near - far);
    proj[2][3] = (2 * near * far) / (near - far);
    
    proj[3][0] = 0;
    proj[3][1] = 0;
    proj[3][2] = -1;
    proj[3][3] = 0;
}