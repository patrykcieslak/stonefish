//
//  NameManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_NameManager__
#define __Stonefish_NameManager__

#include "common.h"

class NameManager
{
public:
    NameManager();
    virtual ~NameManager();
    
    std::string AddName(std::string proposedName);
    void RemoveName(std::string name);
    void ClearNames();
    
private:
    std::vector<std::string> names;
};


#endif
