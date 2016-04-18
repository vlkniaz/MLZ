// graphBuilder.cpp
// create accuracy graph for MLZ method

#include <iostream>
#include <fstream>
#include <random>
#include <math.h>
#include <IMLib/IMLib.h>
#include <IMLib/PHCamera.h>
#include <IMLib/PHOrientation.h>

#define _USE_MATH_DEFINES

using namespace std;

// 2d error vector of a given length
MAVector3 errorVector(double modulus)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    
    MAVector3 imError;
    
    imError.x = dist(mt);
    imError.y = dist(mt);
    imError.normalize();
    imError *= modulus;
    
    return imError;
}

int main()
{
    // object space coordinate system X - left, Z - forward, Y - up
    // object space refernce points locations
    MAVector3 objA, objB, imA, imB, imAError, imBError;
    MAVector3 gtX0, mlzX0, error;
    std::vector<MAVector3> imagePoints, objectPoints;
    int j = 0;
    int nSteps = 100;
    double angle = 0.0;
    double gtAlpha;
    double maxAngle = 89 / 180.0 * M_PI;
    double radius = 500.0;
    
    PHCamera camera;
    camera.loadParameters("_impara01.txt");
    gtAlpha = camera.m_alpha;
    
    ofstream outfile("out.csv");
    

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 1.0);
    
    // loop alog all angles
    for(j = 0; j < nSteps; j++)
    {
        angle = static_cast<double>(j)/(nSteps-1) * maxAngle;
        cout << "---------------" << endl;
        cout << "angle: " << angle * 180.0 / M_PI << endl;
        
        objA.x = cos(angle)*radius;
        objA.y = 0.0;
        objA.z = -sin(angle)*radius;
        
        objB.x = -objA.x;
        objB.y = 0.0;
        objB.z = -objA.z;
        cout << "objA.x: " << objA.x << ", objA.z: " << objA.z << endl;
        cout << "objB.x: " << objB.x << ", objB.z: " << objB.z << endl;
        
        imAError = errorVector(dist(mt));
        imBError = errorVector(dist(mt));
        
        
        gtX0 = camera.m_X0;
        imA = camera.project(objA);
        imB = camera.project(objB);
        cout << imAError << "; " << imBError << endl;
        cout << "imA: " << imA << ", imB: " << imB << endl;
        
        imA += imAError;
        imB += imBError;
        
        // estimate external orietnation using MLZ
        PHOrientation orientation;
        orientation.setCamera(camera);
        imagePoints.clear();
        objectPoints.clear();
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
        double angleError = fabs(gtAlpha - orientation.camera().m_alpha);
        cout << "error: " << error.modulus() << endl;
        cout << "angle error: " << angleError << endl;
        
        outfile << angle * 180.0 / M_PI << ";" << error.modulus() << endl;

    }
    
    outfile.close();
    

    return 0;
}