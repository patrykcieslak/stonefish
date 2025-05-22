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
//  ConsoleSimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ConsoleSimulationApp__
#define __Stonefish_ConsoleSimulationApp__

#include <SDL2/SDL_thread.h>
#include "core/SimulationApp.h"

namespace sf
{
    class Console;
    
    //! A class that defines a console application interface.
    class ConsoleSimulationApp : public SimulationApp
    {
    public:
        //! A constructor.
        /*!
         \param title a title for the application
         \param dataDirPath a path to the directory containing the simulation data
         \param sim a pointer to the simulation manager
         */
        ConsoleSimulationApp(std::string title, std::string dataDirPath, SimulationManager* sim);
        
        //! A destructor.
        virtual ~ConsoleSimulationApp();

        //! A method that starts the simulation on demand.
        void StartSimulation() override;

        //! A method that stops the simulation on demand.
        void StopSimulation() override;

        //! A method that resumes the simulation on demand.
        void ResumeSimulation() override;
        
        //! A method informing if the application is graphical.
        bool hasGraphics();
        
    protected:
        void Init();
        void LoopInternal();
        
    private:
        SDL_Thread* simulationThread;
        static int RunSimulation(void* data);
    };
    
    //! A structure used to pass information between threads.
    struct ConsoleSimulationThreadData
    {
        ConsoleSimulationApp& app;
    };
}

#endif
