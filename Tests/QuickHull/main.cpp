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
    
    std::vector<MAVector3> vertices;
    vertices.push_back(MAVector3(0.0, 0.0, 0.0));
    vertices.push_back(MAVector3(1.0, 0.0, 0.0));
    vertices.push_back(MAVector3(0.0, 1.0, 0.0));
    vertices.push_back(MAVector3(0.0, 0.0, 1.0));
    
    PHQuickHull quickHull(vertices);
    quickHull.buildMesh(0.1);
    
    cout << "Hello, world!" << endl;
    
    return 0;
}
