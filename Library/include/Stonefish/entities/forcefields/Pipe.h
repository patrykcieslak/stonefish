//
//  Pipe.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pipe__
#define __Stonefish_Pipe__

#include "VelocityField.h"

//! Pipe (current) velocity field class.
/*!
	Class implements a velocity field in a shape of a simple tube with variable diameter.
    The flow velocity is specified at the centre of the beginning of the tube. 
    The closer to the boundary the slower the flow (zero at boudary).
 */
class Pipe : public VelocityField
{
public:
    Pipe(const btVector3& point1, const btVector3& point2, btScalar radius1, btScalar radius2, btScalar inletVelocity, btScalar exponent);
    
    btVector3 GetVelocityAtPoint(const btVector3& p);
    std::vector<Renderable> Render();    

private:
    btVector3 p1, n;
    btScalar r1, r2, l;
    btScalar vin;
    btScalar gamma;
};

#endif