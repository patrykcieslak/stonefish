//
//  Stream.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Stream__
#define __Stonefish_Stream__

#include "entities/forcefields/VelocityField.h"

//! Stream (current) velocity field class.
/*!
	Class implements a velocity field in a shape of a tube along a Hermite spline, with variable diameter.
    The flow velocity is specified at the centre of the beginning of the tube (tanget to spline). 
    The closer to the boundary the slower the flow (zero at boudary).
 */
class Stream : public VelocityField
{
public:
    Stream(const std::vector<btVector3>& streamline, const std::vector<btScalar>& radius, btScalar inputVelocity, btScalar exponent);
    
    btVector3 GetVelocityAtPoint(const btVector3& p);
    std::vector<Renderable> Render();
    
private:
    std::vector<btVector3> c;
    std::vector<btScalar> r;
    btScalar vin;
    btScalar gamma;
};

#endif
