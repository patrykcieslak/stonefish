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
//  Fall.h
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Fall__
#define __Stonefish_Fall__

#include "visuals/Visual.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLFall.h"

namespace sf
{

//! A class implementing a system of falling particles.
class Fall : public Visual
{
public:
    //! A constructor.
    /*!
     \param uniqueName a name for the particle system
     \param maxParticles the number of simulated particles
     \param lifetime the lifetime of the particles [s]
     \param emitterSizeX the size of the emmiter in X direction [m]
     \param emitterSizeY the size of the emmiter in Y direction [m]
     \param particleModelPaths a vector of paths to the models representing the particles
     \param particleMaterial a material for the particles
     \param particleLook a look for the particles
     */
    Fall(const std::string& uniqueName, unsigned int maxParticles, Scalar lifetime, Scalar emitterSizeX, Scalar emitterSizeY, 
        std::vector<std::string> particleModelPaths, const std::string& particleMaterial, const std::string& particleLook);

    //! A destructor.
    virtual ~Fall();

    //! A method that updates the emitter position.
    void UpdateTransform() override;

    //! A method implementing the rendering of the particle system dummy.
    std::vector<Renderable> Render() override;

    //! A method returning the type of the visual.
    VisualType getType() const override;

protected:
    OpenGLFall* glFall;
};

}

#endif /* defined(__Stonefish_Fall__) */