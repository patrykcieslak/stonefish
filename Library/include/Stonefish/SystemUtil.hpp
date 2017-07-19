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

#ifdef __linux__
    #include <ctime>
    #include <chrono>
    #include <unistd.h>
#elif __APPLE__
    #include <mach/mach_time.h>
    #include <mach/mach.h>
    #include <unistd.h>
    #include <Carbon/Carbon.h>
#else //WINDOWS
    #include <windows.h>
#endif

#include "SimulationApp.h"
#include "OpenGLPipeline.h"

//Precise time
inline int64_t GetTimeInMicroseconds()
{
#ifdef __linux__
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
#elif __APPLE__
    return mach_absolute_time()/1000;
#else //WINDOWS
    unsigned ticks;
	LARGE_INTEGER count;
	static bool firstTime = true;
	static bool haveTimer = false;
	static LARGE_INTEGER frequency;
	static LARGE_INTEGER baseCount;
    
	if (firstTime) {
		firstTime = false;
		haveTimer = QueryPerformanceFrequency(&frequency) ? true : false;
		QueryPerformanceCounter (&baseCount);
	}
    
	QueryPerformanceCounter (&count);
	count.QuadPart -= baseCount.QuadPart;
	ticks = (uint64_t)(count.QuadPart * LONGLONG (1000000) / frequency.QuadPart);
	return ticks;
#endif
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
	char cwd[1024];
    GetCWD(cwd, 1024);
    std::string path(cwd);
	path += "/" + SimulationApp::getApp()->getShaderPath() + "/";
	return path;
}

inline std::string GetDataPath()
{
    char cwd[1024];
	GetCWD(cwd, 1024);
    std::string path(cwd);
	path += "/" + SimulationApp::getApp()->getDataPath() + "/";
	return path;
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

#endif 