//
//  Jet.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Jet__
#define __Stonefish_Jet__

#include "entities/forcefields/VelocityField.h"

namespace sf
{

//! Jet velocity field class.
/*!
	Class implements a velocity field coming from a water jet.
    The flow velocity is specified at the centre of the jet outlet.
    The closer to the outlet boundary the slower the flow (zero at boudary).
 */
class Jet : public VelocityField
{
public:
    Jet(const Vector3& point, const Vector3& direction, Scalar radius, Scalar outletVelocity);
    
    Vector3 GetVelocityAtPoint(const Vector3& p);
    std::vector<Renderable> Render();    

private:
    Vector3 c, n;
    Scalar r;
    Scalar vout;
};

}

#endif
