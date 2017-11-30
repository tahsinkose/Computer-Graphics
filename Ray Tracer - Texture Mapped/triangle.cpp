#include "Triangle.h"

bool TriangleIntersection(parser::Scene& scene,parser::Face& f,parser::Vec3f& d,parser::Vec3f& o,float epsilon,float& t_min){
    parser::Vec3f a = scene.vertex_data[f.v0_id - 1];
    parser::Vec3f b = scene.vertex_data[f.v1_id - 1];
    parser::Vec3f c = scene.vertex_data[f.v2_id - 1];
   
    parser::Vec3f a_b = a-b;
    parser::Vec3f a_c = a-c;
    parser::Vec3f a_o = a-o;

    float detA = (a_b.x * (a_c.y * d.z - a_c.z * d.y)) 
    - (a_b.y *(a_c.x * d.z - a_c.z *d.x )) 
    + (a_b.z *(a_c.x*d.y - a_c.y*d.x));
    
    float Beta = (a_o.x * (a_c.y * d.z - a_c.z * d.y)) 
    - (a_o.y *(a_c.x * d.z - a_c.z *d.x )) 
    + (a_o.z *(a_c.x*d.y - a_c.y*d.x));
    Beta /= detA;

    float Gama = (a_b.x * (a_o.y * d.z - a_o.z * d.y)) 
    - (a_b.y *(a_o.x * d.z - a_o.z *d.x )) 
    + (a_b.z *(a_o.x*d.y - a_o.y*d.x));
    Gama /= detA;

    float t = (a_b.x * (a_c.y * a_o.z - a_c.z * a_o.y)) 
    - (a_b.y *(a_c.x * a_o.z - a_c.z *a_o.x )) 
    + (a_b.z *(a_c.x*a_o.y - a_c.y*a_o.x));
    t /= detA;

    if(Beta>-epsilon && Gama>-epsilon && (Beta+Gama<=1.0) && t>0 && t<t_min){
        t_min = t;
        return true;
    }
    return false;
}

void FindBarycentric(parser::Vec3f& p,parser::Vec3f& a, parser::Vec3f& b, parser::Vec3f& c, float& Beta, float& Gama){

    parser::Vec3f v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    Beta = (d11 * d20 - d01 * d21) / denom;
    Gama = (d00 * d21 - d01 * d20) / denom;
    return;

}


bool InstancedTriangleIntersection(parser::RealFace& f,parser::Vec3f& d,parser::Vec3f& o,float epsilon,float& t_min){
    parser::Vec3f a = f.v1;
    parser::Vec3f b = f.v2;
    parser::Vec3f c = f.v3;
    parser::Vec3f a_b = a-b;
    parser::Vec3f a_c = a-c;
    parser::Vec3f a_o = a-o;

    float detA = (a_b.x * (a_c.y * d.z - a_c.z * d.y)) 
    - (a_b.y *(a_c.x * d.z - a_c.z *d.x )) 
    + (a_b.z *(a_c.x*d.y - a_c.y*d.x));
    
    float Beta = (a_o.x * (a_c.y * d.z - a_c.z * d.y)) 
    - (a_o.y *(a_c.x * d.z - a_c.z *d.x )) 
    + (a_o.z *(a_c.x*d.y - a_c.y*d.x));
    Beta /= detA;

    float Gama = (a_b.x * (a_o.y * d.z - a_o.z * d.y)) 
    - (a_b.y *(a_o.x * d.z - a_o.z *d.x )) 
    + (a_b.z *(a_o.x*d.y - a_o.y*d.x));
    Gama /= detA;

    float t = (a_b.x * (a_c.y * a_o.z - a_c.z * a_o.y)) 
    - (a_b.y *(a_c.x * a_o.z - a_c.z *a_o.x )) 
    + (a_b.z *(a_c.x*a_o.y - a_c.y*a_o.x));
    t /= detA;

    if(Beta>-epsilon && Gama>-epsilon && (Beta+Gama<=1.0) && t>0 && t<t_min){
        t_min = t;
        return true;
    }
    return false;
}
