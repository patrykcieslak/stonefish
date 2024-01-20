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
//  SystemUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/24/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "utils/SystemUtil.hpp"

#ifdef WIN32
    #define WINDOWS_LEAN_AND_MEAN
    #include <windows.h>
    #define PATH_MAX                MAX_PATH
#endif

namespace sf
{

    void GetCWD(char* buffer, int length)
    {
    #ifdef _MSC_VER
        GetCurrentDirectory(length, buffer);
    #else
        getcwd(buffer, length);
    #endif
    }

    const char* GetDataPathPrefix(const char* directory)
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
        envDataPath = getenv("STONEFISH_DATA_PATH");
        
        // set data path prefix / base directory.  This will
        // be either from an environment variable, or from
        // a compiled in default based on original configure
        // options
        if (envDataPath != 0)
            strcpy(dataPathPrefix, envDataPath);
        else
            strcpy(dataPathPrefix, "CEGUI_SAMPLE_DATAPATH");
    #endif
        
        return dataPathPrefix;
    }

}
