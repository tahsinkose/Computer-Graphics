#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "jpeg.h"
#include "ctpl_stl.h"
#include <climits>
#include <map>
#include <iomanip>
#include <chrono>
#include <algorithm>


using namespace std;
using  ns = chrono::nanoseconds;
using get_time = chrono::steady_clock ;

parser::Scene scene;
map<int,vector<parser::Vec3f> > mesh_normals;
map<int, parser::Vec3f> triangle_normals;

void normalize(parser::Vec3f& vec){
    float normal_length = sqrt(pow(vec.x,2) + pow(vec.y,2) + pow(vec.z,2));
    vec = vec * (1.0/normal_length);
}
float DOT_PRODUCT(const parser::Vec3f & lhs,const parser::Vec3f & rhs){
     return lhs.x*rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

parser::Vec3f CalculateColor(parser::Vec3f& n,parser::Vec3f& wo,parser::Vec3f& wi, parser::PointLight& pl,parser::Vec3f& diffuse,parser::Vec3f& specular,float phong_exponent){
    
    
    float wi_length = pow(wi.x,2) + pow(wi.y,2) + pow(wi.z,2); //r_square
    float light_intensity_r = pl.intensity.x / wi_length;
    float light_intensity_g = pl.intensity.y / wi_length;
    float light_intensity_b = pl.intensity.z / wi_length;                   
    normalize(wi);
    normalize(wo);
    parser::Vec3f h = wi + wo;
    normalize(h);
    float tmp = DOT_PRODUCT(n,wi);

    float theta = max((float)0,tmp);

    float diffuse_light_intensity_r = light_intensity_r * theta*diffuse.x;
    float diffuse_light_intensity_g = light_intensity_g * theta*diffuse.y;
    float diffuse_light_intensity_b = light_intensity_b * theta*diffuse.z;
    
    
    float alpha = max((float)0,DOT_PRODUCT(n,h));
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

void FillColor(parser::Vec3f& RGB,vector<vector<vector<unsigned char> > >& image,int i,int j){
    char Cr = CLAMP(RGB.x);
    char Cg = CLAMP(RGB.y);
    char Cb = CLAMP(RGB.z);
    image[i][j][0] = Cr;
    image[i][j][1] = Cg;
    image[i][j][2] = Cb;
}

bool not_zero(float A,float epsilon){
    if(A<epsilon && A>-epsilon)
        return false;
    return true;
}

bool TriangleIntersection(parser::Face& f,parser::Vec3f& d,parser::Vec3f& o,float epsilon,float& t_min){
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

bool SphereIntersection(parser::Sphere& sphere,parser::Vec3f& center,parser::Vec3f& o,parser::Vec3f& d,float epsilon,float& t_min){
    float R = sphere.radius;
    parser::Vec3f o_c = o - center;
    float d_o_c = DOT_PRODUCT(d,o_c);
    float d_d = DOT_PRODUCT(d,d);
    float discriminant = pow(d_o_c,2) - d_d * (DOT_PRODUCT(o_c,o_c) - R*R);
    if(discriminant<epsilon)
        return false;
    else if(!not_zero(discriminant,epsilon)){//Has one root
        float t = -d_o_c / d_d;
        if(t>epsilon && t<t_min){
            t_min = t;
            return true;
        }
    }
    else{
         float t1 = (-d_o_c + sqrt(discriminant)) / d_d;
         float t2 = (-d_o_c - sqrt(discriminant)) / d_d;
         if(t1>epsilon && t2>epsilon && (t1<t_min || t2<t_min)){
            t_min = (t1 < t2) ? t1 : t2;
            return true;
         }
         else if(t1>epsilon && t1<t_min){
            t_min = t1;
            return true;
         }
         else if(t2>epsilon && t2<t_min){
            t_min = t2;
            return true;
         }
         else
            return false; 
    }
}

bool ShadowIntersection(parser::Vec3f& wi,parser::Vec3f& shadow_source,float epsilon,float t_light){
    for(int obj = 0; obj < scene.meshes.size();obj++){
        parser::Mesh mesh = scene.meshes[obj];
        for(int face=0;face<mesh.faces.size();face++){
            parser::Face f = mesh.faces[face];
            if(TriangleIntersection(f,wi,shadow_source,epsilon,t_light))
                return true;
        }
    }
    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;
        if(TriangleIntersection(f,wi,shadow_source,epsilon,t_light))
            return true;                
    }

    for(int obj = 0; obj < scene.spheres.size();obj++){
        parser::Sphere sphere = scene.spheres[obj];
        parser::Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];
        if(SphereIntersection(sphere,center,shadow_source,wi,epsilon,t_light))
            return true;
    }
    return false;
}

parser::Vec3f recursive_ray_tracer(parser::Vec3f& d,parser::Vec3f& o,int recursion_depth){

    parser::Vec3f ambient_light = scene.ambient_light;
    parser::Vec3i background = scene.background_color;
    float triangle_t=FLT_MAX,sphere_t=FLT_MAX;
    float epsilon = scene.shadow_ray_epsilon;

    parser::Vec3f intersection_point;
    parser::Vec3f sphere_normal;
    parser::Vec3f normal;
    int material_id = 0;

    /*-------------------- INTERSECTION SUBROUTINE--------------------*/
    for(int obj = 0; obj < scene.meshes.size();obj++){
        parser::Mesh mesh = scene.meshes[obj];
        for(int face=0;face<mesh.faces.size();face++){
            parser::Face f = mesh.faces[face];
            if(TriangleIntersection(f,d,o,epsilon,triangle_t)){
                intersection_point = o + d*triangle_t;
                material_id = mesh.material_id;
                normal = mesh_normals[obj][face];
            }  
        }
    }
    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;
        if(TriangleIntersection(f,d,o,epsilon,triangle_t)){
            intersection_point = o + d*triangle_t;
            material_id = tri.material_id;
            normal = triangle_normals[obj];
        }                 
    }
    for(int obj = 0; obj < scene.spheres.size();obj++){
        parser::Sphere sphere = scene.spheres[obj];
        parser::Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];
        float R = sphere.radius;
        if(SphereIntersection(sphere,center,o,d,epsilon,sphere_t)){
            intersection_point = o + d*sphere_t;
            sphere_normal = (intersection_point - center ) * (1.0/R);
            material_id = sphere.material_id;   
        }
    }
    /*-------------------------END OF INTERSECTION SUBROUTINE----------*/
    parser::Vec3f ambient;
    parser::Vec3f diffuse;
    parser::Vec3f specular;
    parser::Vec3f mirror;
    float phong_exponent;
    parser::Vec3f RGB;
    RGB.x = 0;
    RGB.y = 0;
    RGB.z = 0;
    // IF material_id is zero, that means no intersection occured. Hence terminate the execution for specific pixel.
    if(material_id){
        ambient = scene.materials[material_id-1].ambient;
        diffuse = scene.materials[material_id-1].diffuse;
        specular = scene.materials[material_id-1].specular;
        mirror = scene.materials[material_id-1].mirror;
        phong_exponent = scene.materials[material_id-1].phong_exponent;
    }
    else
        return RGB;
    /*AMBIENT LIGHT is not AFFECTED by the shadows*/
    if((triangle_t>0 && triangle_t<FLT_MAX) || (sphere_t>0 && sphere_t<FLT_MAX)){
        RGB.x= ambient.x * ambient_light.x;
        RGB.y= ambient.y * ambient_light.y;
        RGB.z= ambient.z * ambient_light.z;
    }
    /*-------------------------------------------*/
    // Wr and reflection_point is needed to handle the recursive bouncing case.
    parser::Vec3f wr;
    parser::Vec3f reflection_point;
    // Start light loop for all point light sources.
    bool is_mirrored = false;
    for(int light=0;light<scene.point_lights.size();light++){
        parser::PointLight pl = scene.point_lights[light];
        if(triangle_t>0 && triangle_t < sphere_t){
            
            parser::Vec3f wi = pl.position - intersection_point;
            parser::Vec3f shadow_source = intersection_point + (wi * epsilon);
            float t_light = (pl.position - shadow_source) / wi;
            parser::Vec3f wo = d * -1;

            wr = (wo*-1) + (normal*2)*DOT_PRODUCT(normal,wo);
            reflection_point = intersection_point + wr*epsilon;

            if((mirror.x>epsilon || mirror.y>epsilon || mirror.z>epsilon) && recursion_depth>0 && !is_mirrored){
                parser::Vec3f mirror_radiance = recursive_ray_tracer(wr,reflection_point,recursion_depth-1);
                RGB.x += (mirror.x * mirror_radiance.x);
                RGB.y += (mirror.y * mirror_radiance.y);
                RGB.z += (mirror.z * mirror_radiance.z);

                is_mirrored = true;
            }

            if(ShadowIntersection(wi,shadow_source,epsilon,t_light))
                continue;
            if(DOT_PRODUCT(normal,wo) < 0.0)
                normal = normal * -1;
            parser::Vec3f diffuse_and_specular_intensity = CalculateColor(normal,wo,wi,pl,diffuse,specular,phong_exponent);
            RGB = RGB + diffuse_and_specular_intensity;
        }
        else if(sphere_t>epsilon && sphere_t< FLT_MAX ){
            parser::Vec3f wi = pl.position - intersection_point;
            parser::Vec3f shadow_source = intersection_point + (wi * epsilon);
            float t_light = (pl.position - shadow_source) / wi;

            parser::Vec3f wo = d * -1;
            wr = (wo*-1) + (sphere_normal*2)*DOT_PRODUCT(sphere_normal,wo);
            reflection_point = intersection_point + wr*epsilon;

            if((mirror.x>epsilon || mirror.y>epsilon || mirror.z>epsilon) && recursion_depth>0 && !is_mirrored){
                parser::Vec3f mirror_radiance = recursive_ray_tracer(wr,reflection_point,recursion_depth-1);
                RGB.x += (mirror.x * mirror_radiance.x);
                RGB.y += (mirror.y * mirror_radiance.y);
                RGB.z += (mirror.z * mirror_radiance.z);

                is_mirrored = true;
            }
            
            if(ShadowIntersection(wi,shadow_source,epsilon,t_light))
                continue;
            if(DOT_PRODUCT(sphere_normal,wo) <  0.0)
                sphere_normal = sphere_normal * -1;
            parser::Vec3f diffuse_and_specular_intensity = CalculateColor(sphere_normal,wo,wi,pl,diffuse,specular,phong_exponent);
            RGB = RGB + diffuse_and_specular_intensity;        
        }
        else{
            RGB.x = background.x;
            RGB.y = background.y;
            RGB.z = background.z;
        }
    }
    return RGB;
}
int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    auto start = get_time::now();
    
    scene.loadFromXml(argv[1]);
    
    
    vector<parser::Camera> cameras = scene.cameras; //There will be as many output pictures as camera number.
    

    int recursion_depth = scene.max_recursion_depth;

    for(int obj = 0; obj < scene.meshes.size();obj++){
        parser::Mesh mesh = scene.meshes[obj];
        for(int face=0;face<mesh.faces.size();face++){
            parser::Face f = mesh.faces[face];

            parser::Vec3f p0 = scene.vertex_data[f.v0_id - 1];
            parser::Vec3f p1 = scene.vertex_data[f.v1_id - 1];
            parser::Vec3f p2 = scene.vertex_data[f.v2_id - 1];

            parser::Vec3f normal = (p1 - p0) * (p2 - p0);
            normalize(normal);
            mesh_normals[obj].push_back(normal);
        }
    }

    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;

        parser::Vec3f p0 = scene.vertex_data[f.v0_id - 1];
        parser::Vec3f p1 = scene.vertex_data[f.v1_id - 1];
        parser::Vec3f p2 = scene.vertex_data[f.v2_id - 1];

        parser::Vec3f normal = (p1 - p0) * (p2 - p0);
        normalize(normal);
        triangle_normals[obj] = normal;
    }


    for(int k=0;k<cameras.size();k++){
        parser::Camera camera = cameras[k];
        parser::Vec3f m = camera.position + camera.gaze * camera.near_distance; // m = e + -w.distance -> m is point of gaze vector in plane.
        
        parser::Vec3f v = camera.up;
        
        parser::Vec3f u = v * (camera.gaze*-1);
        
        float left = camera.near_plane.x;
        float right = camera.near_plane.y;
        float bottom = camera.near_plane.z;
        float top = camera.near_plane.w;
        float right_left = right-left;
        float top_bottom = top-bottom;

        parser::Vec3f q = m + (u*left) + (v*top);
        
        int height = camera.image_height;
        int width = camera.image_width;
        vector<vector<vector<unsigned char> > > image (height,vector<vector<unsigned char> > (width, vector<unsigned char>(3)));
        ctpl::thread_pool pool(height);
        // Spatial Localization is exploited. Though, gain is not as expected.
        int shared_var = 0;
        for(int i=0;i<height;i++){
            pool.push([&pool,i, &q, &u,&v, &camera, &image, &height, &width, &right_left, &top_bottom, &recursion_depth,&shared_var](int id){
                float s_v = top_bottom * (i + 0.5)/height;
                for(int j=0;j<width;j++){
                    float s_u = right_left * (j + 0.5)/width;
                    parser::Vec3f s = q + (u*s_u) - (v*s_v);
                    parser::Vec3f o = camera.position;
                    parser::Vec3f d = s - o;
                    parser::Vec3f RGB = recursive_ray_tracer(d,o,recursion_depth);
                    FillColor(RGB,image,i,j);
                }
            });
        }
        pool.stop(true);
        //write_ppm(camera.image_name.c_str(),image,width,height);
        string delimiter = ".";
        string token = camera.image_name.substr(0, camera.image_name.find(delimiter));
        
        token.append(".jpeg");
        write_jpeg(token.c_str(), image, width, height);
    }
    auto end = get_time::now();
    auto differ = end - start;
    cout<<"Elapsed time is :  "<< chrono::duration_cast<ns>(differ).count()<<" ns "<<endl;
    return 0;
}
