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
    ClearMaterialsAndFluids();
}

void MaterialManager::ClearMaterialsAndFluids()
{
    materials.clear();
    fluids.clear();
    materialNameManager.ClearNames();
    fluidNameManager.ClearNames();
}

std::string MaterialManager::CreateMaterial(std::string uniqueName, btScalar density, btScalar restitution)
{
    Material mat;
    mat.index = (int)materials.size();
    mat.name = materialNameManager.AddName(uniqueName);
    mat.density = UnitSystem::SetDensity(density);
    mat.restitution = restitution;
    
    for(int i = 0; i <= mat.index; i++)
    {
        mat.staticFriction.push_back(btScalar(1.));
        mat.dynamicFriction.push_back(btScalar(1.));
    }
    
    for(int i = 0; i < mat.index; i++)
    {
        materials[i].staticFriction.push_back(btScalar(1.));
        materials[i].dynamicFriction.push_back(btScalar(1.));
    }
    
    materials.push_back(mat);
    
    return mat.name;
}

std::string MaterialManager::CreateFluid(std::string uniqueName, btScalar density, btScalar viscosity, btScalar IOR)
{
    Fluid flu;
    flu.name = fluidNameManager.AddName(uniqueName);
    flu.density = UnitSystem::SetDensity(density);
    flu.viscosity = viscosity;
    flu.IOR = IOR > 0 ? IOR : 1.0;
    fluids.push_back(flu);
    
    return flu.name;
}

bool MaterialManager::SetMaterialsInteraction(std::string firstMaterialName, std::string secondMaterialName, btScalar staticFricCoeff, btScalar dynamicFricCoeff)
{
    int index1 = getMaterialIndex(firstMaterialName);
    int index2 = getMaterialIndex(secondMaterialName);
    
    if(index1 < 0 || index2 < 0)
        return false;
    
    materials[index1].staticFriction[index2] = staticFricCoeff;
    materials[index2].staticFriction[index1] = staticFricCoeff;
    materials[index1].dynamicFriction[index2] = dynamicFricCoeff;
    materials[index2].dynamicFriction[index1] = dynamicFricCoeff;
    return true;
}

int MaterialManager::getMaterialIndex(std::string name)
{
    for(int i=0; i<materials.size(); i++)
        if(materials[i].name.compare(name) == 0)
            return i;
    
    return -1;
}

Material* MaterialManager::getMaterial(std::string name)
{
    int index = getMaterialIndex(name);
    
    if(index >= 0)
        return &materials[index];
    else
        return NULL;
}

Material* MaterialManager::getMaterial(int index)
{
    if(index >= 0 && index < materials.size())
        return &materials[index];
    else
        return NULL;
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

std::vector<std::string> MaterialManager::GetMaterialsList()
{
    std::vector<std::string> list;
    
    for(int i=0; i<materials.size(); i++)
        list.push_back(materials[i].name);
    
    return list;
}