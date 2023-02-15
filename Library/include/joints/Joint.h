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
//  Joint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Joint__
#define __Stonefish_Joint__

#include "BulletDynamics/Featherstone/btMultiBodyConstraint.h"
#include "StonefishCommon.h"

namespace sf
{
    //! An enum representing the type of joint.
    enum class JointType {FIXED, SPRING, REVOLUTE, SPHERICAL, PRISMATIC, CYLINDRICAL};
    
    struct Renderable;
    class SimulationManager;
    
    //! An abstract class implementing a general joint.
    class Joint
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the joint
         \param collideLinked a flag that sets if the bodies connected by the joint should coliide
         */
        Joint(std::string uniqueName, bool collideLinked = true);
        
        //! A destructor.
        virtual ~Joint();
        
        //! A method used to add joint to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);

        //! A method used to remove joint from the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void RemoveFromSimulation(SimulationManager* sm);
        
        //! A method applying damping to the joint.
        virtual void ApplyDamping();
        
        //! A method that solves initial conditions problem for the joint.
        /*!
         \param linearTolerance a value of the tolerance in position (termination condition)
         \param angularTolerance a value of the tolerance in rotation (termination condition)
         */
        virtual bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance);
        
        //! A method implementing the rendering of the joint.
        virtual std::vector<Renderable> Render();
        
        //! A method returning the type of the joint.
        virtual JointType getType() const = 0;
        
        //! A method returning the name of the joint.
        std::string getName() const;
        
        //! A method returning the internal constraint.
        btTypedConstraint* getConstraint();
        
        //! A method returning joint feedback in the world frame.
        /*!
         \param dof degree of freedom for which the feedback is desired
         \return value of force/torque for the specified degree of freedom
         */
        Scalar getFeedback(unsigned int dof);
        
        //! A method that informs if the joint is of multibody type.
        bool isMultibodyJoint();
        
    protected:
        void setConstraint(btTypedConstraint* c);
        void setConstraint(btMultiBodyConstraint* c);
        
    private:
        std::string name;
        bool collisionEnabled;
        btTypedConstraint* constraint;
        btMultiBodyConstraint* mbConstraint;
    };
}

#endif
