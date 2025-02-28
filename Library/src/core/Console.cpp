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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#include "core/Console.h"
#include <iostream>
#include <fstream>

namespace sf
{
    
Console::Console(bool useStdout)
{
    stdoutEnabled = useStdout;
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

void Console::Print(MessageType t, std::string format, ...)
{
    va_list args;
    char buffer[4096];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);
    
    if(stdoutEnabled)
    {
#ifdef COLOR_CONSOLE
        switch(t)
        {
            default:
            case MessageType::INFO:
                printf("[INFO] %s\n", buffer);
                break;
                
            case MessageType::WARNING:
                printf("\033[33m[WARN] %s\033[0m\n", buffer);
                break;
                
            case MessageType::ERROR:
                printf("\033[31m[ERROR] %s\033[0m\n", buffer);
                break;
                
            case MessageType::CRITICAL:
                printf("\033[1;31m[CRITICAL] %s\033[0m\n", buffer);
                break;
        }
#else
        switch(t)
        {
            default:
            case MessageType::INFO:
                printf("[INFO] %s\n", buffer);
                break;
                
            case MessageType::WARNING:
                printf("[WARN] %s\n", buffer);
                break;
                
            case MessageType::ERROR:
                printf("[ERROR] %s\n", buffer);
                break;
                
            case MessageType::CRITICAL:
                printf("[CRITICAL] %s\n", buffer);
                break;
        }
#endif
    }

    ConsoleMessage msg;
    msg.type = t;
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

bool Console::SaveToFile(std::string filename)
{
    std::ofstream outFile(filename);
    if(outFile.is_open())
    {
        SDL_LockMutex(linesMutex);
        for(size_t i=0; i<lines.size(); ++i)
        {
            switch(lines[i].type)
            {
                case MessageType::INFO:
                    outFile << "[INFO] " << lines[i].text << std::endl;
                    break;
                case MessageType::WARNING:
                    outFile << "[WARN] " << lines[i].text << std::endl;
                    break;
                case MessageType::ERROR:
                    outFile << "[ERROR] " << lines[i].text << std::endl;
                    break;
                case MessageType::CRITICAL:
                    outFile << "[CRITICAL] " << lines[i].text << std::endl;
                    break;
            }
        }
        SDL_UnlockMutex(linesMutex);
        outFile.close();
        return true;
    }
    else
        return false;
}

}
