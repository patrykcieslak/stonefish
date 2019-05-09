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
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
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
#include <LinearMath/btMatrixX.h>

//Eigen matrix library
#include <eigen3/Eigen/Dense>

namespace sf
{
    typedef btScalar Scalar;
    typedef btQuaternion Quaternion;
    typedef btVector3 Vector3;
    typedef btMatrix3x3 Matrix3;
    typedef btTransform Transform;
    typedef btVectorXu VectorX;
    typedef btMatrixXu MatrixX;
    
#ifdef BT_USE_DOUBLE_PRECISION
    typedef Eigen::MatrixXd MatrixXEigen;
    typedef Eigen::Matrix3d Matrix3Eigen;
    typedef Eigen::Matrix4d Matrix4Eigen;
    typedef Eigen::Matrix<double, 6, 6> Matrix6Eigen;
#else
    typedef Eigen::MatrixXf MatrixXEigen;
    typedef Eigen::Matrix3f Matrix3Eigen;
    typedef Eigen::Matrix4f Matrix4Eigen;
    typedef Eigen::Matrix<float, 6, 6> Matrix6Eigen;
#endif
    
    inline Transform I4() { return Transform::getIdentity(); }
    inline Matrix3 I3() { return Matrix3::getIdentity(); }
    inline Quaternion IQ() { return Quaternion::getIdentity(); }
}

#endif
