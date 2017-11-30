#include "Color.h"


void FillColor(parser::Vec3f& RGB,std::vector<std::vector<std::vector<unsigned char> > >& image,int i,int j){
    char Cr = CLAMP(RGB.x);
    char Cg = CLAMP(RGB.y);
    char Cb = CLAMP(RGB.z);
    image[i][j][0] = Cr;
    image[i][j][1] = Cg;
    image[i][j][2] = Cb;
}

parser::Vec3f CalculateColor(parser::Vec3f& n,parser::Vec3f& wo,parser::Vec3f& wi, parser::PointLight& pl,parser::Vec3f& diffuse,parser::Vec3f& specular,float phong_exponent){
    
    
    float wi_length = pow(wi.x,2) + pow(wi.y,2) + pow(wi.z,2); //r_square
    float light_intensity_r = pl.intensity.x / wi_length;
    float light_intensity_g = pl.intensity.y / wi_length;
    float light_intensity_b = pl.intensity.z / wi_length;                   
    parser::normalize(wi);
    parser::normalize(wo);
    parser::Vec3f h = wi + wo;
    parser::normalize(h);
    float tmp = dot(n,wi);

    float theta = std::max((float)0,tmp);

    float diffuse_light_intensity_r = light_intensity_r * theta*diffuse.x;
    float diffuse_light_intensity_g = light_intensity_g * theta*diffuse.y;
    float diffuse_light_intensity_b = light_intensity_b * theta*diffuse.z;
    
    
    float alpha = std::max((float)0,dot(n,h));
    float exp_alpha = pow(alpha,phong_exponent);
    float specular_r = specular.x * exp_alpha * light_intensity_r;
    float specular_g = specular.y * exp_alpha * light_intensity_g;
    float specular_b = specular.z * exp_alpha * light_intensity_b;
    
    parser::Vec3f result;
    result.x = diffuse_light_intensity_r + specular_r;
    result.y = diffuse_light_intensity_g + specular_g;
    result.z = diffuse_light_intensity_b + specular_b;
    return result;
}

