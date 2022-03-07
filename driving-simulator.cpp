#include "library.h"
#include "Circuit.h"

#define MAXSPEED 20.0
#define COEFF 1
#define PI (2*acos(0.0))

double cameraAngle;
int drawgrid;
int drawaxes;
double car = 0;
double X = 10, Y = 50, Z = 10;
double x_font = 12.0, y_font = 35.0;
float lx=0.0f,lz=-1.0f; // actual vector representing the camera's direction
double leftRightMove = 0;
double sky = -1000;
double speed = 0.0;
time_t oldTime_fps;
time_t oldTime;
int fps = 0;
unsigned int state = 0; // different than 0 for shake camera
double alpha = 1.5; //proportion for position determination
double RATIO = 1.7;
double x_rename = 0.0;
double y_rename = 0.0;
std::vector<std::pair<double,double>> roads; // vector of pairs containing x and y informations on the the roads' points.
std::vector<std::pair<double,double>> middle_roads; //same but for the white line on the road


bool turned = false;
// The different windows
int winMenu, winGuide, winRun;

//point l, u, r;
car_coord coord_car;

ISoundEngine* soundEngine; //for sound
ISoundEngine* soundEngine_effets; //for sound for effets

keys_use keys;

int main(int argc, char **argv){
    glutInit(&argc,argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);

    // Window and callback functions for Menu
    winMenu = glutCreateWindow("Menu");
    glutDisplayFunc(renderMenu);
    glutReshapeFunc(stopReshape);
    glutMouseFunc(mouseMenu);
    //glutHideWindow();

    // Window and callback functions for Guide
    winGuide = glutCreateWindow("Guide");
    glutDisplayFunc(renderGuide);
    glutReshapeFunc(stopReshape);
    glutKeyboardFunc(keyboardGuide);
    glutHideWindow();

    // Main application
    glutInitWindowSize(WINDOW_W_H_RUN, WINDOW_W_H_RUN);
    winRun = glutCreateWindow("car driving test");
    glutDisplayFunc(display);
    glutIdleFunc(animate);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutSpecialUpFunc(specialUpListener);
    //glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutHideWindow();

    init();

    // start the sound engines with default parameters
    soundEngine = createIrrKlangDevice();
    if (!soundEngine){
        printf("Could not startup soundEngine\n");
    }
    soundEngine_effets = createIrrKlangDevice();
    if (!soundEngine_effets){
        printf("Could not startup soundEngine for effects\n");
    }

    glEnable(GL_DEPTH_TEST);
    /*glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_CLAMP);*/

    glutMainLoop();

    /* End of programme */

    if(soundEngine) {
        soundEngine->drop(); // delete soundEngine
    }
    if(soundEngine_effets) {
        soundEngine_effets->drop(); // delete soundEngine effects
    }
    //glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);

    return 0;
}

void display() {
    // get FPS :
    ++fps;
    time_t temp = time(NULL);
    unsigned long diff_second = (unsigned long) difftime(temp, oldTime_fps);
    if(diff_second >= 1){
        oldTime_fps = temp;
        printf("FPS=%i\n", fps);
        fps = 0;
    }

    //clear the display
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.345, 0.4, 0,0);	//color black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /********************
            camera setup
        ********************/

    //load the correct matrix -- MODEL-VIEW matrix
    glMatrixMode(GL_MODELVIEW);

    //initialize the matrix
    glLoadIdentity();


    // animation is case of collision -> shake the camera
    double delta = 0.3;
    switch (state) {
        case 0:
            gluLookAt(0,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            break;
        case 1:
            gluLookAt(delta,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 2;
            break;
        case 2:
            gluLookAt(0,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 3;
            break;
        case 3:
            gluLookAt(-delta,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 4;
            break;
        case 4:
            gluLookAt(0,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 5;
            break;
        case 5:
            gluLookAt(delta,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 6;
            break;
        case 6:
            gluLookAt(0,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 7;
            break;
        case 7:
            gluLookAt(-delta,Y,10,	0+lx,-99999999999, 0+lz,	0,0,1);
            state = 0;
            break;
    }

    glMatrixMode(GL_MODELVIEW);

    car -= speed;
    Y -= speed;
    sky -= speed;
    speed *= 0.98;


    /* ajout des 4 points nécessaires à la construction de la route de bezier */
    // premiere portion
   /* std::array<double, 2> p1, p2, p3, p4;
    p1[0] = 100.0; p1[1] = -2500.0;
    p2[0] = 120.0; p2[1] = -3200.0;
    p3[0] = 250.0; p3[1] = -3800.0;
    p4[0] = 300.0; p4[1] = -4000.0;

    drawRoadBezier(p1, p2, p3, p4);

    // 2e portion
    p1[0] = 300.0; p1[1] = -4000.0;
    p2[0] = 400.0; p2[1] = -4500.0;
    p3[0] = -200.0; p3[1] = -4800.0;
    p4[0] = 500.0; p4[1] = -5000.0;

    drawRoadBezier(p1, p2, p3, p4);*/

    std::array<double, 2> p1, p2, p3, p4, g1, g2, g3, g4;

    //first portion of the road
    /*
    p1[0] = (0.0 + x_rename) * RATIO; p1[1] = (0.0 + y_rename) * RATIO;
    p2[0] = (100.0 + x_rename) * RATIO; p2[1] = (-100.0 + y_rename) * RATIO;
    p3[0] = (200.0 + x_rename) * RATIO; p3[1] = (-100.0 + y_rename) * RATIO;
    p4[0] = (300.0 + x_rename) * RATIO; p4[1] = (0.0 + y_rename) * RATIO;

    g1[0] = (-100.0 + x_rename) * RATIO; g1[1] = (0.0 + y_rename) * RATIO;
    g2[0] = (50.0 + x_rename) * RATIO; g2[1] = (-200.0 + y_rename) * RATIO;
    g3[0] = (250.0 + x_rename) * RATIO; g3[1] = (-200.0 + y_rename) * RATIO;
    g4[0] = (400.0 + x_rename) * RATIO; g4[1] = (0.0 + y_rename) * RATIO;
    */

    //first portion of the road
    p1[0] = (100.0 + x_rename) * RATIO; p1[1] = (0.0 + y_rename) * RATIO;
    p2[0] = (100.0 + x_rename) * RATIO; p2[1] = (-100.0 + y_rename) * RATIO;
    p3[0] = (100.0 + x_rename) * RATIO; p3[1] = (-200.0 + y_rename) * RATIO;
    p4[0] = (100.0 + x_rename) * RATIO; p4[1] = (-300.0 + y_rename) * RATIO;

    g1[0] = (-100.0 + x_rename) * RATIO; g1[1] = (0.0 + y_rename) * RATIO;
    g2[0] = (-100.0 + x_rename) * RATIO; g2[1] = (-100.0 + y_rename) * RATIO;
    g3[0] = (-100.0 + x_rename) * RATIO; g3[1] = (-200.0 + y_rename) * RATIO;
    g4[0] = (-100.0 + x_rename) * RATIO; g4[1] = (-300.0 + y_rename) * RATIO;


    //print both matrices once only
    if (!turned) {

        drawRoadBezier(p1, p2, p3, p4, g1, g2, g3, g4, roads, middle_roads); //initialise the circuit ?

        for (auto it = roads.begin(); it != roads.end(); ++it) {
            std::cout << "\t" << it->first << " \t" << it->second << "\n";
        }
        std::cout << "\n\n";
        for (auto it = middle_roads.begin(); it != middle_roads.end(); ++it) {
            std::cout << "\t" << it->first << " \t" << it->second << "\n";
        }
        turned = true;
    }
    drawPolygonsFromVectors(roads, -30.0, 0.245, 0.245, 0.245);
    drawPolygonsFromVectors(middle_roads, -29.80, 1, 1, 1);

    drawMainCar(leftRightMove, car);
    drawBackground(sky);
    drawHill(sky);
    drawHUD(speed);
    glutSwapBuffers();

}

void animate() {

    /*******************
         key listeners
      ******************/

    if(keys.IS_KEY_UP) {
        speed = acceleration(speed);
        car -= speed;
        Y -= speed;
        X -= speed;
        sky -= speed;
        y_font -= speed;
    }
    if(keys.IS_KEY_DOWN) {
        speed = deceleration(speed);
        car += speed;
        Y += speed;
        X += speed;
        sky += speed;
    }
    if(keys.IS_KEY_LEFT) {
        /*
        if (coord_car.fl.x >= 100) {
            soundEngine_effets->play2D("./irrKlang/media/metal.wav", false);
            if (soundEngine_effets) {
                soundEngine_effets->setSoundVolume(0.1f);
            }
            state = 1;
            speed = 0;
        } else {
            leftRightMove += 0.5;
            //cameraAngle -= 0.03;

            coord_car.fl.x += alpha;
            coord_car.fr.x += alpha;
            coord_car.bl.x += alpha;
            coord_car.br.x += alpha;
        }
         */
        leftRightMove += 3.5;
        x_rename = leftRightMove;
        //cameraAngle -= 0.03;

        coord_car.fl.x += alpha;
        coord_car.fr.x += alpha;
        coord_car.bl.x += alpha;
        coord_car.br.x += alpha;
    }
    if(keys.IS_KEY_RIGHT) {
        /*
        if (coord_car.fr.x <= -100) {
            soundEngine_effets->play2D("./irrKlang/media/metal.wav", false);
            if (soundEngine_effets) {
                soundEngine_effets->setSoundVolume(0.1f);
            }
            state = 1;
            speed = 0;
        } else {
            glRotatef(1, 0.0, 1.0, 0.0);
            glRotatef(1, 0.0, -1.0, 0.0);
            glTranslatef(2.0, 0.0, 0.0);

            leftRightMove -= 0.5;
            //cameraAngle += 0.03;

            coord_car.fl.x -= alpha;
            coord_car.fr.x -= alpha;
            coord_car.bl.x -= alpha;
            coord_car.br.x -= alpha;
        }
        */
        glRotatef(1, 0.0, 1.0, 0.0);
        glRotatef(1, 0.0, -1.0, 0.0);
        glTranslatef(2.0, 0.0, 0.0);

        leftRightMove -= 3.5;
        x_rename = leftRightMove;
        //cameraAngle += 0.03;

        coord_car.fl.x -= alpha;
        coord_car.fr.x -= alpha;
        coord_car.bl.x -= alpha;
        coord_car.br.x -= alpha;
    }



    double ratio = 20/sqrt(2);
    //angle += 0.05;
    glutPostRedisplay();

    //To get time :
    time_t temp = time(NULL);
    unsigned long diff_second = (unsigned long) difftime(temp, oldTime);
    /*if(diff_second >= 1){
        oldTime = temp;

        // Try to get the position
        coord_car.bl.y -= diff_second * speed * ratio;
    }*/

    oldTime = temp;
    // Try to get the position
    coord_car.bl.y -= diff_second * speed * ratio;


    //printf("pos : [%f, %f]\n", coord_car.bl.x, coord_car.bl.y);

}

void init() {
    drawgrid = 0;
    drawaxes = 1;
    //cameraHeight = 150.0;
    cameraAngle = 1.0;
    //angle = 0;

    coord_car.fl.x = 17;
    coord_car.fl.y = -130;
    coord_car.fl.z = -30;

    coord_car.fr.x = -17;
    coord_car.fr.y = -130;
    coord_car.fr.z = -30;

    coord_car.bl.x = 17;
    coord_car.bl.y = -60;
    coord_car.bl.z = -30;

    coord_car.br.x = -17;
    coord_car.br.y = -60;
    coord_car.br.z = -30;


    /*
    u.x = 0;
    u.y = 0;
    u.z = 1;

    r.x = -1/sqrt(2);
    r.y = 1/sqrt(2);
    r.z = 0;

    l.x = -1/sqrt(2);
    l.y = -1/sqrt(2);
    l.z = 0;
    */

    //map_angle = 90;

    oldTime = time(NULL);

    //clear the screen
    glClearColor(0, 0, 0, 0);

    /************************
    / set-up projection here
    ************************/

    //load the PROJECTION matrix
    glMatrixMode(GL_PROJECTION);

    //initialize the matrix
    glLoadIdentity();

    //give PERSPECTIVE parameters
    gluPerspective(80, 1,	1, 30000.0);
}

double acceleration (double speed_x) {
    double ret;
    if (speed_x == MAXSPEED)
        return speed_x;

    if (speed_x < 3) {
        ret = speed_x + 0.5*COEFF;
    } else if (speed_x < 8) {
        ret = speed_x + 0.3*COEFF;
    } else if (speed_x < 13) {
        ret = speed_x + 0.2*COEFF;
    } else if (speed_x < 16) {
        ret = speed_x + 0.1*COEFF;
    } else {
        ret = speed_x + 0.07*COEFF;
        if (speed_x > MAXSPEED)
            ret = 20.0;
    }

    return ret;
}

double deceleration(double speed_x) {
    double ret;
    if (speed_x > 0.08) {
        ret = (speed_x * 0.95);
    } else {
        ret = speed_x - 0.2 * COEFF;
        if (ret < -5.0)
            ret = -5.0;
    }
    return ret;
}

void specialKeyListener(int key, int x,int y) {
    switch(key) {
        case GLUT_KEY_UP:
            keys.IS_KEY_UP = true;
            break;

        case GLUT_KEY_DOWN:
            keys.IS_KEY_DOWN = true;
            break;

        case GLUT_KEY_LEFT:
            keys.IS_KEY_LEFT = true;
            break;

        case GLUT_KEY_RIGHT:
            keys.IS_KEY_RIGHT = true;
            break;

        //UP AND DOWN -> mouvement de la caméra
        case GLUT_KEY_PAGE_UP :

            cameraAngle += 0.03;
            lx = sin(cameraAngle);
            lz = -cos(cameraAngle);
            break;

        case GLUT_KEY_PAGE_DOWN :
            cameraAngle -= 0.03;
            lx = sin(cameraAngle);
            lz = -cos(cameraAngle);
            break;
    }
}

void specialUpListener(int key, int x, int y){
    switch(key){
        case GLUT_KEY_UP :
            keys.IS_KEY_UP = false;
            break;

        case GLUT_KEY_DOWN :
            keys.IS_KEY_DOWN = false;
            break;

        case GLUT_KEY_LEFT :
            keys.IS_KEY_LEFT = false;
            break;

        case GLUT_KEY_RIGHT :
            keys.IS_KEY_RIGHT = false;
            break;
    }
}

void keyboardListener(unsigned char Key, int x, int y){
    switch(Key){
        case 27: // Escape key
            glutHideWindow();
            glutSetWindow(winMenu);
            glutShowWindow();
            soundEngine->stopAllSounds();
            break;
    }
}
