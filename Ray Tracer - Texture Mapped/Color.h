/* 
    Color operations
*/

#ifndef __COLOR__
#define __COLOR__

#include "parser.h"
void FillColor(parser::Vec3f& RGB,std::vector<std::vector<std::vector<unsigned char> > >& image,int i,int j);
parser::Vec3f CalculateColor(parser::Vec3f& n,parser::Vec3f& wo,parser::Vec3f& wi, parser::PointLight& pl, 
            parser::Vec3f& diffuse,parser::Vec3f& specular,float phong_exponent);
#endif
