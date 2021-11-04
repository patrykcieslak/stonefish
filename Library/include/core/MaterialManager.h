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
//  MaterialManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/7/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MaterialManager__
#define __Stonefish_MaterialManager__

#include <unordered_map>
#include "core/NameManager.h"

namespace sf
{
    //! A structure holding material properties.
    struct Material
    {
        std::string name;
        Scalar density;
        Scalar restitution;
        Scalar magnetic; // <0 ferromagnetic, 0 nonmagnetic, >0 magnet
    };
    
    //! A structure holding fluid properties.
    struct Fluid
    {
        std::string name;
        Scalar density;
        Scalar viscosity;
        Scalar IOR;
        
        Fluid()
        {
            name = "";
            density = Scalar(0);
            viscosity = Scalar(0);
            IOR = Scalar(1);
        }
    };
    
    //! A strcture holding friction coefficients.
    struct Friction
    {
        Scalar fStatic;
        Scalar fDynamic;
    };
    
    //! A structure representing a pair of materials.
    struct MaterialPair
    {
        int mat1Id;
        int mat2Id;
        
        bool operator==(const MaterialPair& rhs) const
        {
            return (mat1Id == rhs.mat1Id && mat2Id == rhs.mat2Id)||(mat1Id == rhs.mat2Id && mat2Id == rhs.mat1Id);
        }
    };
    
    //! A hashing function for material interaction.
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
    
    class NameManager;
    
    //! A class implementing a physical material manager.
    class MaterialManager
    {
    public:
        //! A constructor.
        MaterialManager();
        
        //! A destructor.
        ~MaterialManager();
        
        //! A method that creates a new material.
        /*!
         \param uniqueName a name for the material
         \param density a density of the material [kg*m^-3]
         \param restitution a restitution factor <0,1>
         \param magnetic a factor defining magnetic properties
         \return a name of the created material
         */
        std::string CreateMaterial(const std::string& uniqueName, Scalar density, Scalar restitution, Scalar magnetic = Scalar(0));
        
        //! A method that sets interaction between a pair of materials.
        /*!
         \param firstMaterialName a name of the first material
         \param secondMaterialName a name of the second material
         \param staticFricCoeff a coefficient of static friction between materials
         \param dynamicFricCoeff a coefficient of dynamic friction between materials
         \return was the interaction was set properly?
         */
        bool SetMaterialsInteraction(const std::string& firstMaterialName, const std::string& secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff);
        
        //! A method that returns friction information for a specified pair of materials.
        /*!
         \param mat1Index an id of the first material
         \param mat2Index and id of the second material
         \return a structure containing friction coefficients
         */
        Friction GetMaterialsInteraction(int mat1Index, int mat2Index);
        
        //! A method that returns friction information for a specified pair of materials.
        /*!
         \param mat1Name a name of the first material
         \param mat2Name a name of the second material
         \return a structure containing friction coefficients
         */
        Friction GetMaterialsInteraction(const std::string& mat1Name, const std::string& mat2Name);
        
        //! A method returning a list of materials (names).
        std::vector<std::string> GetMaterialsList();
        
        //! A method returning material information.
        /*!
         \param name a name of the material
         \return a structure containing properties of the material
         */
        Material getMaterial(const std::string& name);
        
        //! A method returning material information.
        /*!
         \param index an id of the material
         \return a structure containing properties of the material
         */
        Material getMaterial(int index);
        
        //! A method that creates a new fluid.
        /*!
         \param uniqueName a name for the fluid
         \param density a density of the fluid [kg*m^-3]
         \param viscosity a .... viscosity of the fluid [...]
         \param IOR index of refraction of the fluid
         \return a name of the created fluid
         */
        std::string CreateFluid(const std::string& uniqueName, Scalar density, Scalar viscosity, Scalar IOR);
        
        //! A method returning a fluid by name.
        /*!
         \param name a name of the fluid
         \return a the fluid structure
         */
        Fluid getFluid(const std::string& name);
        
        //! A method returning a fluid by id.
        /*!
         \param index an id of the fluid
         \return a the fluid structure
         */
        Fluid getFluid(int index);
        
        //! A method that deletes all materials and fluids from the manager.
        void ClearMaterialsAndFluids();
        
    private:
        int getMaterialIndex(const std::string& name);
        
        std::vector<Material> materials;
        std::unordered_map<MaterialPair, Friction, MaterialPairHash> interactions;
        std::vector<Fluid> fluids;
        
        NameManager materialNameManager;
        NameManager fluidNameManager;
    };
}

#endif 
