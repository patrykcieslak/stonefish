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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Console__
#define __Stonefish_Console__

#include <SDL2/SDL_thread.h>
#include "StonefishCommon.h"

namespace sf
{
    //! An enum defining types of messages.
    enum class MessageType{INFO, WARNING, ERROR, CRITICAL};

    //!A structure representing one line of text on the console.
    struct ConsoleMessage
    {
        MessageType type;
        std::string text;
    };
    
    //! A class implementing a text console.
    class Console
    {
    public:
        //! A constructor.
        /*!
         \param useStdout a flag enabling message printing on the system standard output
         */
        Console(bool useStdout = true);
        
        //! A destructor.
        virtual ~Console();
        
        //! A method used to print a message on the console.
        /*!
         \param t a type of message to be printed
         \param format a format string
         \param ... a set of variables refering to the format string (like printf() from standard library)
        */
        void Print(MessageType t, std::string format, ...);
    
        //! A method to add messages to the console
        /*!
         \param msg a message to append to the console lines
         */
        void AppendMessage(const ConsoleMessage& msg);
        
        //! A method that clears the console.
        void Clear();

        //! A method that saves the console contents to a text file.
        /*!
         \param filename a path to the output text file
         \return success
         */
        bool SaveToFile(std::string filename);
    
        //! A method that returns a pointer to the console data mutex.
        SDL_mutex* getLinesMutex();
        
        //! A method that returns a copy of the console lines.
        std::vector<ConsoleMessage> getLines();
        
    protected:
        bool stdoutEnabled;
        std::vector<ConsoleMessage> lines;
        SDL_mutex* linesMutex;
    };
}

#endif
