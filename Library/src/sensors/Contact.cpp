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
//  Contact.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/Contact.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "entities/SolidEntity.h"

namespace sf
{
    
Contact::Contact(std::string uniqueName, Entity* entityA, Entity* entityB, unsigned int inclusiveHistoryLength)
{
    name_ = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    A_ = entityA;
    B_ = entityB;
    historyLen_ = inclusiveHistoryLength;
    displayMask_ = CONTACT_DISPLAY_NONE;
    newDataAvailable_ = false;
    pointsLastBeg_ = points_.begin();
}

Contact::~Contact()
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name_);
}

std::string Contact::getName() const
{
    return name_;
}

const Entity* Contact::getEntityA()
{
    return A_;
}

const Entity* Contact::getEntityB()
{
    return B_;
}

void Contact::setDisplayMask(int16_t mask)
{
    displayMask_ = mask;
}

void Contact::MarkDataOld()
{
    newDataAvailable_ = false;
}

bool Contact::isNewDataAvailable() const
{
    return newDataAvailable_;
}

void Contact::AddContactPoint(const btPersistentManifold* manifold, bool swapped, Scalar dt)
{
    size_t added = 0;
    for(int i=0; i<manifold->getNumContacts(); ++i)
    {
        const btManifoldPoint& mp = manifold->getContactPoint(i);
        Vector3 locationA = swapped ? mp.getPositionWorldOnB() : mp.getPositionWorldOnA();
        Vector3 normalForceA = (swapped ? Scalar(1.) : Scalar(-1.)) * mp.m_normalWorldOnB * mp.getAppliedImpulse() / dt;

        //Filtering
        if(points_.size() > 0
           && (locationA - points_.back().locationA).length2() < (Scalar(0.001)*Scalar(0.001)) //Closer than 1 mm from the last point
           && (normalForceA - points_.back().normalForceA).fuzzyZero())  //No change in normal force
            continue;

        ContactPoint p;
        p.locationA = locationA;
        p.locationB = swapped ? mp.getPositionWorldOnA() : mp.getPositionWorldOnB();
        ContactInfo* cInfo = (ContactInfo*)mp.m_userPersistentData;
        p.slippingVelocityA = (swapped ? Scalar(-1.) : Scalar(1.)) * cInfo->slip;
        p.normalForceA = normalForceA;
        AddContactPoint(p);
        ++added;
    }
    pointsLastBeg_ = points_.end()-added; // Save iterator to the beginning of new points
}

void Contact::AddContactPoint(ContactPoint p)
{
    //historyLen = 0 means "full history"
    if(historyLen_ > 0 && points_.size() == historyLen_)
        points_.pop_front();

    p.timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
    points_.push_back(p);
    
    newDataAvailable_ = true;
}

void Contact::ClearHistory()
{
    points_.clear();
}

const std::deque<ContactPoint>& Contact::getHistory()
{
    return points_;
}

std::vector<Renderable> Contact::Render()
{
    std::vector<Renderable> items(0);
    
    if(points_.size() == 0)
        return items;
    
    //Drawing points
    /*if(displayMask & CONTACT_DISPLAY_LAST_A)
        vertices.push_back(glm::vec3((GLfloat)points.back().locationA.getX(), (GLfloat)points.back().locationA.getY(), (GLfloat)points.back().locationA.getZ()));
    
    if(displayMask & CONTACT_DISPLAY_LAST_B)
        vertices.push_back(glm::vec3((GLfloat)points.back().locationB.getX(), (GLfloat)points.back().locationB.getY(), (GLfloat)points.back().locationB.getZ()));
    
    OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::POINTS, vertices, CONTACT_COLOR);*/
    
    //Drawing lines
    std::vector<glm::vec3> vertices;
    
    if(displayMask_ & CONTACT_DISPLAY_LAST_SLIP_VELOCITY_A)
    {
        Vector3 p1 = points_.back().locationA;
        Vector3 p2 = points_.back().locationA + points_.back().slippingVelocityA;
        vertices.push_back(glm::vec3((GLfloat)p1.getX(), (GLfloat)p1.getY(), (GLfloat)p1.getZ()));
        vertices.push_back(glm::vec3((GLfloat)p2.getX(), (GLfloat)p2.getY(), (GLfloat)p2.getZ()));
    }
    
    if(displayMask_ & CONTACT_DISPLAY_LAST_SLIP_VELOCITY_B)
    {
        Vector3 p1 = points_.back().locationB;
        Vector3 p2 = points_.back().locationB - points_.back().slippingVelocityA;
        vertices.push_back(glm::vec3((GLfloat)p1.getX(), (GLfloat)p1.getY(), (GLfloat)p1.getZ()));
        vertices.push_back(glm::vec3((GLfloat)p2.getX(), (GLfloat)p2.getY(), (GLfloat)p2.getZ()));
    }
    
    if(displayMask_ & CONTACT_DISPLAY_NORMAL_FORCE_A)
    {
        for(auto it=pointsLastBeg_; it != points_.end(); ++it)
        {
            Vector3 p1 = (*it).locationA;
            Vector3 p2 = (*it).locationA + (*it).normalForceA;
            vertices.push_back(glm::vec3((GLfloat)p1.getX(), (GLfloat)p1.getY(), (GLfloat)p1.getZ()));
            vertices.push_back(glm::vec3((GLfloat)p2.getX(), (GLfloat)p2.getY(), (GLfloat)p2.getZ()));
        }
    }
    
    if(displayMask_ & CONTACT_DISPLAY_NORMAL_FORCE_B)
    {
        for(auto it=pointsLastBeg_; it != points_.end(); ++it)
        {
            Vector3 p1 = (*it).locationB;
            Vector3 p2 = (*it).locationB - (*it).normalForceA;
            vertices.push_back(glm::vec3((GLfloat)p1.getX(), (GLfloat)p1.getY(), (GLfloat)p1.getZ()));
            vertices.push_back(glm::vec3((GLfloat)p2.getX(), (GLfloat)p2.getY(), (GLfloat)p2.getZ()));
        }
    }
    
    if(vertices.size() > 0)
    {
        Renderable item;
        item.model = glm::mat4(1.f);
        item.type = RenderableType::SENSOR_LINES;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        *item.getDataAsPoints() = vertices;
        items.push_back(item);
    }
        
    //Drawing line strips
    if(displayMask_ & CONTACT_DISPLAY_PATH_A)
    {
        Renderable item;
        item.model = glm::mat4(1.f);
        item.type = RenderableType::SENSOR_POINTS;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto itemPoints = item.getDataAsPoints();
        
        for(size_t i = 0; i < points_.size(); ++i)
        {	
            Vector3 p = points_[i].locationA;
            itemPoints->push_back(glm::vec3((GLfloat)p.getX(), (GLfloat)p.getY(), (GLfloat)p.getZ()));
        }
        
        items.push_back(item);
    }
    
    if(displayMask_ & CONTACT_DISPLAY_PATH_B)
    {
        Renderable item;
        item.model = glm::mat4(1.f);
        item.type = RenderableType::SENSOR_POINTS;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto itemPoints = item.getDataAsPoints();
        
        for(size_t i = 0; i < points_.size(); ++i)
        {	
            Vector3 p = points_[i].locationB;
            itemPoints->push_back(glm::vec3((GLfloat)p.getX(), (GLfloat)p.getY(), (GLfloat)p.getZ()));
        }
        
        items.push_back(item);
    }
    
    return items;
}

}
