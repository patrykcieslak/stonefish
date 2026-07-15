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
//  MaterialManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "core/MaterialManager.h"

#include "core/SimulationApp.h"

namespace sf
{

MaterialManager::~MaterialManager()
{
    ClearMaterialsAndFluids();
}

void MaterialManager::ClearMaterialsAndFluids()
{
    materials_.clear();
    fluids_.clear();
    interactions_.clear();
    materialNameManager_.ClearNames();
    fluidNameManager_.ClearNames();
}

std::string MaterialManager::CreateMaterial(const std::string& uniqueName, Scalar density, Scalar restitution, Scalar magnetic)
{
    //Create and add new material
    Material mat;
    mat.name = materialNameManager_.AddName(uniqueName);
    mat.density = density;
    mat.restitution = restitution;
    mat.magnetic = magnetic;
    materials_.push_back(mat);
    
    cInfo("Material %s (%d) created.", mat.name.c_str(), materials_.size()-1);
    
    //Set initial friction coefficients
    Friction f;
    f.fStatic = Scalar(1);
    f.fDynamic = Scalar(1);
    
    MaterialPair p;
    p.mat1Id = (int)materials_.size()-1;
    
    for(unsigned int i=0; i < materials_.size(); ++i)
    {
        p.mat2Id = i;
        interactions_.insert({p,f});
    }
    
    //Display interactions
    //for(std::pair<MaterialPair, Friction> element : interactions)
    //	std::cout << "(" << element.first.mat1Id << "," << element.first.mat2Id << ")" << std::endl;

    return mat.name;
}

std::string MaterialManager::CreateFluid(const std::string& uniqueName, Scalar density, Scalar viscosity, Scalar IOR)
{
    Fluid flu;
    flu.name = fluidNameManager_.AddName(uniqueName);
    flu.density = density;
    flu.viscosity = viscosity;
    flu.IOR = IOR > 0 ? IOR : 1.0;
    fluids_.push_back(flu);
    
    return flu.name;
}

bool MaterialManager::SetMaterialsInteraction(const std::string& firstMaterialName, const std::string& secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff)
{
    MaterialPair p;
    p.mat1Id = getMaterialIndex(firstMaterialName);
    p.mat2Id = getMaterialIndex(secondMaterialName);
    
    Friction f;
    f.fStatic = staticFricCoeff;
    f.fDynamic = dynamicFricCoeff;
    
    try
    {
        interactions_.at(p) = f;
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
        Friction f = interactions_.at(p);
        return f;
    }
    catch(const std::out_of_range& e)
    {
        cError("Material pair (%d,%d) not found!", mat1Index, mat2Index);
        
        Friction f;
        f.fStatic = Scalar(1);
        f.fDynamic = Scalar(2);
        
        return f;
    }
}

Friction MaterialManager::GetMaterialsInteraction(const std::string& mat1Name, const std::string& mat2Name)
{
    return GetMaterialsInteraction(getMaterialIndex(mat1Name), getMaterialIndex(mat2Name));
}

int MaterialManager::getMaterialIndex(const std::string& name)
{
    auto it = std::find_if(materials_.begin(), materials_.end(), [&name](const Material& m) { return m.name == name; });
    if(it != materials_.end())
        return it - materials_.begin();
    else
        return -1;
}

Material MaterialManager::getMaterial(int index)
{
    if(index >= 0 && index < (int)materials_.size())
        return materials_[index];
    else
        return materials_[0];
}

Material MaterialManager::getMaterial(const std::string& name)
{
    return getMaterial(getMaterialIndex(name));
}

Fluid MaterialManager::getFluid(const std::string& name)
{
    auto it = std::find_if(fluids_.begin(), fluids_.end(), [&name](const Fluid& f) { return f.name == name; });
    if(it != fluids_.end())
        return *it;
    else
        return Fluid();
}

Fluid MaterialManager::getFluid(int index)
{
    if(index >= 0 && index < (int)fluids_.size())
        return fluids_[index];
    else
        return Fluid();
}

std::vector<std::string> MaterialManager::GetMaterialsList()
{
    std::vector<std::string> list;
    
    for(unsigned int i=0; i<materials_.size(); i++)
        list.push_back(materials_[i].name);
    
    return list;
}

}
