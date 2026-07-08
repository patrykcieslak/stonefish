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
//  Jet.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Jet.h"

namespace sf
{

Jet::Jet(const Vector3& point, const Vector3& direction, Scalar radius, Scalar outletVelocity)
{
    c_ = point;
    n_ = direction.normalized();
    r_ = radius;
    setOutletVelocity(outletVelocity);
}

void Jet::setOutletVelocity(Scalar x)
{
    vout_ = x;
}

Scalar Jet::getOutletVelocity() const
{
    return vout_;
}

VelocityFieldType Jet::getType() const
{
    return VelocityFieldType::JET;
}

Vector3 Jet::GetVelocityAtPoint(const Vector3& p) const
{
    //Calculate distance to axis
    Vector3 cp = p-c_;
    Scalar d = cp.cross(n_).norm();
    
    //Calculate distance from outlet
    Scalar t = cp.dot(n_);
    if(t < 0.0) return Vector3(0,0,0);
    
    //Calculate radius at point
    Scalar r = Scalar(1)/Scalar(5)*(t + Scalar(5)*r_); //Jet angle is around 24 deg independent of conditions!
    if(d >= r) return Vector3(0,0,0);
    
    //Calculate central velocity
    Vector3 vmax = Scalar(10)*r_/(t + Scalar(5)*r_) * vout_ * n_;  
    
    //Calculate fraction of central velocity
    Scalar f = btExp(-Scalar(50)*d*d/(t*t));
    
    return f*vmax;
}

std::vector<Renderable> Jet::Render(VelocityFieldUBO& ubo)
{
    std::vector<Renderable> items(0);
    ubo.posR = glm::vec4((GLfloat)c_.getX(), (GLfloat)c_.getY(), (GLfloat)c_.getZ(), (GLfloat)r_);
    ubo.dirV = glm::vec4((GLfloat)n_.getX(), (GLfloat)n_.getY(), (GLfloat)n_.getZ(), (GLfloat)vout_);
    ubo.params = glm::vec3(0.f);
    ubo.type = 1;

    //Model matrix
    glm::vec3 z_(n_.x(), n_.y(), n_.z());
    glm::vec3 x_(-n_.y(), n_.x(), n_.z());
    glm::vec3 y_ = glm::cross(z_,x_);
    glm::mat4 model(glm::vec4(x_, 0.f), glm::vec4(y_, 0.f), glm::vec4(z_, 0.f), glm::vec4(c_.x(), c_.y(), c_.z(), 1));    
    
    //Orifice
    Renderable orifice;
    orifice.type = RenderableType::HYDRO_LINE_STRIP;
    orifice.model = model;
    orifice.data = std::make_shared<std::vector<glm::vec3>>();
    auto orificePoints = orifice.getDataAsPoints();
    
    for(unsigned int i=0; i<=12; ++i)
    {
        Scalar alpha = Scalar(i)/Scalar(12) * M_PI * Scalar(2);
        Vector3 v(btCos(alpha)*r_, btSin(alpha)*r_, 0);
        orificePoints->push_back(glm::vec3(v.x(), v.y(), v.z()));
    }
    
    //Cone
    Renderable cone;
    cone.type = RenderableType::HYDRO_LINES;
    cone.model = orifice.model;
    cone.data = std::make_shared<std::vector<glm::vec3>>();
    auto conePoints = cone.getDataAsPoints();
    conePoints->push_back(glm::vec3(0, 0, 0));
    conePoints->push_back(glm::vec3(0, 0, vout_));
    
    Scalar r = Scalar(1)/Scalar(5)*(Scalar(10)*r_ + Scalar(5)*r_);
    
    for(unsigned int i=0; i<12; ++i)
    {
        Scalar alpha = Scalar(i)/Scalar(12) * M_PI * Scalar(2);
        Vector3 v1(btCos(alpha)*r_, btSin(alpha)*r_, 0);
        Vector3 v2(v1.x()*r/r_, v1.y()*r/r_, Scalar(10)*r_);
        conePoints->push_back(glm::vec3(v1.x(), v1.y(), v1.z()));
        conePoints->push_back(glm::vec3(v2.x(), v2.y(), v2.z()));
    }
    
    //Build
    items.push_back(orifice);
    items.push_back(cone);

    return items;
}

}
