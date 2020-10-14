#include <GLFW/glfw3.h>
#include <GL/glu.h>   

#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <math.h>
#include <bits/stdc++.h> 

using namespace std;
using namespace boost;
 
vector<array<double,3>> vertices;
vector<array<int,3>> faces;
array<double,3> currentVertex;
array<int,3> currentFace;
double currentStod;
double maxVertexCoordValue = 0;

int pbufferWidth, pbufferHeight;

// this vector contains .obj positions where
vector<int> usemtlPositions; // 'usemtl <material>' met

float red = 0.0f, green = 0.0f, blue = 0.0f;

// checks if vector<int> contains the integer
bool isInVector(int thatInt, vector<int> thatVector) {
    int i;
    for (i = 0; i < thatVector.size(); i++) {
        if (thatVector[i] == thatInt) {
            break;
        }
    }
    if (i == thatVector.size()) return false;
    else return true;
}

// draws a layer with specified offsets
void renderLayer(int vX, int vY, int vZ, GLFWwindow* window) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < faces.size(); i++) {
        // changing color tint for each faces group
        if (isInVector(i, usemtlPositions)) {
            if (red < 1.0f) red += 0.1f;
            if (green < 1.0f && red == 1.0f) green += 0.1f;
            if (blue < 0.6f && green == 1.0f) blue += 0.1f;
            glColor3f(red, green, blue);
        }
        currentFace = faces.at(i);
        for (int j = 0; j < 3; j++) {
            currentVertex = vertices[currentFace[j]];
            glVertex3d(currentVertex[vX]/maxVertexCoordValue, 
                        currentVertex[vY]/maxVertexCoordValue, 
                        currentVertex[vZ]/maxVertexCoordValue);
        }
    }
    glEnd();
    red = 0; green = 0; blue = 0;
    
    glfwSwapBuffers(window);
}


int main(int argc, char** argv) {
    setprecision(1); // dunno if this needed

    // "dimension" is like, the max voxel width of a result
    int dimension; sscanf(argv[2], "%d", &dimension);

    //argv[1] = "/home/m8u/manjaro backup/3dmodels/suzanne.obj";
    //dimension = 32;

    // and also this is a resolution value of a render
    pbufferWidth = dimension, pbufferHeight = dimension;

    int i, j;
    string line;
    ifstream file (argv[1]);
    vector<string> splittedLine;
    vector<string> splittedFaceFragment;

    // .obj parsing
    if (file.is_open()) {
        while ( getline (file,line) ) {
            split(splittedLine, line, is_any_of(" ")); // chevo blyat?
            
            if (splittedLine.at(0) == "v") {
                for (i = 0; i < 3; i++) {
                    currentStod = stod(splittedLine[i+1]);
                    if (abs(currentStod) > maxVertexCoordValue) {
                        maxVertexCoordValue = abs(currentStod);
                    }
                    currentVertex[i] = currentStod;
                }
                vertices.push_back(currentVertex);
            }
            if (splittedLine.at(0) == "usemtl") {
                usemtlPositions.push_back(faces.size());
            }
            if (splittedLine.at(0) == "f") {
                for (i = 0; i < 3; i++) {
                    split(splittedFaceFragment, splittedLine.at(i+1), is_any_of("/"));
                    currentFace[i] = stoi(splittedFaceFragment[0]) - 1;
                }
                faces.push_back(currentFace);
            }
        }
        file.close();
        //for (int i = 0; i < usemtlPositions.size(); i++) cout << usemtlPositions.at(i) << ' ';
    } else { 
        cout << "No such file" << endl;
        return 1;
    }

    // GLFW initialization
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(pbufferWidth, pbufferHeight, "voxelizer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    double zOffset;
    float pixels[3*pbufferWidth*pbufferHeight];
    char *outputVoxels = new char[dimension*dimension*dimension]; // fake 3d array
    int p, x = 0, y = 0, z = 0;
    char charset[27] = "abcdefghijklmnopqrstuvwxyz";

    // "scanning" front and back faces with "camera" flying through the model
    for (zOffset = -1.0-(2.0/(double)dimension); zOffset <= 1.0+(2.0/(double)dimension); zOffset += (2.0/(double)dimension)) {

        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, zOffset+(2.0/(double)dimension), zOffset+2*(2.0/(double)dimension));
        glPushMatrix();
        
        renderLayer(0, 1, 2, window); 

        glReadBuffer(GL_COLOR_ATTACHMENT0);// again dunno if this needed, there was some floatng point problems
        // reading the pixels of rasterized model
        glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGB, GL_FLOAT, &pixels);

        // filling output array with chars respectively to color values
        for (p = 3*pbufferWidth*pbufferHeight-1; p >= 0; p-=3) {
            //cout <<pixels[p-2]<<' '<<pixels[p-1]<<' '<<pixels[p] << ", ";
            if (pixels[p-2] > 0) {
                //cout << (int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1 << ": " << charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1] << endl;
                outputVoxels[z + dimension * (y + dimension * x)] = charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1];
            } else {
                outputVoxels[z + dimension * (y + dimension * x)] = '0'; // emptiness
            }
            x++;

            if (((p)/3) % pbufferWidth == 0 && p != 3*pbufferWidth*pbufferHeight-1) {
                y++; x = 0;
            }
        }
        z++; y = 0; x = 0;
    }

    // then "scanning" in two other axis, weirdly abstractedly rotating the output array

    // left and right
    z = 0; y = 0; x = 0;
    for (zOffset = -1.0-(2.0/(double)dimension); zOffset <= 1.0+(2.0/(double)dimension); zOffset += (2.0/(double)dimension)) {

        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, zOffset+(2.0/(double)dimension), zOffset+2*(2.0/(double)dimension));
        glPushMatrix();
        
        renderLayer(2, 1, 0, window); 

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGB, GL_FLOAT, &pixels);

        for (p = 3*pbufferWidth*pbufferHeight-1; p >= 0; p-=3) {
            if (pixels[p-2] > 0) {
                //cout << (int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1 << ": " << charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1] << endl;
                outputVoxels[x + dimension * (y + dimension * z)] = charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1];
            }
            x++;

            if (((p)/3) % pbufferWidth == 0 && p != 3*pbufferWidth*pbufferHeight-1) {
                y++; x = 0;
            }
        }
        z++; y = 0; x = 0;
    }

    // top and bottom
    z = 0; y = 0; x = 0;
    for (zOffset = -1.0-(2.0/(double)dimension); 
         zOffset <= 1.0+(2.0/(double)dimension);
         zOffset += (2.0/(double)dimension)) {

        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, zOffset+(2.0/(double)dimension), zOffset+2*(2.0/(double)dimension));
        glPushMatrix();
        
        renderLayer(0, 2, 1, window);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGB, GL_FLOAT, &pixels);

        for (p = 3*pbufferWidth*pbufferHeight-1; p >= 0; p-=3) {
            if (pixels[p-2] > 0) {
                //cout << (int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1 << ": " << charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1] << endl;
                outputVoxels[y + dimension * (z + dimension * x)] = charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1];
            }
            x++;

            if (((p)/3) % pbufferWidth == 0 && p != 3*pbufferWidth*pbufferHeight-1) {
                y++; x = 0;
            }
        }
        z++; x = 0; y = 0;
    }

    // outputing to stdout 
    // I know it's stupid
    for (z = 0; z < dimension; z++) {
        for (y = dimension-1; y >= 0; y--) {
            for (x = 0; x < dimension; x++) {
                cout << outputVoxels[z + dimension * (y + dimension * x)];
            }
            cout << endl;
        }
    }


    //glfwTerminate();

    // TODO: fricking make this a native java interface or somthin

    return 0;

}
