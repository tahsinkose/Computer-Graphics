/* This header includes triangle intersection routines which are used in
   Mesh, Mesh-instance and Triangle objects.
*/

#ifndef __TRANSFORMATION__
#define __TRANSFORMATION__

#include "parser.h"
#include "matrixOperations.h"

void ApplyTransformation(parser::Scene& scene,char type,int id,parser::Vec3f& p0,parser::Vec3f& p1,parser::Vec3f& p2);
void ApplyTriangularTransformations(parser::Scene& scene,std::map<int,std::vector<parser::Vec3f> >& mesh_normals,
   std::map<int, parser::Vec3f>& triangle_normals,std::map<int,std::vector<parser::Vec3f> >& mesh_instance_normals );

#endif
