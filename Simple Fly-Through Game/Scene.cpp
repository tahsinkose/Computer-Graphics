#include "Scene.h"
GLfloat global_ambient[4];
RGBpixmap pix[2];
vector<bool> keys;
GLuint indices[] = {
    0, 1, 2, // front
    3, 0, 2, // front
    4, 7, 6, // back
    5, 4, 6, // back
    0, 3, 4, // left
    3, 7, 4, // left
    2, 1, 5, // right
    6, 2, 5, // right
    3, 2, 7, // top
    2, 6, 7, // top
    0, 4, 1, // bottom
    4, 5, 1,  // bottom
};
void Scene::InitTextures(void)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    // Image Path
    std::string iPath = "./";
    std::string img=iPath+"rock.bmp";

    std::string img2=iPath+"brick.bmp";
    
    pix[0].readBMPFile(img);
    pix[0].setTexture(2001);
    pix[1].readBMPFile(img2);
    pix[1].setTexture(2002);
}

void Scene::applyHorizontalRotation(float angle){
    glm::mat4 R(1); // identity matrix.
    R = glm::rotate(R,angle,camera.up);
    camera.gaze = glm::vec3(R*glm::vec4(camera.gaze,1.0));
    camera.left = glm::normalize(glm::cross(camera.up,camera.gaze));   
}
void Scene::applyVerticalRotation(float angle){
    glm::mat4 R(1); // identity matrix.
    R = glm::rotate(R,angle,camera.left);
    camera.gaze = glm::vec3(R*glm::vec4(camera.gaze,1.0));
    camera.up = glm::normalize(glm::cross(camera.gaze,camera.left));   
}
void Scene::checkChange(){
    if(keys[GLFW_KEY_U]){//increase speed
        keys[GLFW_KEY_U]= false;
        speed += FLIGHT_MODE_STRIDE;
    }
    else if(keys[GLFW_KEY_J]){
        keys[GLFW_KEY_J]= false;
        speed -= FLIGHT_MODE_STRIDE;
    }
    else if(keys[GLFW_KEY_LEFT]){
        keys[GLFW_KEY_LEFT]= false;
        float steer_angle = ROTATE_ANGLE*PI/180;
        applyHorizontalRotation(steer_angle);
        if(camera.gaze.z<0)
        	horizontal_angle += ROTATE_ANGLE;
    	else
    		horizontal_angle -= ROTATE_ANGLE;
    }
    else if(keys[GLFW_KEY_RIGHT]){
        keys[GLFW_KEY_RIGHT]= false;
        float steer_angle = ROTATE_ANGLE*PI/180;
        applyHorizontalRotation(-steer_angle);
        if(camera.gaze.z<0)
        	horizontal_angle -= ROTATE_ANGLE;
    	else
    		horizontal_angle += ROTATE_ANGLE;
    }
    else if(keys[GLFW_KEY_UP]){
        keys[GLFW_KEY_UP]= false;
        float steer_angle = ROTATE_ANGLE*PI/180;
        applyVerticalRotation(steer_angle);
        if(camera.gaze.z<0)
        	vertical_angle += ROTATE_ANGLE;
    	else
    		vertical_angle -= ROTATE_ANGLE;
    }
    else if(keys[GLFW_KEY_DOWN]){
        keys[GLFW_KEY_DOWN]= false;
        float steer_angle = ROTATE_ANGLE*PI/180;
        applyVerticalRotation(-steer_angle);
        if(camera.gaze.z<0)
        	vertical_angle -= ROTATE_ANGLE;
    	else
    		vertical_angle += ROTATE_ANGLE;
    }
}


void Scene::setCamera(){
    glViewport(0,0,camera.image_width,camera.image_height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.gaze = glm::normalize(camera.gaze);
    camera.up = glm::normalize(camera.up);
    camera.position = camera.position + (camera.gaze * speed);
    glm::vec3 m = camera.position + (camera.gaze * camera.near_distance);
    gluLookAt(camera.position.x,camera.position.y,camera.position.z,
              m.x,m.y,m.z,
              camera.up.x,camera.up.y,camera.up.z);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(camera.near_plane.x,camera.near_plane.y,camera.near_plane.z,camera.near_plane.w,camera.near_distance,camera.far_distance);
}

void Scene::setMaterial(Material& m){
    GLfloat ambient_reflectance[] = {m.ambient.x,m.ambient.y,m.ambient.z,1};
    GLfloat diffuse_reflectance[] = {m.diffuse.x,m.diffuse.y,m.diffuse.z,1};
    GLfloat specular_reflectance[] = {m.specular.x,m.specular.y,m.specular.z,1};
    GLfloat phong_exponent[] = {m.phong_exponent};


    glMaterialfv(GL_FRONT,GL_AMBIENT,ambient_reflectance);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse_reflectance);
    glMaterialfv(GL_FRONT,GL_SPECULAR,specular_reflectance);
    glMaterialfv(GL_FRONT,GL_SHININESS,phong_exponent);
}

void Scene::turnOnLight(){
    glEnable(GL_LIGHTING);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,global_ambient);
    glShadeModel(GL_SMOOTH);
    GLfloat local_ambient[] = {0.0,0.0,0.0,1.0};

    GLfloat color[] = {l.intensity.x,l.intensity.y,l.intensity.z,1.0f};
    GLfloat position[] = {l.position.x,l.position.y,l.position.z,1.0f};

    glLightfv(GL_LIGHT0,GL_POSITION,position);
    glLightfv(GL_LIGHT0,GL_AMBIENT,global_ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,color);
    glLightfv(GL_LIGHT0,GL_SPECULAR,color);
    glEnable(GL_LIGHT0);
}

void Scene::addLight(Light& l){
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,global_ambient);
    glShadeModel(GL_SMOOTH);
    GLfloat local_ambient[] = {0.0,0.0,0.0,1.0};

    GLfloat color[] = {l.intensity.x,l.intensity.y,l.intensity.z,1.0f};
    GLfloat position[] = {l.position.x,l.position.y,l.position.z,1.0f};

    glLightfv(GL_LIGHT1,GL_POSITION,position);
    glLightfv(GL_LIGHT1,GL_AMBIENT,global_ambient);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,color);
    glLightfv(GL_LIGHT1,GL_SPECULAR,color);
    glEnable(GL_LIGHT0+1);
}

void Scene::draw(vector<Material>& m)
{
    setCamera();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
   
    static bool firstTime = true;

    if (firstTime)
        glEnableClientState(GL_VERTEX_ARRAY);
    
    float stretch_factor = 10.0;
    glBindTexture(GL_TEXTURE_2D,2001);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_QUADS);
    setMaterial(m[0]);
    glTexCoord2f(-16.0, -16.0); glVertex3f(-stretch_factor*(float)17.69,floor_level,-stretch_factor*(float)18.47);
    glTexCoord2f(-16.0, 32.0); glVertex3f(-stretch_factor*(float)17.69,floor_level,stretch_factor*(float)16.94);
    glTexCoord2f(32.0, 32.0); glVertex3f( stretch_factor*(float)17.71,floor_level,stretch_factor*(float)16.94);
    glTexCoord2f(32.0, -16.0); glVertex3f(stretch_factor*(float)17.71,floor_level,-stretch_factor*(float)18.47);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,2000);
    
    
    if(time_counter==0)
	    drawBuildings(m[1]);
    glBindTexture(GL_TEXTURE_2D,2000);
	if(time_counter%BONUS_TIME==0){
		int level = rand() % 4;
		int height = rand() % 8 + 6;
		float unit = 0.25;
		int zlevel_offset = 6-level;
		int twist = rand() % 2;
		int xlevel_offset = 6-level;
		if(twist)
			xlevel_offset *= -1;
		GLfloat vertexPos[] = {
            -unit+camera.position.x+(xlevel_offset), (float)height,  unit+camera.position.z-10-(zlevel_offset), // 0: bottom-left-front
             unit+camera.position.x+(xlevel_offset), (float)height,  unit+camera.position.z-10-(zlevel_offset), // 1: bottom-right-front
             unit+camera.position.x+(xlevel_offset),  height+2*unit,  unit+camera.position.z-10-(zlevel_offset), // 2: top-right-front
            -unit+camera.position.x+(xlevel_offset),  height+2*unit,  unit+camera.position.z-10-(zlevel_offset), // 3: top-left-front
            -unit+camera.position.x+(xlevel_offset), (float)height, -unit+camera.position.z-10-(zlevel_offset), // 4: bottom-left-back
             unit+camera.position.x+(xlevel_offset), (float)height, -unit+camera.position.z-10-(zlevel_offset), // 5: bottom-right-back
             unit+camera.position.x+(xlevel_offset),  height+2*unit, -unit+camera.position.z-10-(zlevel_offset), // 6: top-right-back
            -unit+camera.position.x+(xlevel_offset),  height+2*unit, -unit+camera.position.z-10-(zlevel_offset), // 7: top-left-back
        
        };
        glVertexPointer(3, GL_FLOAT, 0, vertexPos);
        setMaterial(m[3]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
        int lifetime = 5000;
        float cx =  camera.position.x+(xlevel_offset);
        float cz =  camera.position.z-10-(zlevel_offset);
        glm::vec3 center = {cx,height+unit,cz};
        float radius = unit*2;
        pair<glm::vec3,float> sphere(center,radius);
        BonusItem b = {height,camera.position.x+xlevel_offset,camera.position.z-10-zlevel_offset,lifetime,sphere};
        bonus_items.push_back(b);
	}
	time_counter++;
	if(time_counter%SPAWN_TIME==0){
		
		int level = rand() % 4;
		int scalar = rand() % 8 + 6;
		float unit = 1.0;
		int twist = rand() % 2;
		int level_offset = 6-level;
		if(twist)
			level_offset *= -1;
		GLfloat vertexPos[] = {
            -unit+camera.position.x+(level_offset), -floor_level,  unit+camera.position.z-10-(level_offset), // 0: bottom-left-front
             unit+camera.position.x+(level_offset), -floor_level,  unit+camera.position.z-10-(level_offset), // 1: bottom-right-front
             unit+camera.position.x+(level_offset),  scalar*unit,  unit+camera.position.z-10-(level_offset), // 2: top-right-front
            -unit+camera.position.x+(level_offset),  scalar*unit,  unit+camera.position.z-10-(level_offset), // 3: top-left-front
            -unit+camera.position.x+(level_offset), -floor_level, -unit+camera.position.z-10-(level_offset), // 4: bottom-left-back
             unit+camera.position.x+(level_offset), -floor_level, -unit+camera.position.z-10-(level_offset), // 5: bottom-right-back
             unit+camera.position.x+(level_offset),  scalar*unit, -unit+camera.position.z-10-(level_offset), // 6: top-right-back
            -unit+camera.position.x+(level_offset),  scalar*unit, -unit+camera.position.z-10-(level_offset), // 7: top-left-back
        
        };
        glVertexPointer(3, GL_FLOAT, 0, vertexPos);
        setMaterial(m[1]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
        int lifetime = 500;
        float cx =  camera.position.x+(level_offset);
        float cz =  camera.position.z-10-(level_offset);
        vector<pair<glm::vec3,float> > spheres;
        for(int i=1;i<=scalar;i++){
        	glm::vec3 center = {cx,i-0.5,cz};
        	float radius = 0.5;
        	spheres.push_back({center,radius});
        }
        RandBuilding b = {scalar,camera.position.x+level_offset,camera.position.z-10-level_offset,lifetime,spheres};
        rand_buildings.push_back(b);

	}
	drawStaticBuildings(m[1]);
	glBindTexture(GL_TEXTURE_2D,2000);
	drawStaticBonuses(m[3]);
	if(time_counter>BUILDING_TIME)
		time_counter = 0;

    drawAircraft(m[2]);
    string text = "SCORE: ";
    stringstream ss;
    ss << score;
    text.append(ss.str());
    drawScore(text.data(),text.size(),0,0);

    firstTime = false;

}
void Scene::drawBuilding(float offset,float aux_offset,float scalar){
    float unit = 1.0;
    glBindTexture(GL_TEXTURE_2D,2002);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_QUADS); // TOP FACE
    
    glTexCoord2f(-4.0, -4.0); glVertex3f(-unit+offset+aux_offset,scalar*unit,-unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(-unit+offset+aux_offset,scalar*unit,unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(unit+offset+aux_offset,  scalar*unit,  unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(unit+offset+aux_offset,  scalar*unit, -unit+offset-aux_offset);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(-4.0, -4.0); glVertex3f(unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(unit+offset+aux_offset, -floor_level, -unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(unit+offset+aux_offset,  scalar*unit, -unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(unit+offset+aux_offset,  scalar*unit,  unit+offset-aux_offset);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(-4.0, -4.0); glVertex3f(-unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(unit+offset+aux_offset,  scalar*unit,  unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(-unit+offset+aux_offset,  scalar*unit,  unit+offset-aux_offset);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(-4.0, -4.0); glVertex3f(-unit+offset+aux_offset, -floor_level, -unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(unit+offset+aux_offset, -floor_level,  -unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(-unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(-4.0, -4.0); glVertex3f(-unit+offset+aux_offset, -floor_level, -unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(-unit+offset+aux_offset, -floor_level,  unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(-unit+offset+aux_offset,  scalar*unit,  unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(-unit+offset+aux_offset,scalar*unit,-unit+offset-aux_offset);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(-4.0, -4.0); glVertex3f(-unit+offset+aux_offset, -floor_level, -unit+offset-aux_offset);
    glTexCoord2f(-4.0, 8.0); glVertex3f(-unit+offset+aux_offset,scalar*unit,-unit+offset-aux_offset);
    glTexCoord2f(8.0, 8.0); glVertex3f(unit+offset+aux_offset,  scalar*unit,  -unit+offset-aux_offset);
    glTexCoord2f(8.0, -4.0); glVertex3f(unit+offset+aux_offset, -floor_level,  -unit+offset-aux_offset);
    glEnd();
}
void Scene::drawBuildings(Material& m){
    
    int num_of_buildings = rand() % 10 + 5;
    current_buildings.clear();
    setMaterial(m);
    for(int i=0;i<num_of_buildings;i++){
    	int big_offset = rand() % 100 + 10;
    	int little_offset = rand() % big_offset;
    	int offset = rand() % big_offset - little_offset;
    	int aux_offset = rand() % 10;
    	int scalar = rand() % 8 + 6;
    	drawBuilding(offset,aux_offset,scalar);
        float cx =  offset+aux_offset;
        float cz =  offset-aux_offset;
        vector<pair<glm::vec3,float> > spheres;
        for(int i=1;i<=scalar;i++){
        	glm::vec3 center = {cx,i-0.5,cz};
        	float radius = 0.5;
        	spheres.push_back({center,radius});
        }
        Building b = {offset,aux_offset,scalar,spheres};
        current_buildings.push_back(b);
    }
}

void Scene::drawStaticBuildings(Material& m){
	float unit = 1.0;
    setMaterial(m);
	for(int i=0;i<current_buildings.size();i++){
		int offset = current_buildings[i].offset;
		int aux_offset = current_buildings[i].aux_offset;
		int scalar = current_buildings[i].height;
		glBindTexture(GL_TEXTURE_2D,2002);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        drawBuilding(offset,aux_offset,scalar);
	}
	for(int i=0;i<rand_buildings.size();i++){
		rand_buildings[i].lifetime--;
		if(!rand_buildings[i].lifetime){
			rand_buildings.erase(rand_buildings.begin()+i);
			continue;
		}
		int scalar= rand_buildings[i].height;
		int x = rand_buildings[i].x;
		int z = rand_buildings[i].z;
		drawBuilding(x+z,x-z,scalar);
	}
}

void Scene::drawStaticBonuses(Material& m){
	float unit = 0.25;
	for(int i=0;i<bonus_items.size();i++){
		bonus_items[i].lifetime--;
		if(!bonus_items[i].lifetime){
			bonus_items.erase(bonus_items.begin()+i);
			continue;
		}
		int height = bonus_items[i].height;
		int x = bonus_items[i].x;
		int z = bonus_items[i].z;
		GLfloat vertexPos[] = {
            -unit+x, (float)height,  unit+z, // 0: bottom-left-front
             unit+x, (float)height,  unit+z, // 1: bottom-right-front
             unit+x,  height+2*unit,  unit+z, // 2: top-right-front
            -unit+x,  height+2*unit,  unit+z, // 3: top-left-front
            -unit+x, (float)height, -unit+z, // 4: bottom-left-back
             unit+x, (float)height, -unit+z, // 5: bottom-right-back
             unit+x,  height+2*unit, -unit+z, // 6: top-right-back
            -unit+x,  height+2*unit, -unit+z, // 7: top-left-back
        
        };
        glVertexPointer(3, GL_FLOAT, 0, vertexPos);
        setMaterial(m);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
	}
}
void Scene::drawScore(const char* text,int length,int x,int y){
	glMatrixMode(GL_PROJECTION);
	double *matrix = new double[16];
	glGetDoublev(GL_PROJECTION_MATRIX,matrix);
	glLoadIdentity();
	
	glLoadIdentity();
	gluOrtho2D(0, 1500, 750, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	glRasterPos3f(25,30,0);

	for(int i=0;i<length;i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,(int)text[i]);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);
}

void Scene::drawAircraft(Material& m){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
        GLUquadric *shape = gluNewQuadric();
        gluQuadricDrawStyle(shape, GLU_FILL);
        gluQuadricNormals(shape, GLU_SMOOTH);
        setMaterial(m);
        glTranslatef(camera.position.x,camera.position.y,camera.position.z);
        
        if(camera.gaze.z<0){
        	glRotatef(horizontal_angle,camera.up.x,camera.up.y,camera.up.z);
        	glRotatef(vertical_angle,camera.left.x,camera.left.y,camera.left.z);
        	glTranslatef(0,0,-10);
        }
        else{
        	glRotatef(-horizontal_angle,camera.up.x,camera.up.y,camera.up.z);
        	glRotatef(-vertical_angle,camera.left.x,camera.left.y,camera.left.z);
        	glTranslatef(0,0,10);
        }
        GLfloat matrix[16]; 
		glGetFloatv (GL_MODELVIEW_MATRIX, matrix);
		glTranslatef(-matrix[12],-matrix[13],0);
        gluSphere(shape,AIRCRAFT_RADIUS,36 ,18);
        gluDeleteQuadric(shape);
    glPopMatrix();
}


void Scene::intersectionTest(){
	// There are 3 type of collideable objects. Buildings, Random Buildings and Bonus Points.
	// Buildings and Random Buildings require perfect mirror bouncing. Bonus Points are only eaten. So start with Bonus Points.
	glm::vec3 aircraft_position;
	
	aircraft_position = {camera.position.x + camera.gaze.x*10,camera.position.y + camera.gaze.y*10,camera.position.z + camera.gaze.z*10};
	for(int i=0;i<bonus_items.size();i++){
		glm::vec3 c = bonus_items[i].bounding_sphere.first;
		float item_radius = bonus_items[i].bounding_sphere.second;
		float distance = glm::length(aircraft_position - c);
		if(distance < item_radius + AIRCRAFT_RADIUS){
			bonus_items.erase(bonus_items.begin()+i);
			
			score += 20;
			return;
		}
	}

	for(int i=0;i<rand_buildings.size();i++){
		for(int j=0;j<rand_buildings[i].bounding_spheres.size();j++){
			glm::vec3 c = rand_buildings[i].bounding_spheres[j].first;
			float item_radius = rand_buildings[i].bounding_spheres[j].second;
			float distance = glm::length(aircraft_position - c);
			if(distance < item_radius + AIRCRAFT_RADIUS){
				//glm::vec3 dir = glm::normalize(aircraft_position - c);
				//glm::vec3 intersection_point = c + (dir * item_radius);
				glm::vec3 surface_normal = {0,0,1};
				camera.gaze = -2*(glm::dot(camera.gaze,surface_normal))*surface_normal + camera.gaze;
				
				camera.left = glm::cross(camera.up,camera.gaze);
				// Introduce mirror bouncing in here
				score -= 10;
				return;
			}
		}
	}
	for(int i=0;i<current_buildings.size();i++){
		for(int j=0;j<current_buildings[i].bounding_spheres.size();j++){
			glm::vec3 c = current_buildings[i].bounding_spheres[j].first;
			float item_radius = current_buildings[i].bounding_spheres[j].second;
			float distance = glm::length(aircraft_position - c);
			if(distance < item_radius + AIRCRAFT_RADIUS){
				glm::vec3 surface_normal = {0,0,1};
				camera.gaze = -2*(glm::dot(camera.gaze,surface_normal))*surface_normal + camera.gaze;
				
				camera.left = glm::cross(camera.up,camera.gaze);
				score -= 10;
				return;
			}
		}
	}
}