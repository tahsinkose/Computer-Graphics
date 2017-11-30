#include <iostream>
#include "ppm.h"
#include <climits>
#include <iomanip>
#include <chrono>

#include "jpeg.h"
#include "ctpl_stl.h"

#include "Triangle.h"
#include "Color.h"
#include "Transformation.h"
#include "matrixOperations.h"

using namespace std;
using  ns = chrono::nanoseconds;
using get_time = chrono::steady_clock ;

parser::Scene scene;
map<int,vector<parser::Vec3f> > mesh_normals;
map<int,vector<parser::Vec3f> > mesh_instance_normals;
map<int, parser::Vec3f> triangle_normals;
vector<vector<double> > adjunct_matrices;
vector<pair< vector<double>, vector<double> > > transformation_matrices; //first for the origin,second for the dimension.
vector<pair< vector<double>, vector<double> > > inverse_transformation_matrices; // first for the origin, second for the direction.
vector<vector<vector<vector<unsigned char > > > > textureImages;


bool SphereIntersection(parser::Sphere& sphere,parser::Vec3f& center,parser::Vec3f& o,parser::Vec3f& d,float epsilon,float& t_min){
    float R = sphere.radius;
    parser::Vec3f o_c = o - center;
    float d_o_c = dot(d,o_c);
    float d_d = dot(d,d);
    float discriminant = pow(d_o_c,2) - d_d * (dot(o_c,o_c) - R*R);
    if(discriminant<epsilon)
        return false;
    else if(!parser::not_zero(discriminant,epsilon)){//Has one root
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
            if(TriangleIntersection(scene,f,wi,shadow_source,epsilon,t_light))
                return true;
        }
    }
    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;
        if(TriangleIntersection(scene,f,wi,shadow_source,epsilon,t_light))
            return true;                
    }

    for(int obj = 0; obj < scene.mesh_instances.size();obj++){
        parser::MeshInstance mi = scene.mesh_instances[obj];
        for(int face=0;face<mi.faces.size();face++){
            parser::RealFace f = mi.faces[face];
            if(InstancedTriangleIntersection(f,wi,shadow_source,epsilon,t_light))
                return true;
        }
    }
    for(int obj = 0; obj < scene.spheres.size();obj++){
        parser::Sphere sphere = scene.spheres[obj];
        parser::Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];
        vector<double> Tinv = inverse_transformation_matrices[obj].first;
        vector<double> DirectionTinv = inverse_transformation_matrices[obj].second;
        
        parser::Vec3f t_o = matmult_vectorized(Tinv,shadow_source);
        parser::Vec3f t_d = matmult_vectorized(DirectionTinv,wi);
        if(SphereIntersection(sphere,center,t_o,t_d,epsilon,t_light))
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
    parser::Vec3f t_o,t_d;
    parser::Vec3i tex_coord_indexes;
    parser::Vec3f a,b,c; //for texture mapping of triangles only.

    int material_id = 0;
    int texture_id=0;
    bool textured_triangle = false;
    bool textured_sphere = false;
    float radius; // will be used only in sphere calculations.
    int which_sphere = -1;
    /*-------------------- INTERSECTION SUBROUTINE--------------------*/
    
    
    for(int obj = 0; obj < scene.meshes.size();obj++){
        parser::Mesh mesh = scene.meshes[obj];
        for(int face=0;face<mesh.faces.size();face++){
            parser::Face f = mesh.faces[face];
            if(TriangleIntersection(scene,f,d,o,epsilon,triangle_t)){
                intersection_point = o + d*triangle_t;
                material_id = mesh.material_id;
                normal = mesh_normals[obj][face];
                if(mesh.texture){
                    texture_id = mesh.texture;
                    textured_triangle =true;
                    textured_sphere = false;
                    // For texture mapping, these 3 points are necessary.
                    a = scene.vertex_data[f.v0_id - 1];
                    b = scene.vertex_data[f.v1_id - 1];
                    c = scene.vertex_data[f.v2_id - 1];
                    tex_coord_indexes = {f.v0_id-1, f.v1_id-1,f.v2_id-1};
                }
            }  
        }
    }
    
    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;
        if(TriangleIntersection(scene,f,d,o,epsilon,triangle_t)){
            intersection_point = o + d*triangle_t;
            material_id = tri.material_id;
            normal = triangle_normals[obj];
            if(tri.texture){
                texture_id = tri.texture;
                textured_triangle =true;
                textured_sphere = false;
            }
        }                 
    }
    
    for(int obj = 0; obj < scene.mesh_instances.size();obj++){
        parser::MeshInstance mi = scene.mesh_instances[obj];
        for(int face=0;face<mi.faces.size();face++){
            parser::RealFace f = mi.faces[face];
            if(InstancedTriangleIntersection(f,d,o,epsilon,triangle_t)){
                intersection_point = o + d*triangle_t;
                material_id = mi.material_id;
                normal = mesh_instance_normals[obj][face];
                if(mi.texture){
                    texture_id = mi.texture;
                    textured_triangle = true;
                    textured_sphere = false;
                }
            }
        }
    }
    for(int obj = 0; obj < scene.spheres.size();obj++){
        parser::Sphere sphere = scene.spheres[obj];
        parser::Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];

        vector<double> Tinv = inverse_transformation_matrices[obj].first;
        vector<double> DirectionTinv = inverse_transformation_matrices[obj].second;
        
        t_o = matmult_vectorized(Tinv,o);
        t_d = matmult_vectorized(DirectionTinv,d);

        if(SphereIntersection(sphere,center,t_o,t_d,epsilon,sphere_t)){

            intersection_point = t_o + t_d*sphere_t;
            sphere_normal = (intersection_point - center);// sphere_normal should be calculated.
            sphere_normal = matmult_vectorized(adjunct_matrices[obj],sphere_normal); 
            normalize(sphere_normal);
            material_id = sphere.material_id;
            intersection_point = o + d*sphere_t; //Map intersection point to the world space for lighting calculations
            if(sphere.texture){
                texture_id = sphere.texture;
                textured_triangle = false;
                textured_sphere = true;
                which_sphere = obj;
            } 
            radius = sphere.radius;
            
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

            wr = (wo*-1) + (normal*2)*dot(normal,wo);
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
            if(dot(normal,wo) < 0.0)
                normal = normal * -1;
            
            parser::Vec3f diffuse_and_specular_intensity;
            if(!textured_triangle) //There is no texture to map. 
                diffuse_and_specular_intensity = CalculateColor(normal,wo,wi,pl,diffuse,specular,phong_exponent);
            else{

                float Beta,Gama;
                FindBarycentric(intersection_point,a,b,c,Beta,Gama);
                int u_a = scene.tex_coord_data[tex_coord_indexes.x].x;
                int u_b = scene.tex_coord_data[tex_coord_indexes.y].x;
                int u_c = scene.tex_coord_data[tex_coord_indexes.z].x;

                int v_a = scene.tex_coord_data[tex_coord_indexes.x].y;
                int v_b = scene.tex_coord_data[tex_coord_indexes.y].y;
                int v_c = scene.tex_coord_data[tex_coord_indexes.z].y;
                
                float u = (u_a + Beta*(u_b-u_a) + Gama*(u_c-u_a));
                float v = (v_a + Beta*(v_b-v_a) + Gama*(v_c-v_a));
                 
                parser::Texture tex = scene.textures[texture_id-1];

                if(tex.appearance=="clamp"){
                    u = u >1.0 ? 1 : (u<0 ? 0 : u);
                    v = v >1.0 ? 1 : (v<0 ? 0 : v);
                }
                else{ // repeat
                    u = u - floor(u);
                    v = v - floor(v);
                }
                float i = u * tex.width;
                float j = v * tex.height;
                vector<unsigned char> TexelColor (3,0);
                
                if(tex.interpolation=="nearest"){
                    int ii = (i > tex.width-1) ? tex.width - 1 : round(i);
                    int jj = (j > tex.height-1) ? tex.height -1 : round(j);
                    
                    TexelColor = textureImages[texture_id-1][jj][ii];
                }
                else{ //bilinear

                    float p=floor(i),q=floor(j);
                    float dx = i - p, dy = j - q;
                    if(p!=tex.width-1 && q!=tex.height-1){
                        TexelColor[0] =(unsigned char) ((float)textureImages[texture_id-1][q][p][0] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][0] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][0] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][0] * dy * dx);
                        TexelColor[1] =(unsigned char) ((float)textureImages[texture_id-1][q][p][1] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][1] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][1] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][1] * dy * dx);
                        TexelColor[2] =(unsigned char) ((float)textureImages[texture_id-1][q][p][2] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][2] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][2] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][2] * dy * dx);
                        
                    }
                    else{
                        TexelColor[0] = 0;
                        TexelColor[1] = 0;
                        TexelColor[2] = 0;
                    }
                    
                }

                parser::Vec3f C = {(float)TexelColor[0]/255,(float)TexelColor[1]/255,(float)TexelColor[2]/255};
                if(tex.decalmode=="replace_kd")
                    diffuse_and_specular_intensity = CalculateColor(normal,wo,wi,pl,C,specular,phong_exponent);
                else if(tex.decalmode=="blend_kd"){
                    C = {(C.x + diffuse.x)/2,(C.y + diffuse.y)/2,(C.z + diffuse.z)/2};
                    diffuse_and_specular_intensity = CalculateColor(normal,wo,wi,pl,C,specular,phong_exponent);
                }
                else if(tex.decalmode=="replace_all"){
                    diffuse_and_specular_intensity = {(float)TexelColor[0],(float)TexelColor[1],(float)TexelColor[2]};
                    
                }
                // PLEASE NOTE HERE. When we looked at the slides, it is written there that we need only to change diffuse component.
                // But the outputs provided also cancel Ambient Light. In order to be consistent with the outputs provided, we are cancelling Ambient component here as well.
                diffuse_and_specular_intensity.x -= (ambient.x * ambient_light.x);
                diffuse_and_specular_intensity.y -= (ambient.y * ambient_light.y);
                diffuse_and_specular_intensity.z -= (ambient.z * ambient_light.z);

            }
            RGB = RGB + diffuse_and_specular_intensity;
        }
        else if(sphere_t>epsilon && sphere_t< FLT_MAX ){
            parser::Vec3f wi = pl.position - intersection_point;
            parser::Vec3f shadow_source = intersection_point + (wi * epsilon);

            float t_light = (pl.position - shadow_source) / wi;

            parser::Vec3f wo = d * -1;
            wr = (wo*-1) + (sphere_normal*2)*dot(sphere_normal,wo);
            
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
            if(dot(sphere_normal,wo) <  0.0)
                sphere_normal = sphere_normal * -1;
            parser::Vec3f diffuse_and_specular_intensity;
            if(!textured_sphere)
                 diffuse_and_specular_intensity= CalculateColor(sphere_normal,wo,wi,pl,diffuse,specular,phong_exponent);
            else{
                parser::Texture tex = scene.textures[texture_id-1];
                parser::Vec3f texture_inp = matmult_vectorized(inverse_transformation_matrices[which_sphere].first,intersection_point);
                float Theta; 
                float Phi;
                
                parser::Vec3f sphere_center = scene.vertex_data[scene.spheres[which_sphere].center_vertex_id - 1];
                texture_inp = texture_inp - sphere_center;
                Theta = acos(texture_inp.y/radius);
                
                Phi = atan2(texture_inp.z,texture_inp.x);
                    
                
                float u =(-Phi + PI) / (2*PI);
                float v = Theta / PI;

               if(tex.appearance=="clamp"){
                    u = u >1.0 ? 1 : (u<0 ? 0 : u);
                    v = v >1.0 ? 1 : (v<0 ? 0 : v);
                }
                else{ // repeat
                    u = u - floor(u);
                    v = v - floor(v);
                }
                

                float i = u * tex.width;
                float j = v * tex.height;
                vector<unsigned char> TexelColor (3);
                

                if(tex.interpolation=="nearest"){
                    int ii = (i > tex.width-1) ? tex.width - 1 : round(i);
                    int jj = (j > tex.height-1) ? tex.height -1 : round(j);
                    TexelColor = textureImages[texture_id-1][jj][ii];
                }
                else{ //bilinear
                    float p=floor(i),q=floor(j);
                    float dx = i - p, dy = j - q;
                   
                    if(p!=tex.width-1 && q!=tex.height-1){
                        TexelColor[0] =(unsigned char) ((float)textureImages[texture_id-1][q][p][0] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][0] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][0] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][0] * dy * dx);
                        TexelColor[1] =(unsigned char) ((float)textureImages[texture_id-1][q][p][1] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][1] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][1] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][1] * dy * dx);
                        TexelColor[2] =(unsigned char) ((float)textureImages[texture_id-1][q][p][2] * (1-dx) *(1-dy) +
                                     (float)textureImages[texture_id-1][q][p+1][2] * dx * (1-dy) + 
                                     (float)textureImages[texture_id-1][q+1][p][2] * dy *(1-dx) +
                                     (float)textureImages[texture_id-1][q+1][p+1][2] * dy * dx);
                    }
                    else{
                        TexelColor[0] = 0;
                        TexelColor[1] = 0;
                        TexelColor[2] = 0;
                    }
                }

                parser::Vec3f C = {(float)TexelColor[0]/255,(float)TexelColor[1]/255,(float)TexelColor[2]/255};
                if(tex.decalmode=="replace_kd")
                    diffuse_and_specular_intensity = CalculateColor(sphere_normal,wo,wi,pl,C,specular,phong_exponent);
                else if(tex.decalmode=="blend_kd"){
                    C = {(C.x + diffuse.x)/2,(C.y + diffuse.y)/2,(C.z + diffuse.z)/2};
                    diffuse_and_specular_intensity = CalculateColor(sphere_normal,wo,wi,pl,C,specular,phong_exponent);
                }
                else if(tex.decalmode=="replace_all")
                    diffuse_and_specular_intensity = {(float)TexelColor[0],(float)TexelColor[1],(float)TexelColor[2]};

                // PLEASE NOTE HERE. When we looked at the slides, it is written there that we need only to change diffuse component.
                // But the outputs provided also cancel Ambient Light. In order to be consistent with the outputs provided, we are cancelling Ambient component here as well.
                diffuse_and_specular_intensity.x -= ambient.x * ambient_light.x;
                diffuse_and_specular_intensity.y -= ambient.y * ambient_light.y;
                diffuse_and_specular_intensity.z -= ambient.z * ambient_light.z;
                // In case of output conflicts, these 3 lines can be commented out to reach to true outcomes.
            }

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
    textureImages.resize(scene.textures.size()); 

    for(int i=0;i<scene.textures.size();i++){
        read_jpeg_header(scene.textures[i].image_name.c_str(),scene.textures[i].width,scene.textures[i].height);
        vector<vector<vector<unsigned char > > > textureImage (scene.textures[i].height,vector<vector<unsigned char> >(scene.textures[i].width,vector<unsigned char>(3)));
        read_jpeg(scene.textures[i].image_name.c_str(),textureImage,scene.textures[i].width,scene.textures[i].height);
        textureImages[i] = textureImage; 
    }

    int recursion_depth = scene.max_recursion_depth;
    ApplyTriangularTransformations(scene,mesh_normals,triangle_normals,mesh_instance_normals);
    

    for(int obj = 0; obj < scene.spheres.size();obj++){
        parser::Sphere sphere = scene.spheres[obj];
        
        vector<double> T {1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1 
                         };
        vector<double> DirectionT {1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1 
                         };
        for(int trs=0;trs<sphere.transformations.size();trs++){
            string transformation = sphere.transformations[trs];
            char type;
            int id;    
            std::istringstream ss(transformation);
            ss >> type >> id;
            switch(type){
                case 't':
                {
                    parser::Vec3f transl = scene.translations[id-1].transl;
                    vector<double>Translation {1,0,0,transl.x,
                                               0,1,0,transl.y,
                                               0,0,1,transl.z,
                                               0,0,0,1};
                    vector<double> tmp(16);
                    Tmult(Translation,T,tmp);
                    T=tmp;
                    break;
                }
                case 'r':
                {   parser::Vec4f rot = scene.rotations[id-1].rot;
                    double R[9];
                    rodrigues(rot,R);
                    
                    vector<double> Rotation {R[0],R[1],R[2],0,
                                             R[3],R[4],R[5],0,
                                             R[6],R[7],R[8],0,
                                             0,0,0,1};
                    vector<double> tmp(16);
                    Tmult(Rotation,T,tmp);
                    T = tmp;
                    Tmult(Rotation,DirectionT,tmp);
                    DirectionT = tmp;
                    break;
                }
                case 's':
                {
                    parser::Vec3f scaling = scene.scalars[id-1].scalar;
                    vector<double> Scaling {scaling.x,0,0,0,
                                            0,scaling.y,0,0,
                                            0,0,scaling.z,0,
                                            0,0,0,1};
                    vector<double> tmp(16);
                    Tmult(Scaling,T,tmp);
                    T = tmp;
                    Tmult(Scaling,DirectionT,tmp);
                    DirectionT = tmp;
                    break;
                }
            }
        }
        // These all are must calculations for inverse ray tracing operation.
        vector<double> Tinv(16),DirectionTinv(16),adjunct(16);
        invert(T,Tinv);
        invert(DirectionT,DirectionTinv);
        transpose(DirectionTinv,adjunct);
        transformation_matrices.push_back({T,DirectionT});
        inverse_transformation_matrices.push_back({Tinv,DirectionTinv}); //Two matrices. One for origin, and the other for direction.
        adjunct_matrices.push_back(adjunct); //Not sure about it yet, but since Normals are also vectors, exclude translations as done in direction.
    }
    
    for(int k=0;k<cameras.size();k++){
        parser::Camera camera = cameras[k];
        camera.position.x -= camera.gaze.x;
        camera.position.y -= camera.gaze.y;
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
        write_ppm(camera.image_name.c_str(),image,width,height);
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

