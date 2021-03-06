//
//  PHConvexHull.h
//  Библиотека PHLib
//
//  класс PHConvexHull для хранения выпуклой оболочки
//
//  Created by Vladimir Knyaz on 01.12.17.
//  Copyright (c) 2017 Vladimir Knyaz. All rights reserved.
//

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

// класс PHConvexHull для хранения выпуклой оболочки
class PHConvexHull
{
    std::unique_ptr<std::vector<MAVector3>> m_optimizedVertexBuffer;
    std::vector<MAVector3> m_vertices;
    std::vector<size_t> m_indices;
public:
    PHConvexHull() {}
    
    // копирующий конструктор
    PHConvexHull(const PHConvexHull& o)
    {
        m_indices = o.m_indices;
        if (o.m_optimizedVertexBuffer)
        {
            m_optimizedVertexBuffer.reset(new std::vector<MAVector3>(*o.m_optimizedVertexBuffer));
            m_vertices = std::vector<MAVector3>(*m_optimizedVertexBuffer);
        }
        else
        {
            m_vertices = o.m_vertices;
        }
    }
    
    PHConvexHull& operator=(const PHConvexHull& o)
    {
        if (&o == this)
        {
            return *this;
        }
        m_indices = o.m_indices;
        if (o.m_optimizedVertexBuffer)
        {
            m_optimizedVertexBuffer.reset(new std::vector<MAVector3>(*o.m_optimizedVertexBuffer));
            m_vertices = std::vector<MAVector3>(*m_optimizedVertexBuffer);
        }
        else
        {
            m_vertices = o.m_vertices;
        }
        return *this;
    }
    
    PHConvexHull(PHConvexHull&& o)
    {
        m_indices = std::move(o.m_indices);
        if (o.m_optimizedVertexBuffer)
        {
            m_optimizedVertexBuffer = std::move(o.m_optimizedVertexBuffer);
            o.m_vertices = std::vector<MAVector3>();
            m_vertices = std::vector<MAVector3>(*m_optimizedVertexBuffer);
        }
        else
        {
            m_vertices = o.m_vertices;
        }
    }
    
    PHConvexHull& operator=(PHConvexHull&& o)
    {
        if (&o == this)
        {
            return *this;
        }
        m_indices = std::move(o.m_indices);
        if (o.m_optimizedVertexBuffer)
        {
            m_optimizedVertexBuffer = std::move(o.m_optimizedVertexBuffer);
            o.m_vertices = std::vector<MAVector3>();
            m_vertices = std::vector<MAVector3>(*m_optimizedVertexBuffer);
        }
        else
        {
            m_vertices = o.m_vertices;
        }
        return *this;
    }
    
    // создаём массивы вершины и индексов из сеточной модели или облака точек
    PHConvexHull(const PHMesh& mesh, const std::vector<MAVector3>& pointCloud, bool CCW, bool useOriginalIndices)
    {
        if (!useOriginalIndices)
        {
            m_optimizedVertexBuffer.reset(new std::vector<MAVector3>());
        }
        
        std::vector<bool> faceProcessed(mesh.m_faces.size(),false);
        std::vector<size_t> faceStack;
        // переводим индексы вершин из исходного облака точек в новые индексы
        std::unordered_map<size_t,size_t> vertexIndexMapping;
        for (size_t i = 0;i<mesh.m_faces.size();i++)
        {
            if (!mesh.m_faces[i].isDisabled())
            {
                faceStack.push_back(i);
                break;
            }
        }
        if (faceStack.size()==0)
        {
            return;
        }
        
        const size_t finalMeshFaceCount = mesh.m_faces.size() - mesh.m_disabledFaces.size();
        m_indices.reserve(finalMeshFaceCount*3);
        
        while (faceStack.size())
        {
            auto it = faceStack.end()-1;
            size_t top = *it;
            assert(!mesh.m_faces[top].isDisabled());
            faceStack.erase(it);
            if (faceProcessed[top])
            {
                continue;
            }
            else
            {
                faceProcessed[top]=true;
                auto halfEdges = mesh.getHalfEdgeIndicesOfFace(mesh.m_faces[top]);
                size_t adjacent[] = {mesh.m_halfEdges[mesh.m_halfEdges[halfEdges[0]].m_opp].m_face,mesh.m_halfEdges[mesh.m_halfEdges[halfEdges[1]].m_opp].m_face,mesh.m_halfEdges[mesh.m_halfEdges[halfEdges[2]].m_opp].m_face};
                for (auto a : adjacent)
                {
                    if (!faceProcessed[a] && !mesh.m_faces[a].isDisabled())
                    {
                        faceStack.push_back(a);
                    }
                }
                auto vertices = mesh.getVertexIndicesOfFace(mesh.m_faces[top]);
                if (!useOriginalIndices)
                {
                    for (auto& v : vertices)
                    {
                        auto it = vertexIndexMapping.find(v);
                        if (it == vertexIndexMapping.end())
                        {
                            m_optimizedVertexBuffer->push_back(pointCloud[v]);
                            vertexIndexMapping[v] = m_optimizedVertexBuffer->size()-1;
                            v = m_optimizedVertexBuffer->size()-1;
                        }
                        else
                        {
                            v = it->second;
                        }
                    }
                }
                m_indices.push_back(vertices[0]);
                if (CCW)
                {
                    m_indices.push_back(vertices[2]);
                    m_indices.push_back(vertices[1]);
                }
                else
                {
                    m_indices.push_back(vertices[1]);
                    m_indices.push_back(vertices[2]);
                }
            }
        }
        
        if (!useOriginalIndices)
        {
            m_vertices = std::vector<MAVector3>(*m_optimizedVertexBuffer);
        }
        else
        {
            m_vertices = pointCloud;
        }
    }
    
    std::vector<size_t>& getIndexBuffer()
    {
        return m_indices;
    }
    
    std::vector<MAVector3>& getVertexBuffer()
    {
        return m_vertices;
    }
    
    // экспорт модели в формате OBJ
    void writeWaveformOBJ(const std::string& filename, const std::string& objectName = "quickhull")
    {
        std::ofstream objFile;
        objFile.open (filename);
        objFile << "o " << objectName << "\n";
        for (const auto& v : getVertexBuffer())
        {
            objFile << "v " << v.x << " " << v.y << " " << v.z << "\n";
        }
        const auto& indBuf = getIndexBuffer();
        size_t triangleCount = indBuf.size()/3;
        for (size_t i=0;i<triangleCount;i++)
        {
            objFile << "f " << indBuf[i*3]+1 << " " << indBuf[i*3+1]+1 << " " << indBuf[i*3+2]+1 << "\n";
        }
        objFile.close();
    }
    
};
