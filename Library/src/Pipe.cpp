//
//  Pipe.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include <entities/forcefields/Pipe.h>

Pipe::Pipe(const btVector3& point1, const btVector3& point2, btScalar radius1, btScalar radius2, btScalar inletVelocity, btScalar exponent)
{
    p1 = point1;
    l = (point2-point1).length();
    n = (point2-point1)/l;
    r1 = radius1;
    r2 = radius2;
    vin = inletVelocity;
    gamma = exponent;
}

btVector3 Pipe::GetVelocityAtPoint(const btVector3& p)
{
    //Calculate distance to line
    btVector3 p1p = p-p1;
    btScalar d = p1p.cross(n).norm();
    
    //Calculate closest point on line section between P1 and P2
    btScalar t = p1p.dot(n);
    if(t < 0.0 || t > l) return btVector3(0,0,0);
    
    //Calculate radius at point
    btScalar r = r1 + (r2-r1) * t/l;
    if(d >= r) return btVector3(0,0,0);
    
    //Calculate central velocity
    btVector3 v = r1/r * vin * n;
    
    //Calculate fraction of central velocity
    btScalar f = btPow(btScalar(1)-d/r, gamma);
    
    return f*v;
}

std::vector<Renderable> Pipe::Render()
{
    std::vector<Renderable> items(0);
    
    //Model matrix
    glm::vec3 z_(n.x(), n.y(), n.z());
    glm::vec3 x_(n.y(), n.x(), n.z());
    glm::vec3 y_ = glm::cross(z_,x_);
    glm::mat4 model(glm::vec4(x_, 0.f), glm::vec4(y_, 0.f), glm::vec4(z_, 0.f), glm::vec4(p1.x(), p1.y(), p1.z(), 1));    
    
    //Inlet and outlet
    Renderable inlet;
    inlet.type = RenderableType::HYDRO_LINE_STRIP;
    inlet.model = model;
    
    Renderable outlet;
    outlet.type = RenderableType::HYDRO_LINE_STRIP;
    outlet.model = model;

    //Pipe
    Renderable pipe;
    pipe.type = RenderableType::HYDRO_LINES;
    pipe.model = model;
    pipe.points.push_back(glm::vec3(0, 0, 0));
    pipe.points.push_back(glm::vec3(0, 0, l));
    
    for(unsigned int i=0; i<12; ++i)
    {
        btScalar alpha = btScalar(i)/btScalar(12) * M_PI * btScalar(2);
        btVector3 v1(btCos(alpha)*r1, btSin(alpha)*r1, 0);
        btVector3 v2(v1.x()*r2/r1, v1.y()*r2/r1, l);
        pipe.points.push_back(glm::vec3(v1.x(), v1.y(), v1.z()));
        inlet.points.push_back(pipe.points.back());
        pipe.points.push_back(glm::vec3(v2.x(), v2.y(), v2.z()));
        outlet.points.push_back(pipe.points.back());
    }
    
    inlet.points.push_back(inlet.points.front());
    outlet.points.push_back(outlet.points.front());
    
    items.push_back(inlet);
    items.push_back(outlet);
    items.push_back(pipe);
    return items;
}