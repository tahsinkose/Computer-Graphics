#include "parser.h"
#include "tinyxml2.h"
#include <sstream>
#include <stdexcept>

std::vector<std::string> split(const char *str, char c = ' ')
{
    std::vector<std::string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(std::string(begin, str));
    } while (0 != *str++);

    return result;
}
void parser::Scene::loadFromXml(const std::string& filepath)
{
    tinyxml2::XMLDocument file;
    std::stringstream stream;

    auto res = file.LoadFile(filepath.c_str());
    if (res)
    {
        throw std::runtime_error("Error: The xml file cannot be loaded.");
    }

    auto root = file.FirstChild();
    if (!root)
    {
        throw std::runtime_error("Error: Root is not found.");
    }

    //Get BackgroundColor
    auto element = root->FirstChildElement("BackgroundColor");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0 0 0" << std::endl;
    }
    stream >> background_color.x >> background_color.y >> background_color.z;

    //Get ShadowRayEpsilon
    element = root->FirstChildElement("ShadowRayEpsilon");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0.001" << std::endl;
    }
    stream >> shadow_ray_epsilon;

    //Get MaxRecursionDepth
    element = root->FirstChildElement("MaxRecursionDepth");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0" << std::endl;
    }
    stream >> max_recursion_depth;

    //Get Cameras
    element = root->FirstChildElement("Cameras");
    element = element->FirstChildElement("Camera");
    Camera camera;
    while (element)
    {
        auto child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Gaze");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Up");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("NearPlane");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("NearDistance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageResolution");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageName");
        stream << child->GetText() << std::endl;

        stream >> camera.position.x >> camera.position.y >> camera.position.z;
        stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
        stream >> camera.up.x >> camera.up.y >> camera.up.z;
        stream >> camera.near_plane.x >> camera.near_plane.y >> camera.near_plane.z >> camera.near_plane.w;
        stream >> camera.near_distance;
        stream >> camera.image_width >> camera.image_height;
        stream >> camera.image_name;

        cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }

    //Get Lightss
    std::cout<<"Lights"<<std::endl;
    element = root->FirstChildElement("Lights");
    auto child = element->FirstChildElement("AmbientLight");
    stream << child->GetText() << std::endl;
    stream >> ambient_light.x >> ambient_light.y >> ambient_light.z;
    element = element->FirstChildElement("PointLight");
    PointLight point_light;
    while (element)
    {
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Intensity");
        stream << child->GetText() << std::endl;

        stream >> point_light.position.x >> point_light.position.y >> point_light.position.z;
        stream >> point_light.intensity.x >> point_light.intensity.y >> point_light.intensity.z;

        point_lights.push_back(point_light);
        element = element->NextSiblingElement("PointLight");
    }

    //Get Materials
    std::cout<<"Materials"<<std::endl;
    element = root->FirstChildElement("Materials");
    element = element->FirstChildElement("Material");
    Material material;
    while (element)
    {
        child = element->FirstChildElement("AmbientReflectance");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> material.ambient.x >> material.ambient.y >> material.ambient.z;
        }
        child = element->FirstChildElement("DiffuseReflectance");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z;
        }
        child = element->FirstChildElement("SpecularReflectance");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> material.specular.x >> material.specular.y >> material.specular.z;
        }
        child = element->FirstChildElement("MirrorReflectance");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> material.mirror.x >> material.mirror.y >> material.mirror.z;
        }
        child = element->FirstChildElement("PhongExponent");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> material.phong_exponent;
        }
        materials.push_back(material);
        element = element->NextSiblingElement("Material");
    }

    //Get Textures

    Texture texture;
    stream.clear();
    std::cout<<"Textures"<<std::endl;
    element = root->FirstChildElement("Textures");
    if(element){
        element = element->FirstChildElement("Texture");
        while(element){
            
            child = element->FirstChildElement("ImageName");
            stream << child->GetText()<<std::endl;
            stream >> texture.image_name;

            child = element->FirstChildElement("Interpolation");
            stream << child->GetText()<<std::endl;
            stream >> texture.interpolation;

            child = element->FirstChildElement("DecalMode");
            stream << child->GetText()<<std::endl;
            stream >> texture.decalmode;

            child = element->FirstChildElement("Appearance");
            stream << child->GetText()<<std::endl;
            stream >> texture.appearance;
            
            textures.push_back(texture);
            element = element->NextSiblingElement("Texture");
        }
    }
    // Get Translations
    std::cout<<"Translations"<<std::endl;
    element = root->FirstChildElement("Transformations");
    if(element){
        element = element->FirstChildElement("Translation");
        Translation translation;
        while(element){
            
            stream << element->GetText()<<std::endl;
            stream >> translation.transl.x >> translation.transl.y >> translation.transl.z;
            translations.push_back(translation);
            element = element->NextSiblingElement("Translation");
        }
    }

    //Get Rotations
    std::cout<<"Rotations"<<std::endl;
    element = root->FirstChildElement("Transformations");
    if(element){
        element = element->FirstChildElement("Scaling");
        Scaling scaling;
        while(element){
            stream << element->GetText()<<std::endl;
            stream >> scaling.scalar.x >> scaling.scalar.y >> scaling.scalar.z;
            scalars.push_back(scaling);
            element = element->NextSiblingElement("Scaling");
        }
    }
    element = root->FirstChildElement("Transformations");
    if(element){
        element = element->FirstChildElement("Rotation");
        Rotation rotation;
        while(element){
            stream << element->GetText()<<std::endl;
            stream >> rotation.rot.x >> rotation.rot.y >> rotation.rot.z >> rotation.rot.w;
            rotations.push_back(rotation);
            element = element->NextSiblingElement("Rotation");
        }
    }
    //Get VertexData
    element = root->FirstChildElement("VertexData");
    stream << element->GetText() << std::endl;

    Vec3f vertex;
    while (!(stream >> vertex.x).eof())
    {
        stream >> vertex.y >> vertex.z;
        vertex_data.push_back(vertex);
    }
    stream.clear();

    element = root->FirstChildElement("TexCoordData");
    if(element){
        stream << element->GetText()<<std::endl;
        Vec2i coord;
        while(!(stream >> coord.x).eof()){
            stream>>coord.y;
            tex_coord_data.push_back(coord);
        }
    }

    //Get Meshes
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Mesh");
    Mesh mesh;
    stream.clear();
    while (element)
    {
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> mesh.material_id;
        child = element->FirstChildElement("Transformations");
        if(child){
            std::string fake_stream = child->GetText();
            mesh.transformations = split(fake_stream.c_str());
        }
        if(!child)
            mesh.transformations.clear();
        stream.clear();

        child = element->FirstChildElement("Texture");

        if(child){
            stream << child->GetText()<<std::endl;
            stream >> mesh.texture;
        }
        else
            mesh.texture = 0;
        child = element->FirstChildElement("Faces");
        stream << child->GetText() << std::endl;
        Face face;
        while (!(stream >> face.v0_id).eof())
        {
            stream >> face.v1_id >> face.v2_id;
            mesh.faces.push_back(face);
        }
        stream.clear();

        meshes.push_back(mesh);
        mesh.faces.clear();
        element = element->NextSiblingElement("Mesh");
    }
    stream.clear();
    
    // Get MeshInstances
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("MeshInstance");

    MeshInstance mesh_instance;
    while(element){
        stream << element->FindAttribute("baseMeshId")->Value()<<std::endl;
        stream >> mesh_instance.base_mesh_instance; 
        child = element->FirstChildElement("Material");
        stream << child->GetText()<<std::endl;
        stream >> mesh_instance.material_id;
        child = element->FirstChildElement("Texture");
        if(child){
            stream << child->GetText() << std::endl;
            stream >> mesh_instance.texture;
        }
        else
            mesh_instance.texture = 0; // not exists
        child = element->FirstChildElement("Transformations");
        if(child){
            std::string fake_stream = child->GetText();
            mesh_instance.transformations = split(fake_stream.c_str());
        }
        if(!child)
            mesh_instance.transformations.clear();
        stream.clear();

        mesh_instances.push_back(mesh_instance);
        element = element->NextSiblingElement("MeshInstance");
    }
    //Get Triangles
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Triangle");
    Triangle triangle;
    
    while (element)
    {
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> triangle.material_id;

        child = element->FirstChildElement("Transformations");
        if(child){
            std::string fake_stream = child->GetText();
            triangle.transformations = split(fake_stream.c_str());
        }
        if(!child)
            triangle.transformations.clear();
        stream.clear();
        child = element->FirstChildElement("Texture");
        if(child){
            stream << child->GetText()<<std::endl;
            stream >> triangle.texture;
        }
        else
            triangle.texture = 0; // No texture found.
        child = element->FirstChildElement("Indices");
        stream << child->GetText() << std::endl;
        stream >> triangle.indices.v0_id >> triangle.indices.v1_id >> triangle.indices.v2_id;

        triangles.push_back(triangle);
        element = element->NextSiblingElement("Triangle");
    }
    //Get Spheres
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Sphere");
    Sphere sphere;
    int k=1;
    
    while (element)
    {
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> sphere.material_id;
        child = element->FirstChildElement("Transformations");
        if(child){
            std::string fake_stream = child->GetText();
           
            sphere.transformations = split(fake_stream.c_str());
        }
        if(!child)
            sphere.transformations.clear();
        stream.clear();

        child = element->FirstChildElement("Texture");
        if(child){
            stream << child->GetText()<<std::endl;
            stream >> sphere.texture;
        }
        else
            sphere.texture = 0;
        child = element->FirstChildElement("Center");
        stream << child->GetText() << std::endl;
        stream >> sphere.center_vertex_id;

        child = element->FirstChildElement("Radius");
        stream << child->GetText() << std::endl;
        stream >> sphere.radius;

        spheres.push_back(sphere);
        element = element->NextSiblingElement("Sphere");
    }
    

}

std::ostream& operator<<(std::ostream& o,const parser::Vec3f& v){
    o<<"("<<v.x<<","<<v.y<<","<<v.z<<")";
    return o;
}
parser::Vec3f parser::operator+ (const parser::Vec3f& lhs,const parser::Vec3f& rhs){
    parser::Vec3f result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

parser::Vec3f parser::operator- (const parser::Vec3f& lhs,const parser::Vec3f& rhs) {
    parser::Vec3f result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

parser::Vec3f parser::operator* (const parser::Vec3f& lhs,const float& f){
    parser::Vec3f result;

    result.x = lhs.x *f;
    result.y = lhs.y *f;
    result.z = lhs.z *f;
    return result;
}
parser::Vec3f parser::operator* (const parser::Vec3f& lhs,const parser::Vec3f& rhs){
    parser::Vec3f result;
    float lx = IS_ZERO(lhs.x);
    float ly = IS_ZERO(lhs.y);
    float lz = IS_ZERO(lhs.z);
    float rx = IS_ZERO(rhs.x);
    float ry = IS_ZERO(rhs.y);
    float rz = IS_ZERO(rhs.z);

    result.x = (ly * rz) - (lz * ry);
    result.y = (lz * rx) - (lx * rz);
    result.z = (lx * ry) - (ly * rx);
    return result;
}

float parser::operator/ (const parser::Vec3f& lhs,const parser::Vec3f& rhs){
    if(IS_ZERO(lhs.x) && IS_ZERO(rhs.x))
        return lhs.x / rhs.x;
    else if(IS_ZERO(lhs.y) && IS_ZERO(rhs.y))
        return lhs.y / rhs.y;
    else if(IS_ZERO(lhs.z) && IS_ZERO(rhs.z))
        return lhs.z / rhs.z;
}

std::ostream& parser::operator<< (std::ostream& o, const parser::Vec3f& lhs){
    o<<"("<< lhs.x << ","<<lhs.y<<","<<lhs.z<<")";
    return o;
}

std::ostream& parser::operator<< (std::ostream& o, const parser::Vec4f& lhs){
    o<<"("<< lhs.x << ","<<lhs.y<<","<<lhs.z<<","<<lhs.w<<")";
    return o;
}



float parser::dot(const parser::Vec3f & lhs,const parser::Vec3f & rhs)
{
     return lhs.x*rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

void parser::normalize(parser::Vec3f& vec){
    float normal_length = sqrt(pow(vec.x,2) + pow(vec.y,2) + pow(vec.z,2));
    vec = vec * (1.0/normal_length);
}

bool parser::not_zero(float A,float epsilon)
{
    if(A<epsilon && A>-epsilon)
        return false;
    return true;
}

