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
//  Copyright(c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Pipe.h"

namespace sf
{

Pipe::Pipe(const Vector3& point1, const Vector3& point2, Scalar radius1, Scalar radius2, Scalar inletVelocity, Scalar exponent)
{
    p1_ = point1;
    l_ = (point2-point1).length();
    n_ = (point2-point1)/l_;
    r1_ = radius1;
    r2_ = radius2;
    vin_ = inletVelocity;
    gamma_ = exponent;
}

VelocityFieldType Pipe::getType() const
{
    return VelocityFieldType::PIPE;
}

void Pipe::setInletVelocity(Scalar v)
{
    vin_ = v;
}

Scalar Pipe::getInletVelocity() const
{
    return vin_;
}

Vector3 Pipe::GetVelocityAtPoint(const Vector3& p) const
{
    //Calculate distance to line
    Vector3 p1p = p-p1_;
    Scalar d = p1p.cross(n_).norm();
    
    //Calculate closest point on line section between P1 and P2
    Scalar t = p1p.dot(n_);
    if(t < 0.0 || t > l_) return Vector3(0,0,0);
    
    //Calculate radius at point
    Scalar r = r1_ + (r2_-r1_) * t/l_;
    if(d >= r) return Vector3(0,0,0);
    
    //Calculate central velocity
    Vector3 v = r1_/r * vin_ * n_;
    
    //Calculate fraction of central velocity
    Scalar f = btPow(Scalar(1)-d/r, gamma_);
    
    return f*v;
}

std::vector<Renderable> Pipe::Render(VelocityFieldUBO& ubo)
{
    std::vector<Renderable> items(0);
    ubo.posR = glm::vec4((GLfloat)p1_.getX(), (GLfloat)p1_.getY(), (GLfloat)p1_.getZ(), (GLfloat)r1_);
    ubo.dirV = glm::vec4((GLfloat)n_.getX(), (GLfloat)n_.getY(), (GLfloat)n_.getZ(), (GLfloat)vin_);
    ubo.params = glm::vec3((GLfloat)l_, (GLfloat)r2_, (GLfloat)gamma_);
    ubo.type = 2;

    //Model matrix
    glm::vec3 z_(n_.x(), n_.y(), n_.z());
    glm::vec3 x_(n_.y(), n_.x(), n_.z());
    glm::vec3 y_ = glm::cross(z_,x_);
    glm::mat4 model(glm::vec4(x_, 0.f), glm::vec4(y_, 0.f), glm::vec4(z_, 0.f), glm::vec4(p1_.x(), p1_.y(), p1_.z(), 1));    
    
    //Inlet and outlet
    Renderable inlet;
    inlet.type = RenderableType::HYDRO_LINE_STRIP;
    inlet.model = model;
    inlet.data = std::make_shared<std::vector<glm::vec3>>();
    auto inletPoints = inlet.getDataAsPoints();
    
    Renderable outlet;
    outlet.type = RenderableType::HYDRO_LINE_STRIP;
    outlet.model = model;
    outlet.data = std::make_shared<std::vector<glm::vec3>>();
    auto outletPoints = outlet.getDataAsPoints();
    
    //Pipe
    Renderable pipe;
    pipe.type = RenderableType::HYDRO_LINES;
    pipe.model = model;
    pipe.data = std::make_shared<std::vector<glm::vec3>>();
    auto pipePoints = pipe.getDataAsPoints();
    
    pipePoints->push_back(glm::vec3(0, 0, 0));
    pipePoints->push_back(glm::vec3(0, 0, l_));
    
    for(unsigned int i=0; i<12; ++i)
    {
        Scalar alpha = Scalar(i)/Scalar(12) * M_PI * Scalar(2);
        Vector3 v1(btCos(alpha)*r1_, btSin(alpha)*r1_, 0);
        Vector3 v2(v1.x()*r2_/r1_, v1.y()*r2_/r1_, l_);
        pipePoints->push_back(glm::vec3(v1.x(), v1.y(), v1.z()));
        inletPoints->push_back(pipePoints->back());
        pipePoints->push_back(glm::vec3(v2.x(), v2.y(), v2.z()));
        outletPoints->push_back(pipePoints->back());
    }
    
    inletPoints->push_back(inletPoints->front());
    outletPoints->push_back(outletPoints->front());
    
    items.push_back(inlet);
    items.push_back(outlet);
    items.push_back(pipe);
    return items;
}

}
