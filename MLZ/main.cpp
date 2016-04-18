// main.cpp

#include <iostream>
#include <IMLib/IMLib.h>
#include <IMLib/PHCamera.h>
#include <IMLib/PHOrientation.h>

using namespace std;

int main()
{
    // object space coordinate system X - left, Z - forward, Y - up
    // object space refernce points locations
    MAVector3 objA, objB, imA, imB;
    MAVector3 gtX0, mlzX0, error;
    std::vector<MAVector3> imagePoints, objectPoints;
    
    objA.x = -500.0;
    objA.y = 500.0;
    objA.z = 0.0;
    
    objB.x = 500.0;
    objB.y = 0.0;
    objB.z = 0.0;
    
    PHCamera camera;
    camera.loadParameters("_impara01.txt");
    gtX0 = camera.m_X0;
    imA = camera.project(objA);
    imB = camera.project(objB);
    cout << "imA: " << imA << ", imB: " << imB << endl;
    
    // estimate external orietnation using MLZ
    PHOrientation orientation;
    orientation.setCamera(camera);
    imagePoints.push_back(imA);
    imagePoints.push_back(imB);
    objectPoints.push_back(objA);
    objectPoints.push_back(objB);
    orientation.setImagePoints(imagePoints);
    orientation.setObjectPoints(objectPoints);
    orientation.makeMLZ(camera.m_omega, camera.m_kappa);
    mlzX0 = orientation.camera().m_X0;
    
    error = gtX0 - mlzX0;
    
    cout << "gtX0: " << gtX0 << endl;
    cout << "mlzX0: " << mlzX0 << endl;
    cout << "error: " << error.modulus() << endl;

    return 0;
}