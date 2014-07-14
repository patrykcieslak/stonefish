//
//  SystemUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SystemUtil__
#define __Stonefish_SystemUtil__

#include <stdint.h>
#include "OpenGLPipeline.h"

uint64_t GetTimeInMicroseconds();
void GetCWD(char* buffer,int length);
void GetShaderPath(char* path, int len);
void GetDataPath(char* path, int len);
const char* GetDataPathPrefix(const char* directory);
bool CheckForExtension(const char* extensionName);

#endif 

