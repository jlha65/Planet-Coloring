#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <ctime>
#include <math.h>
#include <iostream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <fstream>

using namespace std;

// angle of rotation for the camera direction
float angle = 0.0f;

float rotAngle = 0.5f;
float initAngle = 0.0f;
float PI = 3.14159265;

// actual vector representing the camera's direction
float lx=0.0f,lz=-1.0f,ly=10.0;

// XZ position of the camera
float x=0.0f, z=5.0f;

// the key states. These variables will be zero
//when no key is being presses
float deltaAngle = 0.0f;
float deltaMove = 0;
int xOrigin = -1;

//Flag for pseudo random
bool flippedDirection = false;

//Flag to create planet once
bool planetCreated = false;

double randNum = 0;

//Changeable parameters
float radio;
float divisiones;
float initialHue;
float initialSaturation;
float initialValue;
float initialRatio;
float noiseParameter;

//ofstream myObjFile;


//Functions to convert from RGB to HSV and from HSV to RGB

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

//OpenGL stuff

void changeSize(int w, int h)
{

    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;

    float ratio =  w * 1.0 / h;

    // Use the Projection Matrix
    glMatrixMode(GL_PROJECTION);

    // Reset Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective.
    gluPerspective(45.0f, ratio, 0.1f, 10000.0f);

    // Get Back to the Modelview
    glMatrixMode(GL_MODELVIEW);
}

double toRadians(double angle)
{
    return angle * PI / 180;
}


void drawPlanet()
{

    ofstream myObjFile;

    myObjFile.open("PlanetGenerated.obj");

    float fRadioEsfera = radio;
    float iDivisionesCirculo = divisiones; //Divisiones de la curva
    float iDivisionesFormula = divisiones; //Divisiones en la formula de la cual se estara rotando la figura

    float diffAnguloCirculo = 360.0 /iDivisionesCirculo;
    float diffAnguloFormula    = 360.0 /iDivisionesFormula;

    float ratioHue = initialRatio;

    int iFaces = 1;


    // glColor3f(1,0,0);
    double dPreviousX, dPreviousY, dPreviousZ, dCurrentX, dCurrentY, dCurrentZ;
    double dXTemp;
    double dYTemp;
    double dZTemp;
    double dAngulo = 0;
    //For coloring

    hsv initialColor;
    rgb currentColor;
    rgb previousColor;
    initialColor.h = (int)initialHue % 360;
    initialColor.s = initialSaturation/100.0;
    initialColor.v = initialValue/100.0;

    currentColor = hsv2rgb(initialColor);
    initialColor.h += ratioHue;
    if(initialColor.h > 360)
        initialColor.h = 0;

    glColor3f(currentColor.r, currentColor.g, currentColor.b);

    /*
    Generacion esfera se divide en tres partes:

    1 .- Generacion de tapa superior (hecha de triangulos) de la esfera.
    2 .- Generacion de quads a la mitad de la esfera.
    3.- Generacion de tapa inferior de la esfera.

    */

    //Generacion de tapa superior de la esfera

     glBegin(GL_TRIANGLE_FAN);
     //Punto superior de la esfera

    //  glColor3f(0.5, 0.399, 0.06);
     previousColor = currentColor;
    initialColor.h += ratioHue;
    if(initialColor.h > 360)
        initialColor.h = 0;

    currentColor = hsv2rgb(initialColor);

    glColor3f(previousColor.r, previousColor.g, previousColor.b);
    dXTemp = fRadioEsfera * cos(toRadians(90));
    dYTemp = 0;
    dZTemp = fRadioEsfera * sin(toRadians(90));
     glVertex3f(dXTemp,dYTemp,dZTemp );


    dAngulo = 0;
    dPreviousX = (fRadioEsfera * cos(toRadians(90 - diffAnguloCirculo))) * cos(toRadians(dAngulo));
    dPreviousY = (fRadioEsfera * cos(toRadians(90 - diffAnguloCirculo))) * sin(toRadians(dAngulo));
    dPreviousZ =  fRadioEsfera * sin(toRadians(90 - diffAnguloCirculo));
    glColor3f(currentColor.r, currentColor.g, currentColor.b);
    glVertex3f(dPreviousX, dPreviousY , dPreviousZ);
    dAngulo += diffAnguloFormula;

     for (int i= 1; i <= iDivisionesFormula; i++)
     {
        dCurrentX = (fRadioEsfera * cos(toRadians(90 - diffAnguloCirculo))) * cos(toRadians(dAngulo));
        dCurrentY = (fRadioEsfera * cos(toRadians(90 - diffAnguloCirculo))) * sin(toRadians(dAngulo));
        dCurrentZ = fRadioEsfera * sin(toRadians(90 - diffAnguloCirculo));
        glColor3f(currentColor.r, currentColor.g, currentColor.b);
        glVertex3f(dCurrentX, dCurrentY, dCurrentZ);
        dAngulo += diffAnguloFormula;
        myObjFile <<"v " << dPreviousX << " " << dPreviousY << " " << dPreviousZ << "\n";
        myObjFile <<"v " << dXTemp << " " << dYTemp << " " << dZTemp << "\n";
        myObjFile <<"v " << dCurrentX << " " << dCurrentY << " " << dCurrentZ << "\n";
        dPreviousX = dCurrentX;
        dPreviousY = dCurrentY;
        dPreviousZ = dCurrentZ;
     }


     glEnd();

    //Generacion de parte media de la esfera
    //divisiones - 1 -> cantidad de renglones de quads a genererar en el medio
    //divisiones * 2 -> cantidad de quads en un renglon de quads

    double omega = 90 + 2*diffAnguloCirculo;
    double angulo = 0;

    glBegin(GL_QUADS);
    //angle to radians -> (angle * PI / 180)
    for(int i= 0; i < iDivisionesCirculo/2  -2; i++)
    {
        previousColor = currentColor;

        if (randNum > noiseParameter)
            initialColor.h += ratioHue;
        else
            initialColor.h -= ratioHue;
        if(initialColor.h > 360)
            initialColor.h = 0;
        currentColor = hsv2rgb(initialColor);

        angulo = 0;
        for (int j =  0; j < iDivisionesFormula; j++)
        {
            glColor3f(currentColor.r, currentColor.g, currentColor.b);
            //Esquina superior izquierda quad
            dXTemp =(fRadioEsfera * cos(toRadians(omega)) * cos (toRadians(angulo)));
            dYTemp = (fRadioEsfera * cos(toRadians(omega)))*sin(toRadians(angulo));
            dZTemp =  fRadioEsfera * sin(toRadians(omega));
            glVertex3f (dXTemp,dYTemp,dZTemp);


            myObjFile <<"v " << dXTemp << " " << dYTemp  << " " <<dZTemp << "\n";


            glColor3f(previousColor.r, previousColor.g, previousColor.b);
            //Esquina inferior izquierda quad
            dXTemp = (fRadioEsfera * cos(toRadians(omega - diffAnguloCirculo)) * cos (toRadians(angulo)));
            dYTemp = (fRadioEsfera * cos(toRadians(omega -diffAnguloCirculo)))*sin(toRadians(angulo));
            dZTemp = fRadioEsfera * sin(toRadians(omega - diffAnguloCirculo));
            glVertex3f (dXTemp, dYTemp, dZTemp);
            myObjFile <<"v " << dXTemp << " " << dYTemp  << " " <<dZTemp << "\n";

            glColor3f(previousColor.r, previousColor.g, previousColor.b);
            //Esquina inferior derecha quads
            dXTemp = (fRadioEsfera * cos(toRadians(omega - diffAnguloCirculo)) * cos (toRadians(angulo + diffAnguloFormula)));
            dYTemp = (fRadioEsfera * cos(toRadians(omega -diffAnguloCirculo)))*sin(toRadians(angulo + diffAnguloFormula));
            dZTemp = fRadioEsfera * sin(toRadians(omega - diffAnguloCirculo));

            glVertex3f (dXTemp,dYTemp, dZTemp);
            myObjFile <<"v " << dXTemp << " " << dYTemp  << " " <<dZTemp << "\n";

            glColor3f(currentColor.r, currentColor.g, currentColor.b);
            //Esquina superior derecha quad0
            dXTemp = (fRadioEsfera * cos(toRadians(omega)) * cos (toRadians(angulo+ diffAnguloFormula)));
            dYTemp = (fRadioEsfera * cos(toRadians(omega)))*sin(toRadians(angulo + diffAnguloFormula));
            dZTemp =  fRadioEsfera * sin(toRadians(omega));
            glVertex3f (dXTemp, dYTemp,dZTemp);
            myObjFile <<"v " << dXTemp << " " << dYTemp  << " " <<dZTemp << "\n";

             angulo += diffAnguloFormula;
        }

        omega += diffAnguloCirculo;
    }
    glEnd();

   //  //Generacion de la tapa inferior de la esfera
    glBegin(GL_TRIANGLE_FAN);

    previousColor = currentColor;
    initialColor.h += ratioHue;
    if(initialColor.h > 360)
        initialColor.h = 0;

    currentColor = hsv2rgb(initialColor);


    //Punto inferior de la esfera
     dAngulo = 0;

    dXTemp = fRadioEsfera * cos(toRadians(-90));
    dYTemp = 0;
    dZTemp = fRadioEsfera * sin(toRadians(-90));
    glColor3f(currentColor.r, currentColor.g, currentColor.b);
    glVertex3f(dXTemp, dYTemp, dZTemp );

    dPreviousX = (fRadioEsfera * cos(toRadians(-90 + diffAnguloCirculo))) * cos(toRadians(dAngulo));
    dPreviousY = (fRadioEsfera * cos(toRadians(-90 + diffAnguloCirculo))) * sin(toRadians(dAngulo));
    dPreviousZ = fRadioEsfera * sin(toRadians(-90 + diffAnguloCirculo));
    glColor3f(previousColor.r, previousColor.g, previousColor.b);
    glVertex3f(dPreviousX, dPreviousY, dPreviousZ);
    dAngulo += diffAnguloFormula;

    for (int i= 1; i <= iDivisionesFormula; i++)
     {
        dCurrentX = (fRadioEsfera * cos(toRadians(-90 + diffAnguloCirculo))) * cos(toRadians(dAngulo));
        dCurrentY = (fRadioEsfera * cos(toRadians(-90 + diffAnguloCirculo))) * sin(toRadians(dAngulo));
        dCurrentZ = fRadioEsfera * sin(toRadians(-90 + diffAnguloCirculo));
        glColor3f(previousColor.r, previousColor.g, previousColor.b);
        glVertex3f(dCurrentX, dCurrentY, dCurrentZ);
        dAngulo += diffAnguloFormula;
        myObjFile <<"v " << dPreviousX << " " << dPreviousY << " " << dPreviousZ << "\n";
        myObjFile <<"v " << dXTemp << " " << dYTemp << " " << dZTemp << "\n";
        myObjFile <<"v " << dCurrentX << " " << dCurrentY << " " << dCurrentZ << "\n";
        dPreviousX = dCurrentX;
        dPreviousY = dCurrentY;
        dPreviousZ = dCurrentZ;
     }

    glEnd();

    //Triangulos superiores de la esfera
    for(int i = 0; i < iDivisionesFormula ; i++){
        myObjFile << "f " << iFaces << " " << iFaces + 1 << " " << iFaces + 2 << endl;
        iFaces += 3;
    }

    //Cuadrados del cuerpo de la esfera
    int iCantFacesBodySphere = (iDivisionesCirculo/2 - 2) * iDivisionesFormula;
    for(int i = 0; i < iCantFacesBodySphere; i++){
        myObjFile << "f " << iFaces << " " << iFaces + 1 << " " << iFaces + 2 << " " << iFaces + 3 << endl;
        iFaces += 4;
    }

    //Triangulos Inferiores de la esfera
    for(int i = 0; i < iDivisionesFormula ; i++){
        myObjFile << "f " << iFaces << " " << iFaces + 1 << " " << iFaces + 2 << endl;
        iFaces += 3;
    }
    myObjFile.close();

}

void computePos(float deltaMove)
{

    x += deltaMove * lx * 0.4f;
    z += deltaMove * lz * 0.4f;
}

void renderScene(void)
{

    if (deltaMove)
        computePos(deltaMove);

    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformations
    glLoadIdentity();

    // float y = 12.0f;
    // Set the camera
    gluLookAt(	x, ly, z,
                x+lx, ly,  z+lz,
                0.0f, ly,  0.0f);

    cout << "Edgar \n";
    glPushMatrix();
    glRotated(90, 1, 0, 0);
    glTranslatef(0.0f, 0.0f, - radio * 3);
    drawPlanet();
    glPopMatrix();
    glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int xx, int yy) {
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'q':if(!planetCreated) {
        time_t result = time(nullptr);
        //tm result2;
        localtime(&result);
        planetCreated = true;
        randNum = (result * result) % 100;
    }
        exit(0);
        break;
    case 'a':
        ly += 1;
        break;
    case 'z':
        ly -= 1;
        break;
    default:
        break;
    }
}

void pressKey(int key, int xx, int yy)
{

    switch (key)
    {
    case GLUT_KEY_UP :
        deltaMove = 2.0f;
        break;
    case GLUT_KEY_DOWN :
        deltaMove = -2.0f;
        break;
    }
}

void releaseKey(int key, int x, int y)
{

    switch (key)
    {
    case GLUT_KEY_UP :
    case GLUT_KEY_DOWN :
        deltaMove = 0;
        break;
    }
}

void mouseMove(int x, int y)
{

    // this will only be true when the left button is down
    if (xOrigin >= 0)
    {

        // update deltaAngle
        deltaAngle = (x - xOrigin) * 0.001f;

        // update camera's direction
        lx = sin(angle + deltaAngle);
        lz = -cos(angle + deltaAngle);
    }
}

void mouseButton(int button, int state, int x, int y)
{

    // only start motion if the left button is pressed
    if (button == GLUT_LEFT_BUTTON)
    {

        // when the button is released
        if (state == GLUT_UP)
        {
            angle += deltaAngle;
            xOrigin = -1;
        }
        else   // state = GLUT_DOWN
        {
            xOrigin = x;
        }
    }
}

int main(int argc, char **argv)
{
    cout << "Ingresa el radio del planeta \n";
    cin >> radio;
    cout << "Ingresa la cantidad de divisiones de la esfera \n";
    cin >> divisiones;
    cout << "Ingresa el Hue para HSV inicial (0-359) \n";
    cin >> initialHue;
    cout << "Ingresa la saturacion para HSV inicial (0 - 100) \n";
    cin >> initialSaturation;
    cout << "Ingresa el valor de HSV inicial (0 - 100) \n";
    cin >> initialValue;
    cout << "Ingresa el valor de cambio \n";
    cin >> initialRatio;
    cout << "Ingresa el valor de ruido (de -1 a 1) \n";
    cin >> noiseParameter;

    //Change noise parameter
    noiseParameter = noiseParameter * 50;
    if(noiseParameter < 0) {
        noiseParameter = 100 - noiseParameter;
    }

    z = radio * 3;
    ly = 0.0001 + radio * 3;

    if(!planetCreated) {
        time_t result = time(nullptr);
        //tm result2;
        localtime(&result);
        planetCreated = true;
        randNum = (result * result) % 100;
    }

    // init GLUT and create window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(1366,768);
    glutCreateWindow("Planet");

    // register callbacks
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutIdleFunc(renderScene);

    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(pressKey);
    glutSpecialUpFunc(releaseKey);
    // here are the two new functions
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);

    // OpenGL init
    glEnable(GL_DEPTH_TEST);

    // enter GLUT event processing cycle
    glutMainLoop();

    return 1;
}
