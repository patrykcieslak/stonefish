//
//  NameManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "NameManager.h"

#pragma mark Constructors
NameManager::NameManager()
{
    names = std::vector<std::string>(0);
}

#pragma mark - Destructor
NameManager::~NameManager()
{
    names.clear();
}

#pragma mark - Methods
std::string NameManager::AddName(std::string proposedName)
{
    std::string goodName = proposedName;
    int number = 1;
    
checkname:
    for(int i = 0; i < names.size(); i++)
    {
        if(goodName == names[i])
        {
            goodName = proposedName + std::to_string(number);
            number++;
            goto checkname;
        }
    }
    
    names.push_back(goodName);
    return goodName;
}

void NameManager::RemoveName(std::string name)
{
    std::vector<std::string>::iterator it;
    for(it = names.begin(); it < names.end(); it++)
        if(*it == name)
        {
            names.erase(it);
            break;
        }
}

void NameManager::ClearNames()
{
    names.clear();
}