/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  OpenGLDataStructs.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDataStructs.h"

#include "StonefishCommon.h"

namespace sf
{

GLfloat Color::bbSpectrum(GLfloat wavelength, GLfloat temperature)
{
    double wlm = wavelength * 1e-9;   /* Wavelength in meters */
    return (3.74183e-16 * pow(wlm, -5.0)) / (exp(1.4388e-2 / (wlm * temperature)) - 1.0);
}

void Color::bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z)
{
    GLfloat X = 0, Y = 0, Z = 0, XYZ;
    
    static GLfloat cie_colour_match[81][3] = {
        {0.0014,0.0000,0.0065}, {0.0022,0.0001,0.0105}, {0.0042,0.0001,0.0201},
        {0.0076,0.0002,0.0362}, {0.0143,0.0004,0.0679}, {0.0232,0.0006,0.1102},
        {0.0435,0.0012,0.2074}, {0.0776,0.0022,0.3713}, {0.1344,0.0040,0.6456},
        {0.2148,0.0073,1.0391}, {0.2839,0.0116,1.3856}, {0.3285,0.0168,1.6230},
        {0.3483,0.0230,1.7471}, {0.3481,0.0298,1.7826}, {0.3362,0.0380,1.7721},
        {0.3187,0.0480,1.7441}, {0.2908,0.0600,1.6692}, {0.2511,0.0739,1.5281},
        {0.1954,0.0910,1.2876}, {0.1421,0.1126,1.0419}, {0.0956,0.1390,0.8130},
        {0.0580,0.1693,0.6162}, {0.0320,0.2080,0.4652}, {0.0147,0.2586,0.3533},
        {0.0049,0.3230,0.2720}, {0.0024,0.4073,0.2123}, {0.0093,0.5030,0.1582},
        {0.0291,0.6082,0.1117}, {0.0633,0.7100,0.0782}, {0.1096,0.7932,0.0573},
        {0.1655,0.8620,0.0422}, {0.2257,0.9149,0.0298}, {0.2904,0.9540,0.0203},
        {0.3597,0.9803,0.0134}, {0.4334,0.9950,0.0087}, {0.5121,1.0000,0.0057},
        {0.5945,0.9950,0.0039}, {0.6784,0.9786,0.0027}, {0.7621,0.9520,0.0021},
        {0.8425,0.9154,0.0018}, {0.9163,0.8700,0.0017}, {0.9786,0.8163,0.0014},
        {1.0263,0.7570,0.0011}, {1.0567,0.6949,0.0010}, {1.0622,0.6310,0.0008},
        {1.0456,0.5668,0.0006}, {1.0026,0.5030,0.0003}, {0.9384,0.4412,0.0002},
        {0.8544,0.3810,0.0002}, {0.7514,0.3210,0.0001}, {0.6424,0.2650,0.0000},
        {0.5419,0.2170,0.0000}, {0.4479,0.1750,0.0000}, {0.3608,0.1382,0.0000},
        {0.2835,0.1070,0.0000}, {0.2187,0.0816,0.0000}, {0.1649,0.0610,0.0000},
        {0.1212,0.0446,0.0000}, {0.0874,0.0320,0.0000}, {0.0636,0.0232,0.0000},
        {0.0468,0.0170,0.0000}, {0.0329,0.0119,0.0000}, {0.0227,0.0082,0.0000},
        {0.0158,0.0057,0.0000}, {0.0114,0.0041,0.0000}, {0.0081,0.0029,0.0000},
        {0.0058,0.0021,0.0000}, {0.0041,0.0015,0.0000}, {0.0029,0.0010,0.0000},
        {0.0020,0.0007,0.0000}, {0.0014,0.0005,0.0000}, {0.0010,0.0004,0.0000},
        {0.0007,0.0002,0.0000}, {0.0005,0.0002,0.0000}, {0.0003,0.0001,0.0000},
        {0.0002,0.0001,0.0000}, {0.0002,0.0001,0.0000}, {0.0001,0.0000,0.0000},
        {0.0001,0.0000,0.0000}, {0.0001,0.0000,0.0000}, {0.0000,0.0000,0.0000}
    };
    
    for (int i=0, lambda=380; lambda<780.1; i++,lambda+=5)
    {
        GLfloat Me;
        Me = bbSpectrum(lambda, temperature);
        X += Me * cie_colour_match[i][0];
        Y += Me * cie_colour_match[i][1];
        Z += Me * cie_colour_match[i][2];
    }
    
    XYZ = (X + Y + Z);
    x = X / XYZ;
    y = Y / XYZ;
    z = Z / XYZ;
}

void Color::xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b, ColorSystem cs)
{
    GLfloat xr, yr, zr, xg, yg, zg, xb, yb, zb;
    GLfloat xw, yw, zw;
    GLfloat rx, ry, rz, gx, gy, gz, bx, by, bz;
    GLfloat rw, gw, bw;
    
    xr = cs.xRed;    yr = cs.yRed;    zr = 1 - (xr + yr);
    xg = cs.xGreen;  yg = cs.yGreen;  zg = 1 - (xg + yg);
    xb = cs.xBlue;   yb = cs.yBlue;   zb = 1 - (xb + yb);
    
    xw = cs.xWhite;  yw = cs.yWhite;  zw = 1 - (xw + yw);
    
    /* xyz -> rgb matrix, before scaling to white. */
    rx = (yg * zb) - (yb * zg);  ry = (xb * zg) - (xg * zb);  rz = (xg * yb) - (xb * yg);
    gx = (yb * zr) - (yr * zb);  gy = (xr * zb) - (xb * zr);  gz = (xb * yr) - (xr * yb);
    bx = (yr * zg) - (yg * zr);  by = (xg * zr) - (xr * zg);  bz = (xr * yg) - (xg * yr);
    
    /* White scaling factors.
     Dividing by yw scales the white luminance to unity, as conventional. */
    rw = ((rx * xw) + (ry * yw) + (rz * zw)) / yw;
    gw = ((gx * xw) + (gy * yw) + (gz * zw)) / yw;
    bw = ((bx * xw) + (by * yw) + (bz * zw)) / yw;
    
    /* xyz -> rgb matrix, correctly scaled to white. */
    rx = rx / rw;  ry = ry / rw;  rz = rz / rw;
    gx = gx / gw;  gy = gy / gw;  gz = gz / gw;
    bx = bx / bw;  by = by / bw;  bz = bz / bw;
    
    /* rgb of the desired point */
    r = (rx * x) + (ry * y) + (rz * z);
    g = (gx * x) + (gy * y) + (gz * z);
    b = (bx * x) + (by * y) + (bz * z);
    
    /* constrain rgb */
    GLfloat w;
    w = (0 < r) ? 0 : r;
    w = (w < g) ? w : g;
    w = (w < b) ? w : b;
    w = -w;
    
    /* add just enough white to make r, g, b all positive. */
    if(w > 0)
    {
        r += w;
        g += w;
        b += w;
    }
    
    /* normalize rgb */
    GLfloat maximum = Max(r, Max(g,b));
    if(maximum > 1.f)
    {
        r /= maximum;
        g /= maximum;
        b /= maximum;
    }
}

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
    
glm::mat4 glMatrixFromTransform(const Transform& T)
{
#ifdef BT_USE_DOUBLE_PRECISION
    Scalar glmatrix[16];
    T.getOpenGLMatrix(glmatrix);
    glm::mat4 M((GLfloat)glmatrix[0],(GLfloat)glmatrix[1],(GLfloat)glmatrix[2],(GLfloat)glmatrix[3],
                (GLfloat)glmatrix[4],(GLfloat)glmatrix[5],(GLfloat)glmatrix[6],(GLfloat)glmatrix[7],
                (GLfloat)glmatrix[8],(GLfloat)glmatrix[9],(GLfloat)glmatrix[10],(GLfloat)glmatrix[11],
                (GLfloat)glmatrix[12],(GLfloat)glmatrix[13],(GLfloat)glmatrix[14],(GLfloat)glmatrix[15]);
#else
    GLfloat glmatrix[16];
    T.getOpenGLMatrix(glmatrix);
    glm::mat4 M = glm::make_mat4(glmatrix);
    //OR
    //glm::mat4 M;
    //T.getOpenGLMatrix(&M[0].x);
#endif
    return M;
}

}
