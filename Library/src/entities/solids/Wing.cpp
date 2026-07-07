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
//  Wing.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 17/01/2019.
//  Copyright (c) 2019-2021 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Wing.h"

#include "graphics/OpenGLContent.h"
#include "utils/GeometryFileUtil.h"

namespace sf
{

Wing::Wing(std::string uniqueName, PhysicsSettings phy, Scalar baseChordLength, Scalar tipChordLength,
           Scalar maxCamber, Scalar maxCamberPos, Scalar profileThickness, Scalar wingLength, const Transform& origin, 
           std::string material, std::string look, Scalar thickness)
           : SolidEntity(uniqueName, phy, material, look, thickness)
{
    T_O2G_ = T_O2C_ = origin;
    baseChordLength = baseChordLength <= Scalar(0) ? Scalar(1) : baseChordLength;
    tipChordLength = tipChordLength < Scalar(0) ? baseChordLength : tipChordLength;
    maxCamber = maxCamber < Scalar(0) ? Scalar(0) : (maxCamber > Scalar(100) ? Scalar(100) : maxCamber);
    maxCamberPos = maxCamberPos < Scalar(0) ? Scalar(0) : (maxCamberPos > Scalar(100) ? Scalar(100) : maxCamberPos);
    profileThickness = profileThickness < Scalar(1) ? Scalar(1) : (profileThickness > Scalar(40) ? Scalar(40) : profileThickness);
    wingLength = wingLength < Scalar(0) ? Scalar(0) : wingLength;
    
    //1. Build wing geometry
    phyMesh_ = OpenGLContent::BuildWing((GLfloat)baseChordLength, (GLfloat)tipChordLength, (GLfloat)maxCamber, (GLfloat)maxCamberPos,
                                       (GLfloat)profileThickness, (GLfloat)wingLength);
    
    //2. Compute physical properties
    Vector3 CG;
    Matrix3 Irot;
    ComputePhysicalProperties(phyMesh_, thickness, mat_.density, mass_, CG, volume_, surface_, Ipri_, Irot);
    T_CG2C_.setOrigin(-CG); //Set CG position
    T_CG2C_ = Transform(Irot, Vector3(0,0,0)).inverse() * T_CG2C_; //Align CG frame to principal axes of inertia
    T_CG2O_ = T_CG2C_ * T_O2C_.inverse();
    T_CG2G_ = T_CG2O_ * T_O2G_;
    
    //3. Compute hydrodynamic properties
    ComputeFluidDynamicsApprox(GeometryApproxType::ELLIPSOID);
    T_O2H_ = T_CG2O_.inverse() * T_CG2H_;
    P_CB_ = Vector3(0,0,0);
}
    
Wing::Wing(std::string uniqueName, PhysicsSettings phy, Scalar baseChordLength, Scalar tipChordLength, std::string NACA, Scalar wingLength, const Transform& origin, 
           std::string material, std::string look, Scalar thickness)
           : SolidEntity(uniqueName, phy, material, look, thickness)
{
    T_O2G_ = T_O2C_ = origin;
    baseChordLength = baseChordLength <= Scalar(0) ? Scalar(1) : baseChordLength;
    tipChordLength = tipChordLength < Scalar(0) ? baseChordLength : tipChordLength;
    wingLength = wingLength < Scalar(0) ? Scalar(0) : wingLength;
    
    Scalar maxCamber(0);
    Scalar maxCamberPos(0);
    Scalar profileThickness(20);
    
    //1. Decode NACA
    if(NACA.length() == 4)
    {
        int nacaCode[4];
        int i;
        
        for(i=0; i<4; ++i)
            if(NACA[i] >= '0' && NACA[i] <= '9')
                nacaCode[i] = NACA[i] - '0';
            else
                break;
        
        if(i==4) //Decode
        {
            maxCamber = (Scalar)nacaCode[0];
            maxCamberPos = (Scalar)nacaCode[1] * 10.0;
            profileThickness = (Scalar)nacaCode[2] * 10.0 + (Scalar)nacaCode[3];
        }
    }
    
    //2. Build wing geometry
    phyMesh_ = OpenGLContent::BuildWing((GLfloat)baseChordLength, (GLfloat)tipChordLength, (GLfloat)maxCamber, (GLfloat)maxCamberPos,
                                       (GLfloat)profileThickness, (GLfloat)wingLength);
    
    
    //3. Compute physical properties
    Vector3 CG;
    Matrix3 Irot;
    ComputePhysicalProperties(phyMesh_, thickness, mat_.density, mass_, CG, volume_, surface_, Ipri_, Irot);
    T_CG2C_.setOrigin(-CG); //Set CG position
    T_CG2C_ = Transform(Irot, Vector3(0,0,0)).inverse() * T_CG2C_; //Align CG frame to principal axes of inertia
    T_CG2O_ = T_CG2C_ * T_O2C_.inverse();
    T_CG2G_ = T_CG2O_ * T_O2G_;

    //4. Compute hydrodynamic properties
    ComputeFluidDynamicsApprox(GeometryApproxType::ELLIPSOID);
    T_O2H_ = T_CG2O_.inverse() * T_CG2H_;
    P_CB_ = Vector3(0,0,0);
}
    
SolidType Wing::getSolidType()
{
    return SolidType::WING;
}
    
btCollisionShape* Wing::BuildCollisionShape()
{
    btConvexHullShape* convex = new btConvexHullShape();
    for(size_t i=0; i<phyMesh_->getNumOfVertices(); ++i)
    {
        glm::vec3 pos = phyMesh_->getVertexPos(i);
        Vector3 v(pos.x, pos.y, pos.z);
        convex->addPoint(v);
    }
    //convex->optimizeConvexHull();
    convex->setMargin(0);
    return convex;
}

}
