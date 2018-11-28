//
//  MaterialManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MaterialManager__
#define __Stonefish_MaterialManager__

#include <unordered_map>
#include "core/NameManager.h"

namespace sf
{

struct Material 
{
    std::string name;
    Scalar density;
    Scalar restitution;
};

struct Fluid
{
    std::string name;
    Scalar density;
    Scalar viscosity;
    Scalar IOR;
};

struct Friction
{
	Scalar fStatic;
	Scalar fDynamic;
};

struct MaterialPair
{
	int mat1Id;
	int mat2Id;
	
	bool operator==(const MaterialPair& rhs) const
	{
		return (mat1Id == rhs.mat1Id && mat2Id == rhs.mat2Id)||(mat1Id == rhs.mat2Id && mat2Id == rhs.mat1Id);
	}
};

struct MaterialPairHash
{
	std::size_t operator()(const MaterialPair& mp) const
	{
		if(mp.mat1Id <= mp.mat2Id)
			return std::hash<int>()(mp.mat1Id) ^ (std::hash<int>()(mp.mat2Id) << 16);
		else
			return std::hash<int>()(mp.mat2Id) ^ (std::hash<int>()(mp.mat1Id) << 16);
	}
};

class MaterialManager
{
public:
    MaterialManager();
    ~MaterialManager();
    
    std::string CreateMaterial(std::string uniqueName, Scalar density, Scalar restitution);
    bool SetMaterialsInteraction(std::string firstMaterialName, std::string secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff);
    Friction GetMaterialsInteraction(int mat1Index, int mat2Index);
	Friction GetMaterialsInteraction(std::string mat1Name, std::string mat2Name);
	std::vector<std::string> GetMaterialsList();
    Material getMaterial(std::string name);
    Material getMaterial(int index);
    
    std::string CreateFluid(std::string uniqueName, Scalar density, Scalar viscosity, Scalar IOR);
    Fluid* getFluid(std::string name);
    Fluid* getFluid(int index);
    
    void ClearMaterialsAndFluids();
    
private:
    int getMaterialIndex(std::string name);
    
    std::vector<Material> materials;
	std::unordered_map<MaterialPair, Friction, MaterialPairHash> interactions;
    std::vector<Fluid> fluids;
    
    NameManager materialNameManager;
    NameManager fluidNameManager;
};
    
}

#endif 
