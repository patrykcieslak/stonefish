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
//  USBL.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "comms/USBL.h"

namespace sf
{
    
std::random_device USBL::randomDevice;
std::mt19937 USBL::randomGenerator(randomDevice());
    
USBL::USBL(std::string uniqueName, uint64_t deviceId, 
           Scalar horizontalFOVDeg, Scalar verticalFOVDeg, Scalar operatingRange, bool hasGPS, bool hasPressureSensor, 
           Scalar frequency) : AcousticModem(uniqueName, deviceId, horizontalFOVDeg, verticalFOVDeg, operatingRange, frequency)
{
    pressure = hasPressureSensor;
    gps = hasGPS;
    noise = false;
}
    
void USBL::setNoise(Scalar rangeDev, Scalar angleDevDeg, Scalar nedDev, Scalar depthDev)
{
    noiseRange = std::normal_distribution<Scalar>(Scalar(0), fabs(rangeDev));
    noiseAngle = std::normal_distribution<Scalar>(Scalar(0), fabs(angleDevDeg)/Scalar(180)*M_PI);
    noiseNED = std::normal_distribution<Scalar>(Scalar(0), fabs(nedDev));
    noiseDepth = std::normal_distribution<Scalar>(Scalar(0), fabs(depthDev));
    noise = true;
}
    
void USBL::InternalUpdate(Scalar dt)
{
    AcousticModem::InternalUpdate(dt);
    
    if(isConnectionAlive())
    {
        AcousticModem* cNode = getNode(getConnectedId());
        
        //Get range and direction
        Transform cTrans = cNode->getDeviceFrame();
        Transform trans = getDeviceFrame();
        Vector3 dir = cTrans.getOrigin() - trans.getOrigin(); //In world frame
        Scalar distance = dir.length();
        dir = (trans.inverse() * dir).normalized(); //In device frame
        
        //Find ranging angles
        Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).length();
        Scalar vAngle = atan2(d, dir.getZ());
        Scalar hAngle = atan2(dir.getY(), dir.getX());
        
        //Find position of receiver
        Vector3 pos;
        
        if(noise)
        {
            trans.getOrigin().setX(trans.getOrigin().getX() + noiseNED(randomGenerator));
            trans.getOrigin().setY(trans.getOrigin().getY() + noiseNED(randomGenerator));
            distance += noiseRange(randomGenerator);
            vAngle += noiseAngle(randomGenerator);
            hAngle += noiseAngle(randomGenerator);
            
            Vector3 corruptedDir;
            corruptedDir.setX(btCos(hAngle) * btSin(vAngle));
            corruptedDir.setY(btSin(hAngle) * btSin(vAngle));
            corruptedDir.setZ(btCos(vAngle));
            corruptedDir.normalize();
            
            pos = corruptedDir * distance;
        }
        else 
            pos = dir * distance;
        
        if(gps)
            cNode->UpdatePosition(trans * pos, true);
        else
            cNode->UpdatePosition(pos, false, getName());
    }
    
    //Connect(...)
}

}