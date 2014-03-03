//
//  MaterialManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MaterialManager__
#define __Stonefish_MaterialManager__

#include "common.h"

typedef struct
{
    std::string name;
    btScalar density;
    btScalar restitution;
    btScalar statFriction;
    btScalar dynFriction;
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
    
    std::string CreateMaterial(std::string name, btScalar density, btScalar restitution, btScalar staticF, btScalar dynamicF);
    Material* getMaterial(std::string name);
    Material* getMaterial(int index);
    std::vector<std::string> GetMaterialsList();
    
    std::string CreateFluid(std::string name, btScalar density, btScalar viscousity, btScalar IOR);
    Fluid* getFluid(std::string name);
    Fluid* getFluid(int index);
    
    void clearMaterialsAndFluids();
    
private:
    std::vector<Material> materials;
    std::vector<Fluid> fluids;
};

#endif 
