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
//  NameManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_NameManager__
#define __Stonefish_NameManager__

#include "StonefishCommon.h"

namespace sf
{
    //! A class used to manage unique names of objects in the simulation.
    class NameManager
    {
    public:
        //! A constructor.
        NameManager();
        
        //! A destructor.
        virtual ~NameManager();
        
        //! A method used to add new names to the pool.
        /*!
         \param proposedName a name proposed by the user
         \return a unique name
         */
        std::string AddName(std::string proposedName);
        
        //! A method used to remove names from the pool.
        /*!
         \param name a name to remove
         */
        void RemoveName(std::string name);
        
        //! A method used to clear the pool of names.
        void ClearNames();
        
    private:
        std::vector<std::string> names;
    };
}
    
#endif
