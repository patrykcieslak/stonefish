//
//  OpenGLDataStructs.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDataStructs__
#define __Stonefish_OpenGLDataStructs__

#include "graphics/GLSLShader.h"

namespace sf
{
    //!
    typedef enum {POINTS, LINES, LINE_STRIP} PrimitiveType;
    
    //!
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        
        Vertex()
        {
            pos = glm::vec3(0.f);
            normal = glm::vec3(0.f);
            uv = glm::vec3(-1.f);
        }
        
        friend bool operator==(const Vertex& lhs, const Vertex& rhs)
        {
            if(lhs.pos == rhs.pos && lhs.normal == rhs.normal && lhs.uv == rhs.uv)
                return true;
            return false;
        };
    };
    
    //!
    struct Face
    {
        GLuint vertexID[3];
        
        friend bool operator==(const Face& lhs, const Face& rhs)
        {
            if(lhs.vertexID[0] == rhs.vertexID[0] && lhs.vertexID[1] == rhs.vertexID[1] && lhs.vertexID[2] == rhs.vertexID[2])
                return true;
            return false;
        };
    };
    
    //!
    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<Face> faces;
        bool hasUVs;
        
        glm::vec3 computeFaceNormal(unsigned int faceID)
        {
            glm::vec3 v12 = vertices[faces[faceID].vertexID[1]].pos - vertices[faces[faceID].vertexID[0]].pos;
            glm::vec3 v13 = vertices[faces[faceID].vertexID[2]].pos - vertices[faces[faceID].vertexID[0]].pos;
            return glm::normalize(glm::cross(v12,v13));
        }
    };
    
    //!
    struct Object
    {
        Mesh* mesh;
        GLuint vao;
        GLuint vboVertex;
        GLuint vboIndex;
    };
    
    //!
    typedef enum {NW, NE, SW, SE} Quadrant;
    
    //!
    typedef enum {E, N, W, S} Direction;
    
    struct QuadTree;
    
    struct QuadTreeNode
    {
        QuadTree* tree;
        QuadTreeNode* parent;
        Quadrant q;
        QuadTreeNode* child[4];
        unsigned short level;
        unsigned short delta[4];
        unsigned long code;
        bool leaf;
        bool renderable;
        glm::vec3 origin;
        GLfloat size;
        glm::vec4 edgeFactors;
        
        QuadTreeNode(QuadTree* tree_, QuadTreeNode* parent_, Quadrant q_, glm::vec3 origin_, GLfloat size_);
        ~QuadTreeNode();
        bool NeedsSubdivision(glm::vec3 eye, glm::vec3 origin_);
        void Grow(glm::vec3 eye, glm::mat4 VP);
    };
    
    //!
    struct QuadTree
    {
        QuadTreeNode* root;
        std::vector<QuadTreeNode*> leafs;
        GLuint vao;
        GLuint vboVertex;
        GLuint vboEdgeDiv;
        glm::vec3 origin;
        GLfloat size;
        unsigned int maxLvl;
        
        QuadTree(glm::vec3 origin_, GLfloat size_, unsigned int maxLevel);
        ~QuadTree();
        void Update(glm::vec3 eye, glm::mat4 VP);
        void Cut();
        void Draw();
    };
    
    //!
    typedef enum {SIMPLE, PHYSICAL, MIRROR, TRANSPARENT} LookType;
    
    //!
    typedef enum {FLAT, FULL, UNDERWATER} DrawingMode;
    
    //!
    struct Look
    {
        LookType type;
        glm::vec3 color;
        std::vector<GLfloat> params;
        std::vector<GLuint> textures;
        GLfloat reflectivity;
    };
    
    //!
    struct ViewFrustum
    {
        GLfloat near;
        GLfloat far;
        GLfloat fov;
        GLfloat ratio;
        glm::vec3 corners[8];
    };
}

#endif
