#include "matrixOperations.h"
#include <stdexcept>


void Tmult(std::vector<double>& T1,std::vector<double>& T2,std::vector<double>& Result){
    //First row
    Result[0] = (T1[0] * T2[0]) + (T1[1]*T2[4]) + (T1[2]*T2[8]) + (T1[3]*T2[12]);
    Result[1] = (T1[0] * T2[1]) + (T1[1]*T2[5]) + (T1[2]*T2[9]) + (T1[3]*T2[13]);
    Result[2] = (T1[0] * T2[2]) + (T1[1]*T2[6]) + (T1[2]*T2[10]) + (T1[3]*T2[14]);
    Result[3] = (T1[0] * T2[3]) + (T1[1]*T2[7]) + (T1[2]*T2[11]) + (T1[3]*T2[15]);

    //Second row
    Result[4] = (T1[4] * T2[0]) + (T1[5]*T2[4]) + (T1[6]*T2[8]) + (T1[7]*T2[12]);
    Result[5] = (T1[4] * T2[1]) + (T1[5]*T2[5]) + (T1[6]*T2[9]) + (T1[7]*T2[13]);
    Result[6] = (T1[4] * T2[2]) + (T1[5]*T2[6]) + (T1[6]*T2[10]) + (T1[7]*T2[14]);
    Result[7] = (T1[4] * T2[3]) + (T1[5]*T2[7]) + (T1[6]*T2[11]) + (T1[7]*T2[15]);

    //Third row 
    Result[8] = (T1[8] * T2[0]) + (T1[9]*T2[4]) + (T1[10]*T2[8]) + (T1[11]*T2[12]);
    Result[9] = (T1[8] * T2[1]) + (T1[9]*T2[5]) + (T1[10]*T2[9]) + (T1[11]*T2[13]);
    Result[10] = (T1[8] * T2[2]) + (T1[9]*T2[6]) + (T1[10]*T2[10]) + (T1[11]*T2[14]);
    Result[11] = (T1[8] * T2[3]) + (T1[9]*T2[7]) + (T1[10]*T2[11]) + (T1[11]*T2[15]);

    //Forth row
    Result[12] = (T1[12] * T2[0]) + (T1[13]*T2[4]) + (T1[14]*T2[8]) + (T1[15]*T2[12]);
    Result[13] = (T1[12] * T2[1]) + (T1[13]*T2[5]) + (T1[14]*T2[9]) + (T1[15]*T2[13]);
    Result[14] = (T1[12] * T2[2]) + (T1[13]*T2[6]) + (T1[14]*T2[10]) + (T1[15]*T2[14]);
    Result[15] = (T1[12] * T2[3]) + (T1[13]*T2[7]) + (T1[14]*T2[11]) + (T1[15]*T2[15]);
    return;
}

/* BE VERY CAREFUL USING THIS FUNCTION. IT IS SPECIALLY DESIGNED FOR HOMOGENEOUS
   TRANSFORMATION OF A 3D POINT.*/
void matmult(double T[16],double Point[4],float Result[4]){
    Result[0] = (T[0] * Point[0]) + (T[1]*Point[1]) + (T[2]*Point[2]) + (T[3]*Point[3]);
    Result[1] = (T[4] * Point[0]) + (T[5]*Point[1]) + (T[6]*Point[2]) + (T[7]*Point[3]);
    Result[2] = (T[8] * Point[0]) + (T[9]*Point[1]) + (T[10]*Point[2]) + (T[11]*Point[3]);
    Result[3] = (T[12] * Point[0]) + (T[13]*Point[1]) + (T[14]*Point[2]) + (T[15]*Point[3]); 
}

parser::Vec3f matmult_vectorized(std::vector<double>& T,parser::Vec3f& point){
    parser::Vec3f result;
    result.x = (T[0] * point.x) + (T[1]*point.y) + (T[2]*point.z) + T[3];
    result.y = (T[4] * point.x) + (T[5]*point.y) + (T[6]*point.z) + T[7];
    result.z = (T[8] * point.x) + (T[9]*point.y) + (T[10]*point.z) + T[11];
    return result;
}
void matsum(double lhs[9],double rhs[9],double result[9]){
    for(int i=0;i<9;i++)
        result[i] = lhs[i] + rhs[i];
}

void rodrigues(parser::Vec4f& rotation,double Rotation[9]){
    parser::Vec3f v = {rotation.y,rotation.z,rotation.w}; // vector the rotation is performed about. 
    double I[9] ={0}; I[0] = 1; I[4] = 1; I[8] = 1;
    double theta = rotation.x; // angle of rotation
    double k = sin (theta*PI/180);
    double l = 1 - cos(theta*PI/180);

    double sinV_x[9] = {
        0, -k*v.z, k*v.y,
        k*v.z, 0, -k*v.x,
        -k*v.y, k*v.x, 0
    };
    double cosSquareV_x[9] = {
        l*(-pow(v.z,2) -pow(v.y,2)), l*v.y*v.x, l*v.z*v.x,
        l*v.x*v.y, l*(-pow(v.z,2) - pow(v.x,2)), l*v.z*v.y,
        l*v.z*v.x, l*v.y*v.z, l*(-pow(v.y,2) - pow(v.x,2))
    };

    matsum(I,sinV_x,Rotation); // R is not yet ready.
    matsum(Rotation,cosSquareV_x,Rotation); // R is ready now.
}

 
parser::Vec3f homtransl(parser::Vec3f& translation,parser::Vec3f& point){
    double T[16] = {
                1, 0, 0, translation.x,
                0, 1, 0, translation.y,
                0, 0, 1, translation.z,
                0, 0, 0, 1
                };
    double HomogeneousPoint[4] = { point.x,point.y,point.z,1};
    float Result[4];    
    matmult(T,HomogeneousPoint,Result);
    parser::Vec3f TransformedPoint = {Result[0],Result[1],Result[2]};
    return TransformedPoint;
}

parser::Vec3f homrot(parser::Vec4f& rotation,parser::Vec3f& point){
    double Rotation[9] ={0};
    rodrigues(rotation,Rotation);
    double R[16] = {
        Rotation[0],Rotation[1],Rotation[2],0,
        Rotation[3],Rotation[4],Rotation[5],0,
        Rotation[6],Rotation[7],Rotation[8],0,
        0,0,0,1
    };
    double HomogeneousPoint[4] = { point.x,point.y,point.z,1};
    float Result[4];
    matmult(R,HomogeneousPoint,Result);
    //simplify(Result);
    parser::Vec3f TransformedPoint = {Result[0],Result[1],Result[2]};    
    return TransformedPoint;
}

parser::Vec3f homscale(parser::Vec3f& scaling,parser::Vec3f& point){
    double S[16] = {0}; S[0] = scaling.x; S[5] = scaling.y; S[10] = scaling.z; S[15] = 1;
    double HomogeneousPoint[4] = { point.x,point.y,point.z,1};
    float Result[4];
    matmult(S,HomogeneousPoint,Result);
    //simplify(Result);
    parser::Vec3f TransformedPoint = {Result[0],Result[1],Result[2]};
    return TransformedPoint;
}

void simplify(float M[4]){
    for(int i=0;i<3;i++){
        if(M[i]<0.0001 && M[i]>-0.0001)
            M[i] = 0;
    }
}

void invert(std::vector<double>& m, std::vector<double>& inv)
{
    //
    // This code is taken from the GLU library 
    //    
    double det;

    inv[0] = m[5]  * m[10] * m[15] - 
        m[5]  * m[11] * m[14] - 
        m[9]  * m[6]  * m[15] + 
        m[9]  * m[7]  * m[14] +
        m[13] * m[6]  * m[11] - 
        m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
        m[4]  * m[11] * m[14] + 
        m[8]  * m[6]  * m[15] - 
        m[8]  * m[7]  * m[14] - 
        m[12] * m[6]  * m[11] + 
        m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
        m[4]  * m[11] * m[13] - 
        m[8]  * m[5] * m[15] + 
        m[8]  * m[7] * m[13] + 
        m[12] * m[5] * m[11] - 
        m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
        m[4]  * m[10] * m[13] +
        m[8]  * m[5] * m[14] - 
        m[8]  * m[6] * m[13] - 
        m[12] * m[5] * m[10] + 
        m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
        m[1]  * m[11] * m[14] + 
        m[9]  * m[2] * m[15] - 
        m[9]  * m[3] * m[14] - 
        m[13] * m[2] * m[11] + 
        m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
        m[0]  * m[11] * m[14] - 
        m[8]  * m[2] * m[15] + 
        m[8]  * m[3] * m[14] + 
        m[12] * m[2] * m[11] - 
        m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
        m[0]  * m[11] * m[13] + 
        m[8]  * m[1] * m[15] - 
        m[8]  * m[3] * m[13] - 
        m[12] * m[1] * m[11] + 
        m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
        m[0]  * m[10] * m[13] - 
        m[8]  * m[1] * m[14] + 
        m[8]  * m[2] * m[13] + 
        m[12] * m[1] * m[10] - 
        m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
        m[1]  * m[7] * m[14] - 
        m[5]  * m[2] * m[15] + 
        m[5]  * m[3] * m[14] + 
        m[13] * m[2] * m[7] - 
        m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
        m[0]  * m[7] * m[14] + 
        m[4]  * m[2] * m[15] - 
        m[4]  * m[3] * m[14] - 
        m[12] * m[2] * m[7] + 
        m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
        m[0]  * m[7] * m[13] - 
        m[4]  * m[1] * m[15] + 
        m[4]  * m[3] * m[13] + 
        m[12] * m[1] * m[7] - 
        m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
        m[0]  * m[6] * m[13] + 
        m[4]  * m[1] * m[14] - 
        m[4]  * m[2] * m[13] - 
        m[12] * m[1] * m[6] + 
        m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
        m[1] * m[7] * m[10] + 
        m[5] * m[2] * m[11] - 
        m[5] * m[3] * m[10] - 
        m[9] * m[2] * m[7] + 
        m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
        m[0] * m[7] * m[10] - 
        m[4] * m[2] * m[11] + 
        m[4] * m[3] * m[10] + 
        m[8] * m[2] * m[7] - 
        m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
        m[0] * m[7] * m[9] + 
        m[4] * m[1] * m[11] - 
        m[4] * m[3] * m[9] - 
        m[8] * m[1] * m[7] + 
        m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
        m[0] * m[6] * m[9] - 
        m[4] * m[1] * m[10] + 
        m[4] * m[2] * m[9] + 
        m[8] * m[1] * m[6] - 
        m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
    {
        throw std::runtime_error("Error: Determinant is zero.");
    }

    det = 1.0 / det;

    for (int i = 0; i < 16; ++i)
    {
        inv[i] *= det;
    }
}

void transpose(std::vector<double>& m, std::vector<double>& transpose){
    transpose[0] = m[0];
    transpose[1] = m[4];
    transpose[2] = m[8];
    transpose[3] = m[12];
    transpose[4] = m[1];
    transpose[5] = m[5];
    transpose[6] = m[9];
    transpose[7] = m[13];
    transpose[8] = m[2];
    transpose[9] = m[6];
    transpose[10] = m[10];
    transpose[11] = m[14];
    transpose[12] = m[3];
    transpose[13] = m[7];
    transpose[14] = m[11];
    transpose[15] = m[15];
}
