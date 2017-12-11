//
//  PHMesh.h
//  Библиотека PHLib
//
//  класс PHMesh для хранения сеточной модели
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
#include <math.h>
#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <unordered_map>
#include "MAMatrix.h"

using namespace std;

class PHMesh
{
public:
    struct PHHalfEdge
    {
        size_t m_endVertex;
        size_t m_opp;
        size_t m_face;
        size_t m_next;
        
        void disable() {
            m_endVertex = std::numeric_limits<size_t>::max();
        }
        
        bool isDisabled() const {
            return m_endVertex == std::numeric_limits<size_t>::max();
        }
    };
    
    struct PHFace
    {
        size_t m_he;
        MAPlane m_P;
        double m_mostDistantPointDist;
        size_t m_mostDistantPoint;
        size_t m_visibilityCheckedOnIteration;
        std::uint8_t m_isVisibleFaceOnCurrentIteration : 1;
        std::uint8_t m_inFaceStack : 1;
         // 3 бита, определяющие лежит ли ребро на горизонте
        std::uint8_t m_horizonEdgesOnCurrentIteration : 3;
        std::unique_ptr<std::vector<size_t>> m_pointsOnPositiveSide;
        
        PHFace() : m_he(std::numeric_limits<size_t>::max()),
        m_mostDistantPointDist(0),
        m_mostDistantPoint(0),
        m_visibilityCheckedOnIteration(0),
        m_isVisibleFaceOnCurrentIteration(0),
        m_inFaceStack(0),
        m_horizonEdgesOnCurrentIteration(0)
        {
        }
        
        void disable() {
            m_he = std::numeric_limits<size_t>::max();
        }
        
        bool isDisabled() const {
            return m_he == std::numeric_limits<size_t>::max();
        }
    };
    
    // грани сеточной модели
    std::vector< PHMesh::PHFace > m_faces;
    // полурёбра сеточной модели
    std::vector< PHHalfEdge > m_halfEdges;
    
    // списки удалённых граней и полурёбер
    std::vector< size_t > m_disabledFaces, m_disabledHalfEdges;
    
    size_t addFace()
    {
        if (m_disabledFaces.size())
        {
            size_t index = m_disabledFaces.back();
            auto& f = m_faces[index];

            f.m_mostDistantPointDist = 0;
            m_disabledFaces.pop_back();
            return index;
        }
        m_faces.emplace_back();
        return m_faces.size()-1;
    }
    
    size_t addHalfEdge()
    {
        if (m_disabledHalfEdges.size())
        {
            const size_t index = m_disabledHalfEdges.back();
            m_disabledHalfEdges.pop_back();
            return index;
        }
        m_halfEdges.emplace_back();
        return m_halfEdges.size()-1;
    }
    
    // отмечает грань как удалённую
    std::unique_ptr< std::vector<size_t> > disableFace(size_t faceIndex)
    {
        auto& f = m_faces[faceIndex];
        f.disable();
        m_disabledFaces.push_back(faceIndex);
        return std::move(f.m_pointsOnPositiveSide);
    }
    
    void disableHalfEdge(size_t heIndex)
    {
        auto& he = m_halfEdges[heIndex];
        he.disable();
        m_disabledHalfEdges.push_back(heIndex);
    }
    
    PHMesh() = default;
    
    // создат модель из начального театраэдра ABCD
    // скалярное произведение AB и нормали к треугольнику ABC должно быть отрицательным
    PHMesh(size_t a, size_t b, size_t c, size_t d)
    {
        // создаём полурёбра
        PHHalfEdge AB;
        AB.m_endVertex = b;
        AB.m_opp = 6;
        AB.m_face = 0;
        AB.m_next = 1;
        m_halfEdges.push_back(AB);
        
        PHHalfEdge BC;
        BC.m_endVertex = c;
        BC.m_opp = 9;
        BC.m_face = 0;
        BC.m_next = 2;
        m_halfEdges.push_back(BC);
        
        PHHalfEdge CA;
        CA.m_endVertex = a;
        CA.m_opp = 3;
        CA.m_face = 0;
        CA.m_next = 0;
        m_halfEdges.push_back(CA);
        
        PHHalfEdge AC;
        AC.m_endVertex = c;
        AC.m_opp = 2;
        AC.m_face = 1;
        AC.m_next = 4;
        m_halfEdges.push_back(AC);
        
        PHHalfEdge CD;
        CD.m_endVertex = d;
        CD.m_opp = 11;
        CD.m_face = 1;
        CD.m_next = 5;
        m_halfEdges.push_back(CD);
        
        PHHalfEdge DA;
        DA.m_endVertex = a;
        DA.m_opp = 7;
        DA.m_face = 1;
        DA.m_next = 3;
        m_halfEdges.push_back(DA);
        
        PHHalfEdge BA;
        BA.m_endVertex = a;
        BA.m_opp = 0;
        BA.m_face = 2;
        BA.m_next = 7;
        m_halfEdges.push_back(BA);
        
        PHHalfEdge AD;
        AD.m_endVertex = d;
        AD.m_opp = 5;
        AD.m_face = 2;
        AD.m_next = 8;
        m_halfEdges.push_back(AD);
        
        PHHalfEdge DB;
        DB.m_endVertex = b;
        DB.m_opp = 10;
        DB.m_face = 2;
        DB.m_next = 6;
        m_halfEdges.push_back(DB);
        
        PHHalfEdge CB;
        CB.m_endVertex = b;
        CB.m_opp = 1;
        CB.m_face = 3;
        CB.m_next = 10;
        m_halfEdges.push_back(CB);
        
        PHHalfEdge BD;
        BD.m_endVertex = d;
        BD.m_opp = 8;
        BD.m_face = 3;
        BD.m_next = 11;
        m_halfEdges.push_back(BD);
        
        PHHalfEdge DC;
        DC.m_endVertex = c;
        DC.m_opp = 4;
        DC.m_face = 3;
        DC.m_next = 9;
        m_halfEdges.push_back(DC);
        
        // создаём грани
        PHFace ABC;
        ABC.m_he = 0;
        m_faces.push_back(std::move(ABC));
        
        PHFace ACD;
        ACD.m_he = 3;
        m_faces.push_back(std::move(ACD));
        
        PHFace BAD;
        BAD.m_he = 6;
        m_faces.push_back(std::move(BAD));
        
        PHFace CBD;
        CBD.m_he = 9;
        m_faces.push_back(std::move(CBD));
    }
    
    std::array<size_t,3> getVertexIndicesOfFace(const PHFace& f) const
    {
        std::array<size_t,3> v;
        const PHHalfEdge* he = &m_halfEdges[f.m_he];
        v[0] = he->m_endVertex;
        he = &m_halfEdges[he->m_next];
        v[1] = he->m_endVertex;
        he = &m_halfEdges[he->m_next];
        v[2] = he->m_endVertex;
        return v;
    }
    
    std::array<size_t,2> getVertexIndicesOfHalfEdge(const PHHalfEdge& he) const
    {
        return {{m_halfEdges[he.m_opp].m_endVertex,he.m_endVertex}};
    }
    
    std::array<size_t,3> getHalfEdgeIndicesOfFace(const PHFace& f) const
    {
        return {{f.m_he,m_halfEdges[f.m_he].m_next,m_halfEdges[m_halfEdges[f.m_he].m_next].m_next}};
    }
};
