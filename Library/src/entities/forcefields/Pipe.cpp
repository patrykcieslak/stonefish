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
//  Pipe.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Pipe.h"

namespace sf
{

Pipe::Pipe(const Vector3& point1, const Vector3& point2, Scalar radius1, Scalar radius2, Scalar inletVelocity, Scalar exponent)
{
    p1 = point1;
    l = (point2-point1).length();
    n = (point2-point1)/l;
    r1 = radius1;
    r2 = radius2;
    vin = inletVelocity;
    gamma = exponent;
}

VelocityFieldType Pipe::getType() const
{
    return VelocityFieldType::PIPE;
}

Vector3 Pipe::GetVelocityAtPoint(const Vector3& p) const
{
    //Calculate distance to line
    Vector3 p1p = p-p1;
    Scalar d = p1p.cross(n).norm();
    
    //Calculate closest point on line section between P1 and P2
    Scalar t = p1p.dot(n);
    if(t < 0.0 || t > l) return Vector3(0,0,0);
    
    //Calculate radius at point
    Scalar r = r1 + (r2-r1) * t/l;
    if(d >= r) return Vector3(0,0,0);
    
    //Calculate central velocity
    Vector3 v = r1/r * vin * n;
    
    //Calculate fraction of central velocity
    Scalar f = btPow(Scalar(1)-d/r, gamma);
    
    return f*v;
}

std::vector<Renderable> Pipe::Render(VelocityFieldUBO& ubo)
{
    std::vector<Renderable> items(0);
    ubo.posR = glm::vec4((GLfloat)p1.getX(), (GLfloat)p1.getY(), (GLfloat)p1.getZ(), (GLfloat)r1);
    ubo.dirV = glm::vec4((GLfloat)n.getX(), (GLfloat)n.getY(), (GLfloat)n.getZ(), (GLfloat)vin);
    ubo.params = glm::vec3((GLfloat)l, (GLfloat)r2, (GLfloat)gamma);
    ubo.type = 2;

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
        Scalar alpha = Scalar(i)/Scalar(12) * M_PI * Scalar(2);
        Vector3 v1(btCos(alpha)*r1, btSin(alpha)*r1, 0);
        Vector3 v2(v1.x()*r2/r1, v1.y()*r2/r1, l);
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

}
