//
//  MaterialManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "MaterialManager.h"


MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
    clearMaterialsAndFluids();
}

std::string MaterialManager::CreateMaterial(std::string name, btScalar density, btScalar restitution, btScalar staticF, btScalar dynamicF)
{
    Material mat;
    mat.density = UnitSystem::SetDensity(density);
    mat.restitution = restitution;
    mat.statFriction = staticF;
    mat.dynFriction = dynamicF;
    
    if(name == "")
        return NULL;
    
check_name:
    for(int i=0; i<materials.size(); i++)
    {
        if(name.compare(materials[i].name) == 0)
        {
            name.append("_");
            goto check_name;
        }
    }
    
    mat.name = name;
    materials.push_back(mat);
    
    return name;
}

Material* MaterialManager::getMaterial(std::string name)
{
    for(int i=0; i<materials.size(); i++)
    {
        if(materials[i].name.compare(name) == 0)
            return &materials[i];
    }
    
    return NULL;
}

Material* MaterialManager::getMaterial(int index)
{
    if(index >= 0 && index < materials.size())
        return &materials[index];
    else
        return NULL;
}

std::vector<std::string> MaterialManager::GetMaterialsList()
{
    std::vector<std::string> list;
    
    for(int i=0; i<materials.size(); i++)
        list.push_back(materials[i].name);
    
    return list;
}

std::string MaterialManager::CreateFluid(std::string name, btScalar density, btScalar viscousity, btScalar IOR)
{
    Fluid flu;
    flu.density = UnitSystem::SetDensity(density);
    flu.viscousity = viscousity;
    flu.IOR = IOR > 0 ? IOR : 1.0;
    
    if(name == "")
        return NULL;
    
check_name:
    for(int i=0; i<materials.size(); i++)
    {
        if(name.compare(materials[i].name) == 0)
        {
            name.append("_");
            goto check_name;
        }
    }
    
    flu.name = name;
    fluids.push_back(flu);
    
    return name;
}

Fluid* MaterialManager::getFluid(std::string name)
{
    for(int i=0; i<fluids.size(); i++)
    {
        if(fluids[i].name.compare(name) == 0)
            return &fluids[i];
    }
    
    return NULL;
}

Fluid* MaterialManager::getFluid(int index)
{
    if(index >= 0 && index < fluids.size())
        return &fluids[index];
    else
        return NULL;
}

void MaterialManager::clearMaterialsAndFluids()
{
    materials.clear();
    fluids.clear();
}