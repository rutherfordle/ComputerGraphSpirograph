/*
 * File: main.cxx
 * Description: A simple OpenGL program utilizing vertex and fragment shaders.
 *
 *  Created on: Apr 2, 2012
 *      Author: Nathaniel Cesario
 */

#include <GL/glew.h> //must include this before gl.h
#include <GL/freeglut.h>
#include <GL/gl.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <cmath>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

/**
 * Simple class for keeping track of shader program and vertex attribute
 * locations.
 */
class Shader {
public:
    Shader(string vertFile, string fragFile) { fromFiles(vertFile, fragFile); }

    /**
     * Creates a shader program based on vertex and fragment source.
     *
     * @param vertFile Path to vertex source
     * @param fragFile Path to fragment source
     */
    void fromFiles(string vertFile, string fragFile) {
        //These are shader objects containing the shader source code
        GLint vSource = setShaderSource(vertFile, GL_VERTEX_SHADER);
        GLint fSource = setShaderSource(fragFile, GL_FRAGMENT_SHADER);

        //Create a new shader program
        program = glCreateProgram();

        //Compile the source code for each shader and attach it to the program.
        glCompileShader(vSource);
        printLog("vertex compile log: ", vSource);
        glAttachShader(program, vSource);

        glCompileShader(fSource);
        printLog("fragment compile log: ", fSource);
        glAttachShader(program, fSource);

        //we could attach more shaders, such as a geometry or tessellation
        //shader here.

        //link all of the attached shader objects
        glLinkProgram(program);
    }

    /**
     * Helper method for reading in the source for a shader and creating a
     * shader object.
     *
     * @param file Filename of shader source
     * @param type Type of shader-> Only GL_VERTEX_SHADER and GL_FRAGMENT_SHADER
     *   are supported here.
     */
    GLint setShaderSource(string file, GLenum type) {
        //read source code
        ifstream fin(file.c_str());
        if (fin.fail()) {
            cerr << "Could not open " << file << " for reading" << endl;
            return -1;
        }
        fin.seekg(0, ios::end);
        int count  = fin.tellg();
        char *data = NULL;
        if (count > 0) {
            fin.seekg(ios::beg);
            data = new char[count+1];
            fin.read(data,count);
            data[count] = '\0';
        }
        fin.close();

        //create the shader
        GLint s = glCreateShader(type);
        glShaderSource(s, 1, const_cast<const char **>(&data), NULL);
        delete [] data;
        return s;
    }

    /**
     * Helper function used for debugging.
     */
    void printLog(std::string label, GLint obj) {
        int infologLength = 0;
        int maxLength;

        if(glIsShader(obj)) {
            glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
        } else {
            glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
        }

        char infoLog[maxLength];

        if (glIsShader(obj)) {
            glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
        } else {
            glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
        }

        if (infologLength > 0) {
            cerr << label << infoLog << endl;
        }
    }

    GLint program; //shader program
    GLint modelViewLoc; //location of the modelview matrix in the program (M)
    GLint projectionLoc; //location of the projection matrix in the program (P)
    GLint normalMatrixLoc; //location of the normal matrix in the program (M_n)
    GLint vertexLoc, normalLoc; //vertex attribute locations (pos and norm)
      //respectively
    GLint colorLoc;
    GLint timeLoc; //location of time variable
    GLint lightPosLoc;
    GLint viewPosLoc;
    GLuint vertexBuffer, normalBuffer; //used to keep track of GL buffer objects
    GLuint colorBuffer;
};
Shader *shader = NULL;

int WIN_WIDTH = 1280, WIN_HEIGHT = 720; //window width/height
glm::mat4 modelView, projection, camera; //matrices for shaders
glm::vec3 lightPos(1,1,1), viewPos(0,0,6);
float animTime = 0.0f, deltaT = 0.0001; //variables for animation

vector<float> verts; //vertex array
vector<float> norms; //normal array
vector<float> colors; //normal array
size_t numVerts; //number of total vertices

//updates values based on some change in time
void update(float dt) {
    animTime += dt;

    modelView = glm::rotate(modelView, glm::float_t(0.01), glm::vec3(1,1,0));
}

//reshape function for GLUT
void reshape(int w, int h) {
    WIN_WIDTH = w;
    WIN_HEIGHT = h;
    projection = glm::perspective(
            glm::float_t(45),
            glm::float_t(WIN_WIDTH) / glm::float_t(WIN_HEIGHT),
            glm::float_t(0.1),
            glm::float_t(1000.0)
    );
}

//display function for GLUT
void display() {
    glViewport(0,0,WIN_WIDTH,WIN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Setup the modelview matrix
//    Mat4x4F modelCam = camera * modelView;
    glm::mat4 modelCam = camera * modelView;

    //grab the normal matrix from the modelview matrix (upper 3x3 entries of
    //modelview).
    glm::mat3 normalMatrix(modelCam);
    normalMatrix = glm::inverse(normalMatrix);
    normalMatrix = glm::transpose(normalMatrix);

    //Tell OpenGL which shader program we want to use. In this case, we are only
    //using one, but in general we might have many shader programs.
    glUseProgram(shader->program);

    //Pass the matrices and animation time to GPU
    glUniformMatrix4fv(
            shader->modelViewLoc, //handle to variable in the shader program
            1, //how many matrices we want to send
            GL_FALSE, //transpose the matrix
            glm::value_ptr(modelCam) //a pointer to an array containing the entries for
              //the matrix
            );
    glUniformMatrix4fv(shader->projectionLoc, 1, GL_FALSE,
            glm::value_ptr(projection));
    glUniformMatrix3fv(shader->normalMatrixLoc, 1, GL_FALSE,
            glm::value_ptr(normalMatrix));
    glUniform3fv(shader->lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(shader->viewPosLoc, 1, glm::value_ptr(viewPos));
    glUniform1f(shader->timeLoc, animTime);


    glBindBuffer(GL_ARRAY_BUFFER, shader->vertexBuffer); //which buffer we want
      //to use
    glEnableVertexAttribArray(shader->vertexLoc); //enable the attribute
    glVertexAttribPointer(
            shader->vertexLoc, //handle to variable in shader program
            3, //vector size (e.g. for texture coordinates this could be 2).
            GL_FLOAT, //what type of data is (e.g. GL_FLOAT, GL_INT, etc.)
            GL_FALSE, //normalize the data?
            0, //stride of data (e.g. offset in bytes). Most of the time leaving
              //this at 0 (assumes data is in one, contiguous array) is fine
              //unless we're doing something really complex.
            NULL //since our stride will be 0 in general, leaving this NULL is
              //also fine in general
            );

    //same procedure for the normal and color arrays
    glBindBuffer(GL_ARRAY_BUFFER, shader->normalBuffer);
    glEnableVertexAttribArray(shader->normalLoc);
    glVertexAttribPointer(shader->normalLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    //same procedure for the normal and color arrays
    glBindBuffer(GL_ARRAY_BUFFER, shader->colorBuffer);
    glEnableVertexAttribArray(shader->colorLoc);
    glVertexAttribPointer(shader->colorLoc, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    //draw the vertices/normals we just specified.
    glDrawArrays(GL_TRIANGLES, 0, numVerts);

    //update animation variables.
    //have time oscillate between 0.0 and 1.0.
    if ((animTime >= 1.0 && deltaT > 0.0) ||
            (animTime <= 0.0 && deltaT < 0.0)) {
        deltaT = -deltaT;
    }
    update(deltaT);

    glutSwapBuffers();
}

//idle function for GLUT
void idle() {
    glutPostRedisplay();
}

//captures keyborad input for GLUT
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    }
}

//do some GLUT initialization
void setupGLUT() {
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Simple GL Test");

    glutReshapeFunc(reshape);

    glutDisplayFunc(display);

    glutKeyboardFunc(keyboard);

    glutIdleFunc(idle);
}

//initialize OpenGL background color and vertex/normal arrays
void setupGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //initiallize vertex and normal arrays
    //this is where you might want to read in your model
    verts.push_back(-1);
    verts.push_back(-1);
    verts.push_back(0);
    norms.push_back(0);norms.push_back(0);norms.push_back(1);
    colors.push_back(1);colors.push_back(0);colors.push_back(0);colors.push_back(1);

    verts.push_back(1);
    verts.push_back(-1);
    verts.push_back(0);
    norms.push_back(0);norms.push_back(0);norms.push_back(1);
    colors.push_back(0);colors.push_back(1);colors.push_back(0);colors.push_back(1);

    verts.push_back(0);
    verts.push_back(1);
    verts.push_back(0);
    norms.push_back(0);norms.push_back(0);norms.push_back(1);
    colors.push_back(0);colors.push_back(0);colors.push_back(1);colors.push_back(1);

    numVerts = verts.size() / 3;

    camera = glm::lookAt(glm::vec3(0,0,6), glm::vec3(0,0,0), glm::vec3(0,1,0));

    projection = glm::perspective(
            glm::float_t(45),
            glm::float_t(WIN_WIDTH) / glm::float_t(WIN_HEIGHT),
            glm::float_t(0.1),
            glm::float_t(1000.0)
    );
}

//setup the shader program
void setupShaders() {
    //create the shader program from a vertex and fragment shader
    shader = new Shader("shaders/light.vert", "shaders/light.frag");

    //Here's where we setup handles to each variable that is used in the shader
    //program. See the shader source code for more detail on what the difference
    //is between uniform and vertex attribute variables.
    shader->modelViewLoc = glGetUniformLocation(shader->program, "M");
    shader->projectionLoc = glGetUniformLocation(shader->program, "P");
    shader->normalMatrixLoc = glGetUniformLocation(shader->program, "M_n");
    shader->timeLoc = glGetUniformLocation(shader->program, "time");
    shader->lightPosLoc = glGetUniformLocation(shader->program, "L_p");
    shader->viewPosLoc = glGetUniformLocation(shader->program, "E");

    //notice that, since the vertex attribute norm is not used in the shader
    //program, shader->normalLoc = -1. If we access norm in the shader program,
    //then this value will be >= 0.
    shader->vertexLoc = glGetAttribLocation(shader->program, "pos");
    shader->normalLoc = glGetAttribLocation(shader->program, "norm");
    shader->colorLoc = glGetAttribLocation(shader->program, "color");

    //Create buffers for the vertex and normal attribute arrays
    GLuint bufs[3];
    glGenBuffers(3, bufs);

    shader->vertexBuffer = bufs[0];
    shader->normalBuffer = bufs[1];
    shader->colorBuffer = bufs[2];

    //This is where we pass the vertex/normal data to the GPU.
    //In genereal, the procedure for working with buffers is:
    //  1. Tell OpenGL which buffer we're using (glBindBuffer)
    //  2. Tell OpenGL what to do with the buffer (e.g. fill buffer, use the
    //     in the buffer, etc).
    //
    //Here we are filling the buffers (glBufferData). The last parameter
    //(GL_STATIC_DRAW), says that our intention is to not change the values in
    //these buffers. If we were going to be modifying these positions frequently
    //at runtime, we might want to make this GL_DYNAMIC_DRAW instead. For right
    //now, it's not too important which you choose.

    glBindBuffer(GL_ARRAY_BUFFER, shader->vertexBuffer);
    glBufferData(
            GL_ARRAY_BUFFER, //what kind of buffer (an array)
            verts.size() * sizeof(float), //size of the buffer in bytes
            verts.data(), //pointer to data we want to fill the buffer with
            GL_STATIC_DRAW //how we intend to use the buffer
            );

    glBindBuffer(GL_ARRAY_BUFFER, shader->normalBuffer);
    glBufferData(
            GL_ARRAY_BUFFER,
            norms.size() * sizeof(float),
            norms.data(),
            GL_STATIC_DRAW
            );

    glBindBuffer(GL_ARRAY_BUFFER, shader->colorBuffer);
    glBufferData(
            GL_ARRAY_BUFFER, //what kind of buffer (an array)
            colors.size() * sizeof(float), //size of the buffer in bytes
            colors.data(), //pointer to data we want to fill the buffer with
            GL_STATIC_DRAW //how we intend to use the buffer
            );
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    setupGLUT();
    setupGL();

    glewInit();

    setupShaders();

    glutMainLoop();

    if (shader) delete shader;

    return 0;
}

