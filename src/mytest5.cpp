#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <glm/glm.hpp>
#include <cmath>
#include "shaders.h"
#include <stdlib.h>
#include "vessel.h"
#include <vector>
#include <map>
#include <imgui.h>
#include <backends/imgui_impl_glut.h>
#include <backends/imgui_impl_opengl3.h>
#include <commdlg.h>

const double M_PI = 3.14159265359;

bool start_screen = true;

extern std::vector<float> vertices;
extern std::map<int, std::map<int, std::map<int, std::map<char, unsigned int>>>> vertex_map;
extern std::vector<unsigned int> indices;
extern std::vector<float> colors;
extern std::vector<float> normals;

int mouseoldx, mouseoldy;
glm::vec3 eye;
glm::vec3 up;
const glm::vec3 eyeinit = glm::vec3(0, 500, 0);
const glm::vec3 upinit = glm::vec3(0, 0, 1);
const glm::vec3 center = glm::vec3(0, 0, 0);
const glm::vec2 screen_up = glm::vec2(-1, 0);
GLuint vertexshader, fragmentshader, shaderprogram;
GLuint projectionPos, modelviewPos, lightdirPos;
glm::mat4 projection, modelview;
glm::vec3 lightdir;

const int numobjects = 1;
const int numperobj = 4;
GLuint VAOs[numobjects];
GLuint buffers[numperobj * numobjects];
GLenum PrimType[numobjects];
GLsizei NumElems[numobjects];

enum {Vertices, Colors, Normals, Elements};

void deleteBuffers() {
    glDeleteVertexArrays(numobjects, VAOs);
    glDeleteBuffers(numperobj * numobjects, buffers);
}

glm::mat3 dual_matrix(glm::vec3 a) {
    return glm::mat3(0, a[2], -a[1], -a[2], 0, a[0], a[1], -a[0], 0);
}

glm::vec3 cross(glm::vec3 a, glm::vec3 b) {
    return dual_matrix(a) * b;
}

glm::vec3 normalize(glm::vec3 a) {
    float magnitude = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    return glm::vec3(a[0] / magnitude, a[1] / magnitude, a[2] / magnitude);
}

glm::mat4 lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 x, y, z;
    glm::mat4 translation_matrix, rotation_matrix;
    z = eye - center;
    x = cross(up, z);
    y = cross(z, x);
    x = normalize(x);
    y = normalize(y);
    z = normalize(z);
    rotation_matrix = glm::mat4(x[0], y[0], z[0], 0, x[1], y[1], z[1], 0, x[2], y[2], z[2], 0, 0, 0, 0, 1);
    translation_matrix = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -eye[0], -eye[1], -eye[2], 1);
    return rotation_matrix * translation_matrix;
}

void initobject(GLuint object, GLfloat *vert, GLint sizevert, GLfloat *col, GLint sizecol, GLfloat *nor, GLint sizenor, GLuint *inds, GLint sizeind, GLenum type) {
    int offset = object * numperobj;
    glBindVertexArray(VAOs[object]);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[Vertices + offset]);
    glBufferData(GL_ARRAY_BUFFER, sizevert, vert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[Colors + offset]);
    glBufferData(GL_ARRAY_BUFFER, sizecol, col, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[Normals + offset]);
    glBufferData(GL_ARRAY_BUFFER, sizenor, nor, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Elements + offset]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeind, inds, GL_STATIC_DRAW);
    
    PrimType[object] = type;
    NumElems[object] = sizeind / 4;

    glBindVertexArray(0);
}

void drawobject(GLuint object) {
    glBindVertexArray(VAOs[object]);
    glDrawElements(PrimType[object], NumElems[object], GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

float dot_3d(glm::vec3 vec1, glm::vec3 vec2) {
    return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

void display() {

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    // Start screen
    if (start_screen) {
        ImGui::Begin("Open a segmentation (.nii) file");
        if (ImGui::Button("Open...")) {

            OPENFILENAMEA ofn;
            CHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = FindWindowA(nullptr, "Demo");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "NIFTI (*.nii)\0*.nii\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameA(&ofn) == TRUE) {
                std::cout << ofn.lpstrFile << std::endl;
            }

            start_screen = false;
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawobject(0);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    }
    else {
        int x_ = y, y_ = x;

        if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_UP) {
                // Do Nothing;
            }
            else if (state == GLUT_DOWN) {
                mouseoldx = x_;
                mouseoldy = y_;
            }
        }
        else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
            eye = eyeinit;

            lightdir = normalize(eye);
            glUniform3f(lightdirPos, lightdir[0], lightdir[1], lightdir[2]);

            up = upinit;
            modelview = lookAt(eye, center, up);
            glUniformMatrix4fv(modelviewPos, 1, GL_FALSE, &modelview[0][0]);
            glutPostRedisplay();
        }
    }
}

glm::mat2 rotate_2d(float angle) {
    return glm::mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

glm::mat3 rotate_3d_with_cos_sin(glm::vec3 axis, float cos_angle, float sin_angle) {
    axis = normalize(axis);
    glm::mat3 I = glm::mat3(1);
    glm::mat3 dual_axis = dual_matrix(axis);
    glm::mat3 rotation_matrix = glm::mat3();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rotation_matrix[i][j] = I[i][j] * cos_angle + axis[i] * axis[j] * (1 - cos_angle) + dual_axis[i][j] * sin_angle;
        }
    }
    return rotation_matrix;
}

glm::mat3 rotate_3d(glm::vec3 axis, float angle) {
    axis = normalize(axis);
    float cos_angle = cos(angle);
    float sin_angle = sin(angle);
    glm::mat3 I = glm::mat3(1);
    glm::mat3 dual_axis = dual_matrix(axis);
    glm::mat3 rotation_matrix = glm::mat3();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rotation_matrix[i][j] = I[i][j] * cos_angle + axis[i] * axis[j] * (1 - cos_angle) + dual_axis[i][j] * sin_angle;
        }
    }
    return rotation_matrix;
}

float dot_2d(glm::vec2 vec1, glm::vec2 vec2) {
    return vec1[0] * vec2[0] + vec1[1] * vec2[1];
}

float magnitude_2d(glm::vec2 vec) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

float cos_vec_2d(glm::vec2 vec1, glm::vec2 vec2) {
    return dot_2d(vec1, vec2) / (magnitude_2d(vec1) * magnitude_2d(vec2));
}

void mousedrag(int x, int y) {

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        ImGui_ImplGLUT_MotionFunc(x, y);
    }
    else {
        int x_ = y, y_ = x;
        if (x_ - mouseoldx == 0 && y_ - mouseoldy == 0) return;
        glm::vec2 drag = glm::vec2(x_ - mouseoldx, y_ - mouseoldy);
        drag = rotate_2d(-M_PI / 2) * drag;
        float cos_screen_up_drag = cos_vec_2d(screen_up, drag);
        float sin_screen_up_drag = sqrt(1 - cos_screen_up_drag * cos_screen_up_drag);
        if (drag[1] > 0) {
            sin_screen_up_drag = -sin_screen_up_drag;
        }
        glm::vec3 rotation_axis = rotate_3d_with_cos_sin(eye - center, cos_screen_up_drag, sin_screen_up_drag) * up;
        float rotation_angle = 0.005 * magnitude_2d(drag);
        float degrees = rotation_angle * 180 / M_PI;
        glm::mat3 rotation_matrix = rotate_3d(rotation_axis, rotation_angle);
        eye = rotation_matrix * eye;

        lightdir = normalize(eye);
        glUniform3f(lightdirPos, lightdir[0], lightdir[1], lightdir[2]);

        up = rotation_matrix * up;
        mouseoldx = x_;
        mouseoldy = y_;
        modelview = lookAt(eye, center, up);
        glUniformMatrix4fv(modelviewPos, 1, GL_FALSE, &modelview[0][0]);
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    }
    else {
        switch (key)
        {
        case 27:
            deleteBuffers();
            exit(0);
            break;

        default:
            break;
        }
    }
}

glm::mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
    float d = cos(fovy / 2) / sin(fovy / 2);
    return glm::mat4(d / aspect, 0, 0, 0, 0, d, 0, 0, 0, 0, (zNear + zFar) / (zNear - zFar), -1, 0, 0, 2 * zNear * zFar / (zNear - zFar), 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    projection = perspective(30 * M_PI / 180, (float)w / h, 300, 800);
    glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);
    ImGui_ImplGLUT_ReshapeFunc(w, h);
}

void init() {
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    projection = glm::mat4(1.0f);

    eye = eyeinit;
    up = upinit;
    modelview = lookAt(eye, center, up);

    glGenVertexArrays(numobjects, VAOs);
    glGenBuffers(numperobj * numobjects, buffers);

    initobject(0, (GLfloat*)&vertices[0], vertices.size() * 4, (GLfloat*)&colors[0], colors.size() * 4, (GLfloat*)&normals[0], normals.size() * 4, (GLuint*)&indices[0], indices.size() * 4, GL_TRIANGLES);

    vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/vessel.vert");
    fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/vessel.frag");
    shaderprogram = initprogram(vertexshader, fragmentshader);

    projectionPos = glGetUniformLocation(shaderprogram, "projection");
    modelviewPos = glGetUniformLocation(shaderprogram, "modelview");
    lightdirPos = glGetUniformLocation(shaderprogram, "light_direction");

    glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(modelviewPos, 1, GL_FALSE, &modelview[0][0]);
    
    lightdir = normalize(eyeinit);
    glUniform3f(lightdirPos, lightdir[0], lightdir[1], lightdir[2]);
}

int main(int argc, char** argv) {

    int ret = load_vessel();

    if (ret == 1)
    {
        std::cout << "Success loading vessel" << std::endl;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Demo");
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Error " << glewGetString(err) << std::endl;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

    init();

    // Install GLUT handlers
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mousedrag);
    glutPassiveMotionFunc(ImGui_ImplGLUT_MotionFunc);
    glutKeyboardUpFunc(ImGui_ImplGLUT_KeyboardUpFunc);
    glutSpecialFunc(ImGui_ImplGLUT_SpecialFunc);
    glutSpecialUpFunc(ImGui_ImplGLUT_SpecialUpFunc);

    glutMainLoop();
    deleteBuffers();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}