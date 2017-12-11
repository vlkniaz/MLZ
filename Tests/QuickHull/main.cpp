// main.cpp
// example of IMImageConverter class

#include <iostream>
#include <string>
#include <IMLib/IMLib.h>
#include <IMLib/IMImageConverter.h>
#include <IMLib/IMTIFFImageFileFormat.h>
#include "tiny_obj_loader.h"
#include <IMLib/PHQuickHull.h>

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        cout << "Not enough arguments. Please enter OBJ file name." << endl;
        return 0;
    }
    string file = argv[1];
    size_t lastindex = file.find_last_of(".");
    string filename = file.substr(0, lastindex);
    std::string err;
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    // читаем файл
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());
    
    // получаем вершины
    std::vector<MAVector3> vertices;
    
    int verticesCount = attrib.vertices.size() / 3;
    
    for(int j = 0; j < verticesCount; j++)
    {
        MAVector3 v;
        v.x = attrib.vertices[3*j + 0];
        v.y = attrib.vertices[3*j + 1];
        v.z = attrib.vertices[3*j + 2];
        
        vertices.push_back(v);
    }
    
    PHQuickHull quickHull(vertices);
    PHConvexHull hull = quickHull.createConvexHull(vertices, true, false);
    hull.writeWaveformOBJ("hull.obj");
    
    cout << "Hello, world!" << endl;
    
    return 0;
}
