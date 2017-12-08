// PHQuickHull.h
// класс, реализующий построение выпуклой оболочки

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <array>
#include <math.h>
#include <cmath>
#include "MAMatrix.h"
#include "PHMesh.h"

using namespace std;

class PHPool
{
    std::vector<std::unique_ptr< std::vector<size_t> >> m_data;
public:
    void clear()
    {
        m_data.clear();
    }
    
    void reclaim(std::unique_ptr< std::vector<size_t> >& ptr)
    {
        m_data.push_back(std::move(ptr));
    }
    
    std::unique_ptr< std::vector<size_t> > get()
    {
        if (m_data.size()==0)
        {
            return std::unique_ptr< std::vector<size_t> >(new std::vector<size_t>);
        }
        auto it = m_data.end()-1;
        std::unique_ptr< std::vector<size_t> > r = std::move(*it);
        m_data.erase(it);
        return r;
    }
    
};

class PHQuickHull
{
private:
    // вершины, для которых требуется построить оболочку
    std::vector<MAVector3> m_vertices;
    
    // описываюший параллепипед
    std::array<size_t, 6> m_extremeValues;
    
    double m_epsilon;
    double m_epsilonSquared;
    double m_scale;
    
    bool m_planar;
    
    PHMesh m_mesh;
    PHPool m_indexVectorPool;
    
    std::vector<size_t> m_newFaceIndices;
    std::vector<size_t> m_newHalfEdgeIndices;
    std::vector< std::unique_ptr<std::vector<size_t>> > m_disabledFacePointVectors;
    std::vector<MAVector3> m_planarPointCloudTemp;

public:
    PHQuickHull(std::vector<MAVector3> vertices) : m_vertices(vertices)
    {
    
    }
    
    std::array<size_t,6> getExtremeValues();
    
    double getScale(const std::array<size_t,6>& extremeValues)
    {
        double s = 0;
        for (size_t i=0;i<6;i++)
        {
            const double* v = (const double*)(&m_vertices[extremeValues[i]]);
            v += i/2;
            auto a = std::abs(*v);
            if (a>s) {
                s = a;
            }
        }
        return s;
    }
    
    bool reorderHorizonEdges(std::vector<size_t>& horizonEdges)
    {
        const size_t horizonEdgeCount = horizonEdges.size();
        for (size_t i=0;i<horizonEdgeCount-1;i++)
        {
            const size_t endVertex = m_mesh.m_halfEdges[ horizonEdges[i] ].m_endVertex;
            bool foundNext = false;
            for (size_t j=i+1;j<horizonEdgeCount;j++)
            {
                const size_t beginVertex = m_mesh.m_halfEdges[ m_mesh.m_halfEdges[horizonEdges[j]].m_opp ].m_endVertex;
                if (beginVertex == endVertex)
                {
                    std::swap(horizonEdges[i+1],horizonEdges[j]);
                    foundNext = true;
                    break;
                }
            }
            if (!foundNext)
            {
                return false;
            }
        }
      
        return true;
    }
    
    void reclaimToIndexVectorPool(std::unique_ptr<std::vector<size_t>>& ptr)
    {
        const size_t oldSize = ptr->size();
        if ((oldSize+1)*128 < ptr->capacity())
        {
             ptr.reset(nullptr);
            return;
        }
        m_indexVectorPool.reclaim(ptr);
    }
    
    std::unique_ptr<std::vector<size_t>> getIndexVectorFromPool()
    {
        auto r = std::move(m_indexVectorPool.get());
        r->clear();
        return r;
    }
    
    bool addPointToFace(typename PHMesh::PHFace& f, size_t pointIndex)
    {
        const double D = MAPlane::signedDistanceToPlane(m_vertices[ pointIndex ], f.m_P);
        if (D>0 && D*D > m_epsilonSquared*f.m_P.m_sqrNLength)
        {
            if (!f.m_pointsOnPositiveSide)
            {
                f.m_pointsOnPositiveSide = std::move(getIndexVectorFromPool());
            }
            f.m_pointsOnPositiveSide->push_back( pointIndex );
            if (D > f.m_mostDistantPointDist)
            {
                f.m_mostDistantPointDist = D;
                f.m_mostDistantPoint = pointIndex;
            }
            return true;
        }
        return false;
    }
    
    // построить поверхность
    void buildMesh(double epsilon);
    
    // создаёт начальный тетраэдр
    PHMesh createInitialTetrahedron();
    
    // создаёт выпуклую облочку
    void createConvexHalfEdgeMesh();
};
