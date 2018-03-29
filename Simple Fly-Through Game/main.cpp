/* --------------------------- PLEASE READ HERE BEFORE COMPILATION --------------------------------
 *  In order to compile, run command: g++ *.cpp -o game -std=c++11 -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lrt -lGLEW -lGL -lGLU -lm -ldl -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lXinerama  -lXext -lXcursor -lXrender -lXfixes  -lXau -lXdmcp
 *  If -lglfw3 cannot be linked, go to the link http://stackoverflow.com/a/17772217 and follow Step 1.
 * ------------------------------------------------------------------------------------------------
*/
#include "Scene.h"
extern GLfloat global_ambient[4];
extern RGBpixmap pix[2];  

static GLFWwindow* win = NULL;
extern vector<bool> keys;

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}



Scene* scene;
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
    //if key is valid
    if(key >=0 && key <1024){
        if(action == GLFW_PRESS)
            keys[key]=true;
        else if(action == GLFW_RELEASE)
            keys[key]= false;
    }
    scene->checkChange(); 
}

int main(int argc,char** argv){
	keys.resize(1024,false);
	global_ambient[0] = 0.05;
	global_ambient[1] = 0.05;
	global_ambient[2] = 0.05;
	global_ambient[3] = 1.0f;
	glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glutInit(&argc, argv);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    //......
    Camera camera;
    camera.position = {25,10,130}; // Camera position. To look from farther, just increase z value.
	camera.gaze = {0,0,-1};
	camera.up = {0,1,0};
	camera.left = glm::cross(camera.up,camera.gaze);
	camera.near_distance = 3.0;
	camera.near_plane = {-1,1,-0.5,0.5};
	camera.far_distance = 300.0;
	camera.image_width= 1500,camera.image_height = 750;
    //..... This block can be taken as an input as well. But this is a nice arrangement.
    //.....
    Material floor_material;
    floor_material.ambient = {1,1,1};
    floor_material.diffuse = {1,1,1}; 
    floor_material.specular = {1,1,1};
	floor_material.phong_exponent = 5;

	Material building_material;
    building_material.ambient = {0.05,0.05,0.05};
    building_material.diffuse = {0.8,0.2,0.1}; 
    building_material.specular = {0.3,0.2,0.2};
	building_material.phong_exponent = 1;

	Material vehicle_material;
    vehicle_material.ambient = {0.05,0.05,0.05};
    vehicle_material.diffuse = {0.2,0.2,1.0}; 
    vehicle_material.specular = {0.3,0.2,1.0};
	vehicle_material.phong_exponent = 1;

	Material bonus_material;
	bonus_material.ambient = {0.05,0.05,0.05};
	bonus_material.diffuse = {0.0,1.0,0.0};
	bonus_material.specular = {0.2,1.0,0.2};
	bonus_material.phong_exponent = 3;
    //..... For simplicity, let all buildings have the same property.
    Light light,light2;
    light.position = {25,20,135};
    light.intensity = {1.0,1.0,1.0};
    light2.position = {-2,15,0};
    light2.intensity = {1.0,1.0,1.0};
    scene = new Scene(camera,light);
    
    win = glfwCreateWindow(camera.image_width, camera.image_height, "FLY-THROUGH GAME", NULL, NULL);
    if (!win) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(win);

    
    glfwSetKeyCallback(win, keyCallback);

    //Enable inputting from keyboard but sticky keys are disabled
    glfwSetInputMode(win, GLFW_STICKY_KEYS, GL_FALSE);

    //Modern approach

    glfwSetKeyCallback(win, keyCallback);
    
    scene->setCamera();
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    scene->turnOnLight();
    scene->addLight(light2);
    scene->InitTextures();
    vector<Material> materials;
    materials.push_back(floor_material);
    materials.push_back(building_material);
    materials.push_back(vehicle_material);
    materials.push_back(bonus_material);

    srand (time(NULL));
   
    while(!glfwWindowShouldClose(win)) {
        glfwWaitEvents();
        scene->draw(materials);
        scene->intersectionTest();
        glfwSwapBuffers(win);
       
    }
    return 0;
}