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
