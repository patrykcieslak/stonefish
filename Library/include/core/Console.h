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
//  Console.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Console__
#define __Stonefish_Console__

#include <SDL2/SDL_thread.h>
#include "StonefishCommon.h"
#include "core/SimulationApp.h"

//shortcut
#define cInfo(format, ...)     SimulationApp::getApp()->getConsole()->Print(0, format, ##__VA_ARGS__)
#define cWarning(format, ...)  SimulationApp::getApp()->getConsole()->Print(1, format, ##__VA_ARGS__)
#define cError(format, ...)    SimulationApp::getApp()->getConsole()->Print(2, format, ##__VA_ARGS__)
#define cCritical(format, ...) {SimulationApp::getApp()->getConsole()->Print(3, format, ##__VA_ARGS__);abort();}

namespace sf
{
    //!A structure representing one line of text on the console.
    struct ConsoleMessage
    {
        int type;
        std::string text;
    };
    
    //! A class implementing a text console.
    class Console
    {
    public:
        //! A constructor.
        Console();
        
        //! A destructor.
        virtual ~Console();
        
        //! A method used to pring a message on the console.
        /*!
         \param messageType a type of message to be printed
         \param format a format string
         \param ... a set of variables refering to the format string (like printf() from standard library)
        */
        void Print(int messageType, std::string format, ...);
    
        //! A method to add messages to the console
        /*!
         \param msg a message to append to the console lines
         */
        void AppendMessage(const ConsoleMessage& msg);
        
        //! A method that clears the console.
        void Clear();
    
        //! A method that returns a pointer to the console data mutex.
        SDL_mutex* getLinesMutex();
        
        //! A method that returns a copy of the console lines.
        std::vector<ConsoleMessage> getLines();
        
    protected:
        std::vector<ConsoleMessage> lines;
        SDL_mutex* linesMutex;
    };
}

#endif
