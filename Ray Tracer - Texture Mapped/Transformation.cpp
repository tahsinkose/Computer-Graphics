#include "Transformation.h"


void ApplyTransformation(parser::Scene& scene,char type,int id,parser::Vec3f& p0,parser::Vec3f& p1,parser::Vec3f& p2){
    if(type=='t'){
        parser::Vec3f transl = scene.translations[id-1].transl;
        p0 = homtransl(transl,p0);p1 = homtransl(transl,p1);p2 = homtransl(transl,p2);                
    }
    else if(type=='r'){
        parser::Vec4f rot = scene.rotations[id-1].rot;
        p0 = homrot(rot,p0);p1 = homrot(rot,p1);p2 = homrot(rot,p2);
    }
    else{
        parser::Vec3f scaling = scene.scalars[id-1].scalar;
        p0 = homscale(scaling,p0); p1 = homscale(scaling,p1);p2 = homscale(scaling,p2);                  
    }
    return;
}

void ApplyTriangularTransformations(parser::Scene& scene,std::map<int,std::vector<parser::Vec3f> >& mesh_normals,
   std::map<int, parser::Vec3f>& triangle_normals,std::map<int,std::vector<parser::Vec3f> >& mesh_instance_normals ){

    std::vector<parser::Vec3f> transformed_points = scene.vertex_data;
    for(int obj = 0; obj < scene.meshes.size();obj++){
        parser::Mesh mesh = scene.meshes[obj];
        for(int face=0;face<mesh.faces.size();face++){
            parser::Face f = mesh.faces[face];

            parser::Vec3f p0 = scene.vertex_data[f.v0_id - 1];
            parser::Vec3f p1 = scene.vertex_data[f.v1_id - 1];
            parser::Vec3f p2 = scene.vertex_data[f.v2_id - 1];
            for(int trs=0;trs<mesh.transformations.size();trs++){

                std::string transformation = mesh.transformations[trs];
                char type;
                int id;    
                std::istringstream ss(transformation);
                ss >> type >> id;
                ApplyTransformation(scene,type,id,p0,p1,p2);
            }
            // Record transformed vertexes.
            transformed_points[f.v0_id - 1] = p0;transformed_points[f.v1_id - 1] = p1;transformed_points[f.v2_id - 1] = p2;
            parser::Vec3f normal = (p1 - p0) * (p2 - p0);
            normalize(normal);
            mesh_normals[obj].push_back(normal);
        }     
    }
    scene.vertex_data = transformed_points;

    
    for(int obj = 0; obj < scene.triangles.size();obj++){
        parser::Triangle tri = scene.triangles[obj];
        parser::Face f = tri.indices;

        parser::Vec3f p0 = scene.vertex_data[f.v0_id - 1];
        parser::Vec3f p1 = scene.vertex_data[f.v1_id - 1];
        parser::Vec3f p2 = scene.vertex_data[f.v2_id - 1];
        for(int trs=0;trs<tri.transformations.size();trs++){
            std::string transformation = tri.transformations[trs];
            char type;
            int id;    
            std::istringstream ss(transformation);
            ss >> type >> id;
            ApplyTransformation(scene,type,id,p0,p1,p2);
            
        }
        // Record transformed vertexes.
        transformed_points[f.v0_id - 1] = p0;transformed_points[f.v1_id - 1] = p1;transformed_points[f.v2_id - 1] = p2;
        parser::Vec3f normal = (p1 - p0) * (p2 - p0);
        normalize(normal);
        triangle_normals[obj] = normal;
    }

    scene.vertex_data = transformed_points;
    
    //---------------------------
    // First accumulate real points in mesh_instances themselves.
    for(int obj = 0; obj < scene.mesh_instances.size();obj++){
        parser::MeshInstance mesh_instance = scene.mesh_instances[obj];
        int base_mesh_id = mesh_instance.base_mesh_instance;
        
        parser::Mesh base_mesh = scene.meshes[base_mesh_id-1];
        for(int face=0;face<base_mesh.faces.size();face++){
            parser::Face f = base_mesh.faces[face];
            parser::Vec3f p0 = scene.vertex_data[f.v0_id - 1];
            parser::Vec3f p1 = scene.vertex_data[f.v1_id - 1];
            parser::Vec3f p2 = scene.vertex_data[f.v2_id - 1];
            parser::RealFace r = {p0,p1,p2};
           
            mesh_instance.faces.push_back(r);
        }
        scene.mesh_instances[obj] = mesh_instance;
    }

    // Now we have the real points in hand for each mesh instance. Base mesh will not be affected. Proceed with the own transformations of mesh instance.
    for(int obj=0;obj<scene.mesh_instances.size();obj++){
        parser::MeshInstance mesh_instance = scene.mesh_instances[obj];
        for(int face=0;face<mesh_instance.faces.size();face++){
            parser::Vec3f p0 = mesh_instance.faces[face].v1;
            parser::Vec3f p1 = mesh_instance.faces[face].v2;
            parser::Vec3f p2 = mesh_instance.faces[face].v3;
            for(int trs=0;trs<mesh_instance.transformations.size();trs++){
                std::string transformation = mesh_instance.transformations[trs];
                char type;
                int id;    
                std::istringstream ss(transformation);
                ss >> type >> id;
                ApplyTransformation(scene,type,id,p0,p1,p2);
            }
            mesh_instance.faces[face] = {p0,p1,p2};
            parser::Vec3f normal = (p1 - p0) * (p2 - p0);
            normalize(normal);
            mesh_instance_normals[obj].push_back(normal);
        }
        scene.mesh_instances[obj] = mesh_instance;
    }
}
