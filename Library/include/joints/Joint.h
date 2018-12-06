//
//  Joint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Joint__
#define __Stonefish_Joint__

#include <BulletDynamics/Featherstone/btMultiBodyConstraint.h>
#include "StonefishCommon.h"

#define CONSTRAINT_ERP 0.2
#define CONSTRAINT_CFM 0.0
#define CONSTRAINT_STOP_ERP 1.0
#define CONSTRAINT_STOP_CFM 0.0

namespace sf
{
    //! An enum representing the type of joint.
    typedef enum {JOINT_FIXED, JOINT_REVOLUTE, JOINT_SPHERICAL, JOINT_PRISMATIC, JOINT_CYLINDRICAL, JOINT_GEAR, JOINT_BELT, JOINT_SCREW} JointType;
    
    struct Renderable;
    class SimulationManager;
    
    //! An abstract class implementing a general joint.
    class Joint
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the joint
         \param collideLinkedEntities a flag that sets if the bodies connected by the joint should coliide
         */
        Joint(std::string uniqueName, bool collideLinkedEntities = true);
        
        //! A destructor.
        virtual ~Joint();
        
        //! A method used to add joint to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
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
        virtual JointType getType() = 0;
        
        //! A method returning the name of the joint.
        std::string getName();
        
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
