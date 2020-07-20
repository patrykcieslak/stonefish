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
//  StonefishCommon.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/01/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_StonefishCommon__
#define __Stonefish_StonefishCommon__

//STL
#include <string>
#include <vector>

//Bullet Physics
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>

//Stonefish
#include <version.h>

namespace sf
{
    typedef btScalar Scalar;
    typedef btQuaternion Quaternion;
    typedef btVector3 Vector3;
    typedef btMatrix3x3 Matrix3;
    typedef btTransform Transform;
    
    inline Transform I4() { return Transform::getIdentity(); }
    inline Matrix3 I3() { return Matrix3::getIdentity(); }
    inline Quaternion IQ() { return Quaternion::getIdentity(); }
    inline Vector3 V0() { return Vector3(0,0,0); }
    inline Vector3 VX() { return Vector3(1,0,0); }
    inline Vector3 VY() { return Vector3(0,1,0); }
    inline Vector3 VZ() { return Vector3(0,0,1); }
}

#endif
