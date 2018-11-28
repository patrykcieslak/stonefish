//
//  Pipe.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pipe__
#define __Stonefish_Pipe__

#include "entities/forcefields/VelocityField.h"

namespace sf
{

//! Pipe (current) velocity field class.
/*!
	Class implements a velocity field in a shape of a simple tube with variable diameter.
    The flow velocity is specified at the centre of the beginning of the tube. 
    The closer to the boundary the slower the flow (zero at boudary).
 */
class Pipe : public VelocityField
{
public:
    Pipe(const Vector3& point1, const Vector3& point2, Scalar radius1, Scalar radius2, Scalar inletVelocity, Scalar exponent);
    
    Vector3 GetVelocityAtPoint(const Vector3& p);
    std::vector<Renderable> Render();    

private:
    Vector3 p1, n;
    Scalar r1, r2, l;
    Scalar vin;
    Scalar gamma;
};
    
}

#endif
