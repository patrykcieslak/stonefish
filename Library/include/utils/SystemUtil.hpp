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
//  SystemUtil.hpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SystemUtil__
#define __Stonefish_SystemUtil__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctime>
#include <chrono>

#ifdef __linux__
    #include <unistd.h>
#elif __APPLE__
    #include <unistd.h>
    #include <Carbon/Carbon.h>
#else //WINDOWS
    #include <windows.h>
#endif

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    
inline int64_t GetTimeInMicroseconds()
{
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

inline int64_t GetTimeInNanoseconds()
{
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

inline void GetCWD(char* buffer, int length)
{
#ifdef _MSC_VER
	GetCurrentDirectory(length, buffer);
#else
    getcwd(buffer, length);
#endif
}

inline std::string GetShaderPath()
{
    return ((GraphicalSimulationApp*)SimulationApp::getApp())->getShaderPath();
}

inline std::string GetDataPath()
{
    return SimulationApp::getApp()->getDataPath();
}

inline const char* GetDataPathPrefix(const char* directory)
{
    static char dataPathPrefix[PATH_MAX];
    
#ifdef __linux__

#elif __APPLE__    
    CFStringRef dir = CFStringCreateWithCString(CFAllocatorGetDefault(), directory, kCFStringEncodingMacRoman);
    
    CFURLRef datafilesURL = CFBundleCopyResourceURL(CFBundleGetMainBundle(), dir, 0, 0);
    
    CFURLGetFileSystemRepresentation(datafilesURL, true, reinterpret_cast<UInt8*>(dataPathPrefix), PATH_MAX);
    
    if(datafilesURL != NULL)
        CFRelease(datafilesURL);
    
    CFRelease(dir);
#else //WINDOWS
    char* envDataPath = 0;
    
    // get data path from environment var
    envDataPath = getenv(DATAPATH_VAR_NAME);
    
    // set data path prefix / base directory.  This will
    // be either from an environment variable, or from
    // a compiled in default based on original configure
    // options
    if (envDataPath != 0)
        strcpy(dataPathPrefix, envDataPath);
    else
        strcpy(dataPathPrefix, CEGUI_SAMPLE_DATAPATH);
#endif
    
    return dataPathPrefix;
}

//Extensions
inline bool CheckForExtension(const char* extensionName)
{
#ifdef _MSC_VER
	return glewIsSupported(extensionName);
#else
    char* extensions = (char*)glGetString(GL_EXTENSIONS);
    if(extensions == NULL)
        return false;
    
    size_t extNameLen = strlen(extensionName);
    char* end = extensions + strlen(extensions);
    
    while (extensions < end)
    {
        size_t n = strcspn(extensions, " ");
        if((extNameLen == n) && (strncmp(extensionName, extensions, n) == 0))
            return true;
        extensions += (n+1);
    }
    return false;
#endif
}

//Random functions
inline long lrandom(long *seed)
{
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

inline float frandom(long *seed)
{
    long r = lrandom(seed) >> (31 - 24);
    return r / (float)(1 << 24);
}

inline float grandom(float mean, float stdDeviation, long *seed)
{
    float x1, x2, w, y1;
    static float y2;
    static int use_last = 0;

    if (use_last)
    {
        y1 = y2;
        use_last = 0;
    }
    else
    {
        do
        {
            x1 = 2.0f * frandom(seed) - 1.0f;
            x2 = 2.0f * frandom(seed) - 1.0f;
            w  = x1 * x1 + x2 * x2;
        }
        while (w >= 1.0f);
        w  = sqrt((-2.0f * log(w)) / w);
        y1 = x1 * w;
        y2 = x2 * w;
        use_last = 1;
    }
    return mean + y1 * stdDeviation;
}
    
}

#endif 
