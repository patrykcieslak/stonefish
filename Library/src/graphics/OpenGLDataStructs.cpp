//
//  OpenGLDataStructs.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDataStructs.h"

using namespace sf;

QuadTreeNode::QuadTreeNode(QuadTree* tree_, QuadTreeNode* parent_, Quadrant q_, glm::vec3 origin_, GLfloat size_)
{
    tree = tree_;
    parent = parent_;
    q = q_;
    child[0] = NULL;
    child[1] = NULL;
    child[2] = NULL;
    child[3] = NULL;
    
    if(parent != NULL)
        level = parent->level+1;
    else
        level = 0;
    
    origin = origin_;
    size = size_;
    edgeFactors = glm::vec4(1);
    leaf = true;
    renderable = true;
}

QuadTreeNode::~QuadTreeNode()
{
    if(!leaf)
    {
        delete child[0];
        delete child[1];
        delete child[2];
        delete child[3];
    }
}

bool QuadTreeNode::NeedsSubdivision(glm::vec3 eye, glm::vec3 origin_)
{
    return (level < tree->maxLvl) && (glm::length2(eye - origin_) < 4*size*size);
}

void QuadTreeNode::Grow(glm::vec3 eye, glm::mat4 VP)
{
    if(NeedsSubdivision(eye, origin))
    {
        //Subdivide
        GLfloat childSize = size/2.f;
        GLfloat childOffset = size/4.f;
        child[Quadrant::NW] = new QuadTreeNode(tree, this, Quadrant::NW, origin + glm::vec3(childOffset, -childOffset, 0.f), childSize);
        child[Quadrant::NE] = new QuadTreeNode(tree, this, Quadrant::NE, origin + glm::vec3(childOffset, childOffset, 0.f), childSize);
        child[Quadrant::SW] = new QuadTreeNode(tree, this, Quadrant::SW, origin + glm::vec3(-childOffset, -childOffset, 0.f), childSize);
        child[Quadrant::SE] = new QuadTreeNode(tree, this, Quadrant::SE, origin + glm::vec3(-childOffset, childOffset, 0.f), childSize);
        leaf = false;
        renderable = false;
        
        //Grow further
        for(unsigned short i=0; i<4; ++i)
            child[i]->Grow(eye, VP);
    }
    else
    {
        //Clips space coordinates
        glm::vec4 corners[4];
        GLfloat halfSize = size/2.f;
        corners[0] = VP * glm::vec4(origin + glm::vec3(halfSize, halfSize, 0.f), 1.f);
        corners[1] = VP * glm::vec4(origin + glm::vec3(-halfSize, halfSize, 0.f), 1.f);
        corners[2] = VP * glm::vec4(origin + glm::vec3(-halfSize, -halfSize, 0.f), 1.f);
        corners[3] = VP * glm::vec4(origin + glm::vec3(halfSize, -halfSize, 0.f), 1.f);
        
        //Secure margin accounting for later displacement
        corners[0].w *= 1.1f;
        corners[1].w *= 1.1f;
        corners[2].w *= 1.1f;
        corners[3].w *= 1.1f;
        
        //Check if quad visible
        renderable = true;
        
        if(corners[0].z < -corners[0].w && corners[1].z < -corners[1].w && corners[2].z < -corners[2].w && corners[3].z < -corners[3].w) //Behind camera
            renderable = false;
        else if(corners[0].x < -corners[0].w && corners[1].x < -corners[1].w && corners[2].x < -corners[2].w && corners[3].x < -corners[3].w) //Left of frustum
            renderable = false;
        else if(corners[0].x > corners[0].w && corners[1].x > corners[1].w && corners[2].x > corners[2].w && corners[3].x > corners[3].w) //Right of frustum
            renderable = false;
        else if(corners[0].y < -corners[0].w && corners[1].y < -corners[1].w && corners[2].y < -corners[2].w && corners[3].y < -corners[3].w) //Bottom of frustum
            renderable = false;
        else if(corners[0].y > corners[0].w && corners[1].y > corners[1].w && corners[2].y > corners[2].w && corners[3].y > corners[3].w) //Top of frustum
            renderable = false;
        else if(corners[0].z > corners[0].w && corners[1].z > corners[1].w && corners[2].z > corners[2].w && corners[3].z > corners[3].w) //Further than far plane
            renderable = false;
        
        if(renderable)
        {
            //Adjust edge factors
            glm::vec3 neighborOrigin;
            neighborOrigin = origin + glm::vec3(0.f, size, 0.f);
            if(NeedsSubdivision(eye, neighborOrigin)) edgeFactors.x = 2.f;
            neighborOrigin = origin + glm::vec3(-size, 0.f, 0.f);
            if(NeedsSubdivision(eye, neighborOrigin)) edgeFactors.y = 2.f;
            neighborOrigin = origin + glm::vec3(0.f, -size, 0.f);
            if(NeedsSubdivision(eye, neighborOrigin)) edgeFactors.z = 2.f;
            neighborOrigin = origin + glm::vec3(size, 0.f, 0.f);
            if(NeedsSubdivision(eye, neighborOrigin)) edgeFactors.w = 2.f;
            
            //Add to list of rendered quads
            tree->leafs.push_back(this);
        }
    }
}

QuadTree::QuadTree(glm::vec3 origin_, GLfloat size_, unsigned int maxLevel) : origin(origin_), size(size_), maxLvl(maxLevel)
{
    root = new QuadTreeNode(this, NULL, Quadrant::NW, origin, size);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

QuadTree::~QuadTree()
{
    leafs.clear();
    delete root;
    if(vboVertex > 0) glDeleteBuffers(1, &vboVertex);
    if(vboEdgeDiv > 0) glDeleteBuffers(1, &vboEdgeDiv);
    glDeleteVertexArrays(1, &vao);
}

void QuadTree::Update(glm::vec3 eye, glm::mat4 VP)
{
    //Update quadtree structure
    root->Grow(eye, VP);
 
    //Generate rendering data
    if(vboVertex > 0) glDeleteBuffers(1, &vboVertex);
    if(vboEdgeDiv > 0) glDeleteBuffers(1, &vboEdgeDiv);
    
    std::vector<glm::vec3> data;
    std::vector<glm::vec4> data2;
    for(size_t i=0; i < leafs.size(); ++i)
    {
        glm::vec3 origin = leafs[i]->origin;
        GLfloat half = leafs[i]->size/2.f;
        data.push_back(origin + glm::vec3(half, half, 0.f));
        data.push_back(origin + glm::vec3(-half, half, 0.f));
        data.push_back(origin + glm::vec3(-half, -half, 0.f));
        data.push_back(origin + glm::vec3(half, -half, 0.f));
        data2.push_back(leafs[i]->edgeFactors);
    }
    
    glGenBuffers(1, &vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*data.size(), &data[0].x, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &vboEdgeDiv);
    glBindBuffer(GL_ARRAY_BUFFER, vboEdgeDiv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*data2.size(), &data2[0].x, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QuadTree::Cut()
{
    leafs.clear();
    delete root;
    root = new QuadTreeNode(this, NULL, Quadrant::NW, origin, size);
}

void QuadTree::Draw()
{
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vboEdgeDiv);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDrawArrays(GL_PATCHES, 0, (GLsizei)(4 * leafs.size()));
    
    glBindVertexArray(0);
}
