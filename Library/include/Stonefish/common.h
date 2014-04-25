//
//  common.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/25/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_common_h__
#define __Stonefish_common_h__

//Stonefish
//#define USE_ADVANCED_GUI
//#define USE_DOUBLE_PRECISION

//System
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <math.h>
#include <stdint.h>

//Bullet Physics
#ifdef USE_DOUBLE_PRECISION
#define BT_USE_DOUBLE_PRECISION
#endif
#define BT_EULER_DEFAULT_ZYX
#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/btBulletCollisionCommon.h>

#endif