#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <string>
#include <vector>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <map>

#define EPSILON 0.0001
#define IS_ZERO(x) x<EPSILON && x>-EPSILON ? 0 : x
#define CLAMP(x) x<=255.0 ? (int)round(x) : 255
namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
    struct Vec3f
    {
        float x, y, z;
    };
    Vec3f operator+ (const Vec3f& lhs,const Vec3f& rhs);
    Vec3f operator- (const Vec3f& lhs,const Vec3f& rhs);
    Vec3f operator* (const Vec3f& lhs,const float& f); // Scalar product
    Vec3f operator* (const Vec3f& lhs,const Vec3f& rhs); // Cross product.
    float operator/ (const Vec3f& lhs,const Vec3f& rhs); // Scalar division
    float dot(const Vec3f & lhs,const Vec3f & rhs);
    bool not_zero(float A,float epsilon);
    void normalize(Vec3f& vec);
    std::ostream& operator<< (std::ostream& o, const Vec3f& lhs);

    struct Vec2i
    {
        int x, y;
    };
    struct Vec3i
    {
        int x, y, z;
    };

    struct Vec4f
    {
        float x, y, z, w;
    };
    std::ostream& operator<< (std::ostream& o, const Vec4f& lhs);
    struct Camera
    {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        Vec4f near_plane;
        float near_distance;
        int image_width, image_height;
        std::string image_name;
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
    };

    struct Material
    {
        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        Vec3f mirror;
        float phong_exponent;
    };

    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
    };

    struct RealFace
    {
        Vec3f v1;
        Vec3f v2;
        Vec3f v3; 
    };


    struct Mesh
    {
        int material_id;
        std::vector<Face> faces;
        std::vector<std::string> transformations;
        int texture;
    };

    struct MeshInstance
    {
        int material_id;
        int base_mesh_instance;
        std::vector<std::string> transformations;
        std::vector<RealFace> faces;
        int texture;
    };

    struct Triangle
    {
        int material_id;
        Face indices;
        std::vector<std::string> transformations;
        int texture;
    };
    struct Texture
    {
        std::string image_name;
        std::string interpolation;
        std::string decalmode;
        std::string appearance;
        int width;
        int height;
    };
    struct Sphere
    {
        int material_id;
        int center_vertex_id;
        float radius;
        parser::Vec3f center;
        std::vector<std::string> transformations;
        int texture;
    };
    struct Translation
    {
        Vec3f transl;
    };

    struct Rotation
    {
        Vec4f rot; // mit Peine und Koordinaten.
    };
    
    struct Scaling
    {
        Vec3f scalar;
    };
    struct Scene
    {
        //Data
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<Camera> cameras;
        Vec3f ambient_light;
        std::vector<PointLight> point_lights;
        std::vector<Material> materials;
        std::vector<Vec3f> vertex_data;
        std::vector<Mesh> meshes;
        std::vector<MeshInstance> mesh_instances;
        std::vector<Triangle> triangles;
        std::vector<Sphere> spheres;
        std::vector<Vec2i> tex_coord_data;
        std::vector<Translation> translations;
        std::vector<Rotation> rotations;
        std::vector<Scaling> scalars;
        std::vector<Texture> textures;
        //Functions
        void loadFromXml(const std::string& filepath);
    };
}

#endif
