//
//  common.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/25/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_common_h__
#define __Stonefish_common_h__

//System
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <math.h>
#include <stdint.h>

//Bullet Physics
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>

//OGLFT
#define OGLFT_NO_SOLID
#define OGLFT_NO_QT

//Font
#ifdef __linux__
    #define FONT_NAME "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R.ttf"
    #define FONT_SIZE 12
    #define FONT_BASELINE 9
#elif __APPLE__
    #define FONT_NAME "/Library/Fonts/Arial.ttf"
    #define FONT_SIZE 12
    #define FONT_BASELINE 9
#endif

#define SCREEN_DPI 72

#endif