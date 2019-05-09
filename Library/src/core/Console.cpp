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
//  Console.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "core/Console.h"

namespace sf
{
    
Console::Console()
{
	lines = std::vector<ConsoleMessage>(0);
    linesMutex = SDL_CreateMutex();
}

Console::~Console()
{
    lines.clear();
    SDL_DestroyMutex(linesMutex);
}
    
SDL_mutex* Console::getLinesMutex()
{
    return linesMutex;
}

std::vector<ConsoleMessage> Console::getLines()
{
    return lines;
}

void Console::Print(int messageType, std::string format, ...)
{
	va_list args;
    char buffer[4096];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);
	
#ifdef COLOR_CONSOLE
	switch(messageType)
	{
		default:
		case 0: //Info
			printf("[INFO] %s\n", buffer);
			break;
			
		case 1: //Warning
			printf("\033[33m[WARN] %s\033[0m\n", buffer);
			break;
			
		case 2: //Error
			printf("\033[31m[ERROR] %s\033[0m\n", buffer);
			break;
			
		case 3: //Critical
			printf("\033[1;31m[CRITICAL] %s\033[0m\n", buffer);
			break;
	}
#else
    switch(messageType)
    {
        default:
        case 0: //Info
            printf("[INFO] %s\n", buffer);
            break;
            
        case 1: //Warning
            printf("[WARN] %s\n", buffer);
            break;
            
        case 2: //Error
            printf("[ERROR] %s\n", buffer);
            break;
            
        case 3: //Critical
            printf("[CRITICAL] %s\n", buffer);
            break;
    }
#endif
	
    ConsoleMessage msg;
	msg.type = messageType;
	msg.text = std::string(buffer);
	SDL_LockMutex(linesMutex);
	lines.push_back(msg);
	SDL_UnlockMutex(linesMutex);
}

void Console::AppendMessage(const ConsoleMessage& msg)
{
    SDL_LockMutex(linesMutex);
    lines.push_back(msg);
    SDL_UnlockMutex(linesMutex);
}
    
void Console::Clear()
{
    SDL_LockMutex(linesMutex);
    lines.clear();
    SDL_UnlockMutex(linesMutex);
}

}
