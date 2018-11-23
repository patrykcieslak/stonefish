//
//  MaterialManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "core/MaterialManager.h"
#include "graphics/Console.h"

using namespace sf;

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
	interactions.clear();
    materialNameManager.ClearNames();
    fluidNameManager.ClearNames();
}

std::string MaterialManager::CreateMaterial(std::string uniqueName, btScalar density, btScalar restitution)
{
	//Create and add new material
    Material mat;
    mat.name = materialNameManager.AddName(uniqueName);
    mat.density = UnitSystem::SetDensity(density);
    mat.restitution = restitution;
    materials.push_back(mat);
	
	cInfo("Material %s (%d) created.", mat.name.c_str(), materials.size()-1);
    
	//Set initial friction coefficients
	Friction f;
	f.fStatic = btScalar(1);
	f.fDynamic = btScalar(1);
	
	MaterialPair p;
	p.mat1Id = (int)materials.size()-1;
	
	for(unsigned int i=0; i < materials.size(); ++i)
	{
		p.mat2Id = i;
		interactions.insert({p,f});
	}
	
	//Display interactions
	//for(std::pair<MaterialPair, Friction> element : interactions)
	//	std::cout << "(" << element.first.mat1Id << "," << element.first.mat2Id << ")" << std::endl;

    return mat.name;
}

std::string MaterialManager::CreateFluid(std::string uniqueName, btScalar density, btScalar viscosity, btScalar IOR)
{
    Liquid flu;
    flu.name = fluidNameManager.AddName(uniqueName);
    flu.density = UnitSystem::SetDensity(density);
    flu.viscosity = viscosity;
    flu.IOR = IOR > 0 ? IOR : 1.0;
    fluids.push_back(flu);
    
    return flu.name;
}

bool MaterialManager::SetMaterialsInteraction(std::string firstMaterialName, std::string secondMaterialName, btScalar staticFricCoeff, btScalar dynamicFricCoeff)
{
    MaterialPair p;
	p.mat1Id = getMaterialIndex(firstMaterialName);
	p.mat2Id = getMaterialIndex(secondMaterialName);
	
	Friction f;
	f.fStatic = staticFricCoeff;
	f.fDynamic = dynamicFricCoeff;
	
	try
	{
		interactions.at(p) = f;
		return true;
	}
	catch(const std::out_of_range& e)
	{
		cError("Material pair (%s,%s) not found!", firstMaterialName.c_str(), secondMaterialName.c_str());
		return false;
	}
}

Friction MaterialManager::GetMaterialsInteraction(int mat1Index, int mat2Index)
{
	MaterialPair p;
	p.mat1Id = mat1Index;
	p.mat2Id = mat2Index;
		
	try
	{
		Friction f = interactions.at(p);
		return f;
	}
	catch(const std::out_of_range& e)
	{
		cError("Material pair (%d,%d) not found!", mat1Index, mat2Index);
		
		Friction f;
		f.fStatic = btScalar(1);
		f.fDynamic = btScalar(2);
		
		return f;
	}
}

Friction MaterialManager::GetMaterialsInteraction(std::string mat1Name, std::string mat2Name)
{
	return GetMaterialsInteraction(getMaterialIndex(mat1Name), getMaterialIndex(mat2Name));
}

int MaterialManager::getMaterialIndex(std::string name)
{
    for(unsigned int i=0; i<materials.size(); ++i)
        if(materials[i].name == name)
            return i;
    
	cError("Wrong material name %s!", name.c_str());
    return -1;
}

Material MaterialManager::getMaterial(std::string name)
{
	for(unsigned int i=0; i<materials.size(); ++i)
        if(materials[i].name == name)
			return materials[i];
    
	return materials[0];
}

Material MaterialManager::getMaterial(int index)
{
	if(index >= 0 && index < (int)materials.size())
        return materials[index];
    else
        return materials[0];
}

Liquid* MaterialManager::getFluid(std::string name)
{
    for(unsigned int i=0; i<fluids.size(); ++i)
        if(fluids[i].name == name)
            return &fluids[i];
    
    return NULL;
}

Liquid* MaterialManager::getFluid(int index)
{
    if(index >= 0 && index < (int)fluids.size())
        return &fluids[index];
    else
        return NULL;
}

std::vector<std::string> MaterialManager::GetMaterialsList()
{
    std::vector<std::string> list;
    
    for(unsigned int i=0; i<materials.size(); i++)
        list.push_back(materials[i].name);
    
    return list;
}
