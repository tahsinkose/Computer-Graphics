#include <iostream>
#include <sstream>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <vector>
#define _USE_MATH_DEFINES
#include "RGBpixmap.h"
#include "./glm/glm.hpp" // GL Math library header
#include "./glm/gtc/matrix_transform.hpp"
#include "./glm/gtc/type_ptr.hpp"
#define floor_level  0.023002
#define FLIGHT_MODE_STRIDE 0.1
#define ROTATE_ANGLE 0.3
#define AIRCRAFT_RADIUS 0.5
#define PI 3.14159265
#define BUILDING_TIME 700
#define SPAWN_TIME 200
#define BONUS_TIME 100
using namespace std;


struct Light{
    glm::vec3 position;
    glm::vec3 intensity;
};

struct Camera
{
    glm::vec3 position;
    glm::vec3 gaze;
    glm::vec3 up;
    glm::vec3 left;
    glm::vec4 near_plane;
    float near_distance,far_distance;
    int image_width, image_height;
};

struct Material
{
    glm::vec3 ambient,diffuse,specular;
    float phong_exponent;
};

struct Building
{
	int offset;
	int aux_offset;
	int height;
	vector<pair<glm::vec3,float> > bounding_spheres;
};
struct RandBuilding
{
	int height;
	float x;
	float z;
	int lifetime;
	vector<pair<glm::vec3,float> > bounding_spheres;
};
struct BonusItem{
	int height;
	float x;
	float z;
	int lifetime;
	pair<glm::vec3,float> bounding_sphere;
};
class Scene{
    private:
        Camera camera;
        Light l;
        float speed;
        float horizontal_angle;
        float vertical_angle;
        int score;
        int time_counter;
        vector<Building> current_buildings;
        vector<RandBuilding> rand_buildings;
        vector<BonusItem> bonus_items;
    public:
        Scene(Camera& c,Light& light){
            speed = 0;
            camera = c;
            l = light;
            horizontal_angle = 0;
            vertical_angle = 0;
            score = 0;
            time_counter = 0;
        }
        void draw(vector<Material>& m);
        void drawBuilding(float offset,float aux_offset,float scalar);
        void drawBuildings(Material& m);
        void drawStaticBuildings(Material& m);
        void drawStaticBonuses(Material& m);
        void drawScore(const char* text,int length,int x,int y);
        void drawAircraft(Material& m); 
        void setCamera();
        void addLight(Light& l);
        void turnOnLight();
        void checkChange();
        void setMaterial(Material& m);
        void applyHorizontalRotation(float angle);
        void applyVerticalRotation(float angle);
        void intersectionTest();
        void InitTextures();

};