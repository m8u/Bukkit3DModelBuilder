#include <GL/osmesa.h>
#include <GL/glu.h>   

#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>
#include <math.h>

#include<bits/stdc++.h> 

using namespace std;
using namespace boost;
 
vector<array<double,3>> vertices;
vector<array<int,3>> faces;
array<double,3> currentVertex;
array<int,3> currentFace;
double currentStod;
double maxVertexCoordValue = 0;

int pbufferWidth, pbufferHeight;

vector<int> usemtlPositions;
float red = 0.0f, green = 0.0f, blue = 0.0f;

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

void renderLayer(int vX, int vY, int vZ) {  // Display function will draw the image.
 
    glClearColor(0, 0, 0, 1);  // (In fact, this is the default.)
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < faces.size(); i++) {
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

}


int main(int argc, char** argv) {
    setprecision(1);

    int dimension; sscanf(argv[2], "%d", &dimension);

    //argv[1] = "/home/m8u/Shlakocode/C++/voxelization/suzanne.obj";
    //dimension = 16;

    pbufferWidth = dimension, pbufferHeight = dimension;

    int i, j;

    string line;
    ifstream file (argv[1]);
    vector<string> splittedLine;
    vector<string> splittedFaceFragment;

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

    GLfloat *buffer;
 
    /* Create an RGBA-mode context */
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
    /* specify Z, stencil, accum sizes */
    OSMesaContext ctx = OSMesaCreateContextExt( GL_RGBA, 16, 0, 0, NULL );
#else
    OSMesaContext ctx = OSMesaCreateContext( GL_RGBA, NULL );
#endif
    if (!ctx) {
        printf("OSMesaCreateContext failed!\n");
        return 0;
    }
 
    /* Allocate the image buffer */
    buffer = (GLfloat *) malloc( pbufferWidth * pbufferHeight * 4 * sizeof(GLfloat));
    if (!buffer) {
        printf("Alloc image buffer failed!\n");
        return 0;
    }
 
    /* Bind the buffer to the context and make it current */
    if (!OSMesaMakeCurrent( ctx, buffer, GL_FLOAT, pbufferWidth, pbufferHeight )) {
        printf("OSMesaMakeCurrent failed!\n");
        return 0;
    }


    double zOffset;
    float pixels[3*pbufferWidth*pbufferHeight];

    char *outputVoxels = new char[dimension*dimension*dimension];

    int p, x = 0, y = 0, z = 0;

    /*map<int, char> charset;

    char mapValue = 'a';
    for (float mapKey = 0.1f; mapKey <= 2.6f; mapKey += 0.1f) {
        if ((int)round(mapKey*10) % 10 == 0) continue;
        cout << mapKey*10 << ": " << mapValue << endl;
        charset.insert(make_pair((int)mapKey*10, mapValue));
        mapValue++;
    }*/
    char charset[27] = "abcdefghijklmnopqrstuvwxyz";

    // front and back
    for (zOffset = -1.0-(2.0/(double)dimension); zOffset <= 1.0+(2.0/(double)dimension); zOffset += (2.0/(double)dimension)) {

        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, zOffset+(2.0/(double)dimension), zOffset+2*(2.0/(double)dimension));
        glPushMatrix();
        
        renderLayer(0, 1, 2); 

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGB, GL_FLOAT, &pixels);

        for (p = 3*pbufferWidth*pbufferHeight-1; p >= 0; p-=3) {
            //cout <<pixels[p-2]<<' '<<pixels[p-1]<<' '<<pixels[p] << ", ";
            
            if (pixels[p-2] > 0) {
                //cout << (int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1 << ": " << charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1] << endl;
                outputVoxels[z + dimension * (y + dimension * x)] = charset[(int)round(pixels[p]+pixels[p-1]+pixels[p-2]*10)-1];
            } else {
                outputVoxels[z + dimension * (y + dimension * x)] = '0';
            }
            x++;

            if (((p)/3) % pbufferWidth == 0 && p != 3*pbufferWidth*pbufferHeight-1) {
                y++; x = 0;
            }
        }
        z++; y = 0; x = 0;
    }

    // left and right
    z = 0; y = 0; x = 0;
    for (zOffset = -1.0-(2.0/(double)dimension); zOffset <= 1.0+(2.0/(double)dimension); zOffset += (2.0/(double)dimension)) {

        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, zOffset+(2.0/(double)dimension), zOffset+2*(2.0/(double)dimension));
        glPushMatrix();
        
        renderLayer(2, 1, 0); 

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
        
        renderLayer(0, 2, 1);

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

    for (z = 0; z < dimension; z++) {
        for (y = dimension-1; y >= 0; y--) {
            for (x = 0; x < dimension; x++) {
                cout << outputVoxels[z + dimension * (y + dimension * x)];
            }
            cout << endl;
        }
    }

    return 0;

}
