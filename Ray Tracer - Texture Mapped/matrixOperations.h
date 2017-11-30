#ifndef __matrixOperations_h__
#define __matrixOperations_h__

#include "parser.h"
#define PI 3.14159265
parser::Vec3f homtransl(parser::Vec3f& translation,parser::Vec3f& point);
parser::Vec3f homrot(parser::Vec4f& rotation,parser::Vec3f& point);
parser::Vec3f homscale(parser::Vec3f& translation,parser::Vec3f& point);
void rodrigues(parser::Vec4f& rotation,double Rotation[9]);
void matmult(double T[16],double Point[4],float Result[4]);
parser::Vec3f matmult_vectorized(std::vector<double>& T,parser::Vec3f& point);
void matsum(double lhs[9],double rhs[9],double result[9]);
void simplify(float M[4]);
void invert(std::vector<double>& m, std::vector<double>& inv);
void transpose(std::vector<double>& m, std::vector<double>& transpose);
void Tmult(std::vector<double>& T1,std::vector<double>& T2,std::vector<double>& Result);

#endif //__matrixOperations_h__
