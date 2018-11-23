//
//  common.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/01/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_common__
#define __Stonefish_common__

//System
#include <string>
#include <vector>
#include <deque>

//Bullet Physics
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>

//Eigen matrix library
#include <eigen3/Eigen/Dense>

namespace sf
{
    
#ifdef BT_USE_DOUBLE_PRECISION
	#define eigMatrix 	 Eigen::MatrixXd
    #define eigMatrix3x3 Eigen::Matrix3d
    #define eigMatrix4x4 Eigen::Matrix4d
	#define eigMatrix6x6 Eigen::Matrix<double, 6, 6>
#else
	#define eigMatrix 	 Eigen::MatrixXf
    #define eigMatrix3x3 Eigen::Matrix3f
    #define eigMatrix4x4 Eigen::Matrix4f
	#define eigMatrix6x6 Eigen::Matrix<float, 6, 6>
#endif
    
}

#endif
