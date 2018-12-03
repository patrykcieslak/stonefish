//
//  StonefishCommon.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/01/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
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
