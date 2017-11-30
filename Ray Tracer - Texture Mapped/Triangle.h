/* This header includes triangle intersection routines which are used in
   Mesh, Mesh-instance and Triangle objects.
*/

#ifndef __TRIANGLE__
#define __TRIANGLE__

#include "parser.h"

bool TriangleIntersection(parser::Scene& s,parser::Face& f,parser::Vec3f& d,parser::Vec3f& o,float epsilon,float& t_min);
bool InstancedTriangleIntersection(parser::RealFace& f,parser::Vec3f& d,parser::Vec3f& o,float epsilon,float& t_min);
void FindBarycentric(parser::Vec3f& p,parser::Vec3f& a, parser::Vec3f& b, parser::Vec3f& c, float& Beta, float& Gama);

#endif
