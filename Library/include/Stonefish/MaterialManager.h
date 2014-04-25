//
//  MaterialManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MaterialManager__
#define __Stonefish_MaterialManager__

#include "UnitSystem.h"
#include "NameManager.h"

typedef struct
{
    int index;
    std::string name;
    btScalar density;
    btScalar restitution;
    std::vector<btScalar> staticFriction;
    std::vector<btScalar> dynamicFriction;
}
Material;

typedef struct
{
    std::string name;
    btScalar density;
    btScalar viscousity;
    btScalar IOR;
}
Fluid;

class MaterialManager
{
public:
    MaterialManager();
    ~MaterialManager();
    
    std::string CreateMaterial(std::string uniqueName, btScalar density, btScalar restitution);
    bool SetMaterialsInteraction(std::string firstMaterialName, std::string secondMaterialName, btScalar staticFricCoeff, btScalar dynamicFricCoeff);
    std::vector<std::string> GetMaterialsList();
    Material* getMaterial(std::string name);
    Material* getMaterial(int index);
    
    std::string CreateFluid(std::string uniqueName, btScalar density, btScalar viscousity, btScalar IOR);
    Fluid* getFluid(std::string name);
    Fluid* getFluid(int index);
    
    void ClearMaterialsAndFluids();
    
private:
    int getMaterialIndex(std::string name);
    
    std::vector<Material> materials;
    std::vector<Fluid> fluids;
    
    NameManager materialNameManager;
    NameManager fluidNameManager;
};

#endif 
