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
//  FeatherstoneEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright(c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FeatherstoneEntity__
#define __Stonefish_FeatherstoneEntity__

#include "BulletDynamics/Featherstone/btMultiBody.h"
#include "BulletDynamics/Featherstone/btMultiBodyLinkCollider.h"
#include "BulletDynamics/Featherstone/btMultiBodyLink.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointFeedback.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointMotor.h"
#include "entities/SolidEntity.h"

namespace sf
{
    class StaticEntity;
    
    //! A structure holding data of a single multibody link.
    struct FeatherstoneLink
    {
        //! A constructor.
        /*!
         \param s a pointer to a solid entity
         \param t the transformation of the solid entity
         */
        FeatherstoneLink(SolidEntity* s, const Transform& t) : solid(s), trans(t) {}
        
        SolidEntity* solid;
        Transform trans;
    };
    
    //! A structure holding data of a single multibody joint.
    struct FeatherstoneJoint
    {
        //! A constructor.
        /*!
         \param n the name of the joint
         \param t the type of the joint
         \param p the id of the parent link
         \param c the id of the child link
         */
        FeatherstoneJoint(std::string n, btMultibodyLink::eFeatherstoneJointType t, unsigned int p, unsigned int c)
        : name(n), type(t), feedback(NULL), limit(NULL), motor(NULL), parent(p), child(c), sigDamping(0), velDamping(0), lowerLimit(10e9), upperLimit(-10e9) {}
        
        std::string name;
        btMultibodyLink::eFeatherstoneJointType type;
        btMultiBodyJointFeedback* feedback;
        btMultiBodyJointLimitConstraint* limit;
        btMultiBodyJointMotor* motor;
        
        unsigned int parent;
        unsigned int child;
        Vector3 axisInChild;
        Vector3 pivotInChild;
        Scalar sigDamping;
        Scalar velDamping;
		Scalar lowerLimit;
		Scalar upperLimit;
    };
    
    //! A class that implements simplified creation of multi-body trees, using Roy Featherstone's algorithm.
    class FeatherstoneEntity : public Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the multibody
         \param totalNumOfLinks a number of links of the multibody
         \param baseSolid a pointer to the rigid body constituting the base of the multibody
         \param fixedBase a flag to designate if the multibody is fixed to the world or mobile
         */
        FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, bool fixedBase = false);
        
        //! A destructor.
        virtual ~FeatherstoneEntity();
        
        //! A method to define a new multibody link.
        /*!
         \param solid a pointer to a rigid body
         \param transform a position of the rigid body in the world frame
         */
        void AddLink(SolidEntity* solid, const Transform& transform);
        
        //! A method to define a new revolute joint.
        /*!
         \param name a name for the joint
         \param parent an id of the parent link
         \param child an id of the child link
         \param pivot the position of the joint pivot point
         \param axis the axis of the joint
         \param collide a flag to decide if the links connected by the joint should collide
         \return id of the joint
         */
        int AddRevoluteJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot, const Vector3& axis, bool collide = false);
        
        //! A method to define a new prismatic joint.
        /*!
         \param name a name for the joint
         \param parent an id of the parent link
         \param child an id of the child link
         \param axis the axis of the joint
         \param collide a flag to decide if the links conneted by the joint should collide
         \return id of the joint
         */
        int AddPrismaticJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& axis, bool collide = false);
        
        //! A method to define a new fixed joint.
        /*!
         \param name a name for the joint
         \param parent an id of the parent link
         \param child an id of the child link
         \param pivot the position of the joint pivot point
         \return id of the joint
         */
        int AddFixedJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot);
        
        //! A Method to add a joint motor.
        /*!
         \param index the id of the joint
         \param maxForceTorque the maximum value of the force [N] or torque [Nm] generated by the motor
         */
        void AddJointMotor(unsigned int index, Scalar maxForceTorque);
        
        //! A method to add limits to the joint.
        /*!
         \param index the id of the joint
         \param lower the lower limit of the joint ([m] or [rad])
         \param upper the upper limit of the joint ([m] or [rad])
         */
        void AddJointLimit(unsigned int index, Scalar lower, Scalar upper);
        
        //! A method to change the setpoint of the joint motor.
        /*!
         \param index the id of the joint
         \param pos the setpoint of position of the joint ([m] or [rad])
         \param kp the position control gain
         */
        void MotorPositionSetpoint(unsigned int index, Scalar pos, Scalar kp);
        
        //! A method to change the setpoint of the joint motor.
        /*!
         \param index the id of the joint
         \param vel the setpoint of velocity of the joint ([m/s] or [rad/s])
         \param kd the velocity control gain
         */
        void MotorVelocitySetpoint(unsigned int index, Scalar vel, Scalar kd);
        
        //! A method used to apply direct force or torque to the joint.
        /*!
         \param index the id of the joint
         \param forceTorque the value of the applied force [N] or torque [Nm]
         */
        void DriveJoint(unsigned int index, Scalar forceTorque);
        
        //! A method to apply gravity to the multibody.
        /*!
         \param g the gravity acceleration vector
         */
        void ApplyGravity(const Vector3& g);
        
        //! A method to apply damping to the multibody.
        void ApplyDamping();
        
        //! A method to add a force acting directly on a link.
        /*!
         \param index an id of the link
         \param F the value of the force [N]
         */
        void AddLinkForce(unsigned int index, const Vector3& F);
        
        //! A method to add a torque acting directly on a link.
        /*!
         \param index an id of the link
         \param tau the value of the torque [Nm]
         */
        void AddLinkTorque(unsigned int index, const Vector3& tau);
        
        //! A method used to compute accelerations of the links.
        /*!
         \param dt a step time of the simulation [s]
         */
        void UpdateAcceleration(Scalar dt);
        
        //! A method used to set the initial conditions for a joint.
        /*!
         \param index an id of the joint
         \param position the initial position ([m] or [rad])
         \param velocity the initial velocity ([m/s] or [rad/s])
         */
        void setJointIC(unsigned int index, Scalar position, Scalar velocity);
        
        //! A method used to set damping characteristics of the joint.
        /*!
         \param index an id of the joint
         \param constantFactor the value of the constant damping force [N] or torque [Nm]
         \param viscousFactor the value of the damping factor multiplied by velocity
         */
        void setJointDamping(unsigned int index, Scalar constantFactor, Scalar viscousFactor);
     
        //! A method returning the structure containing joint information.
        /*!
         \param index an id of the joint
         \return a structure containing joint information
         */
        FeatherstoneJoint getJoint(unsigned int index);
     
        //! A method returning the name of the joint.
        /*!
         \param index an id of the joint
         \return the name of the joint
         */
        std::string getJointName(unsigned int index);
        
        //! A method returning the position of the joint.
        /*!
         \param index an id of the joint
         \param position a reference to the variable which will store the position
         \param jointType a reference to a variable which will store the type of the joint
         */
        void getJointPosition(unsigned int index, Scalar& position, btMultibodyLink::eFeatherstoneJointType& jointType);
        
        //! A method returning the velocity of the joint.
        /*!
         \param index an id of the joint
         \param velocity a reference to the variable which will store the velocity
         \param jointType a reference to a variable which will store the type of the joint
         */
        void getJointVelocity(unsigned int index, Scalar& velocity, btMultibodyLink::eFeatherstoneJointType& jointType);
        
        //! A method returning the value of the directly applied torque.
        /*!
         \param index an id of the joint
         \return the sum of the directly applied torque
         */
        Scalar getJointTorque(unsigned int index);
        
        //! A method to change the maximum force/torque produced by the joint motor.
        /*!
         \param index the id of the joint
         \param tau the maximum value of the applied force [N] or torque [Nm] 
         */
        void setMaxMotorForceTorque(unsigned int index, Scalar maxT);
        
        //! A method returning the force or torque generated by the joint motor.
        /*!
         \param index an id of the joint
         \return the value of the force [N] or torque [Nm] generated by the joint motor
         */
        Scalar getMotorForceTorque(unsigned int index);
        
        //! A method returning the axis of the joint.
        /*!
         \param index an id of the joint
         \return a unit vector parallel to the joint axis
         */
        Vector3 getJointAxis(unsigned int index);
        
        /* COMMENT:
         * Both vectors are in the CoG frame of the child link.
         * The force is equal to the sum of reaction forces acting on the CoG.
         * The torque is calculated as a cross product of a vector from CoG to joint pivot and the force defined above.
         * The force acting on every link causes a reaction force and a torque on this link.
         * If the rection torque acts around the axis of the joint, then this torque does not transfer directly to previous joints (only force is transferred)
         */
        
        //! A method returning the joint feedback.
        /*!
         \param index an id of the joint
         \param force a reference to a variable that will store the forces acting on the joint [N]
         \param torque a reference to a variable that will store the torques acting on the joint [Nm]
         */
        unsigned int getJointFeedback(unsigned int index, Vector3& force, Vector3& torque);
        
        //! A method used to set the position of the multibody in the world frame.
        /*!
         \param trans the transformation of the multibody base in the world frame
         */
        void setBaseTransform(const Transform& trans);
        
        //! A method used to set if the multibody base should be rendered.
        /*!
         \param render a flag to set if the base should be rendered
         */
        void setBaseRenderable(bool render);
        
        //! A method returning the structure containing link information.
        /*!
         \param index an id of the link
         \return a structure containing link information
         */
        FeatherstoneLink getLink(unsigned int index);
        
        //! A method returning the world frame position of the link.
        /*!
         \param index an id of the link
         \return a position of the link in the world frame
         */
        Transform getLinkTransform(unsigned int index);
        
        //! A method returning the linear velocity of the link.
        /*!
         \param index an id of the link
         \return the linear velocity of the link [m/s]
         */
        Vector3 getLinkLinearVelocity(unsigned int index);
        
        //! A method returning the angular velocity of the link.
        /*!
         \param index an id of the link
         \return the angular velocity of the link [rad/s]
         */
        Vector3 getLinkAngularVelocity(unsigned int index);
        
        //! A method returning the number of joints.
        unsigned int getNumOfJoints();
        
        //! A method returning the number of mobile joints.
        unsigned int getNumOfMovingJoints();
        
        //! A method returning the number of multibody links.
        unsigned int getNumOfLinks();
        
        //! A method returning a pointer to the Bullet multibody object.
        btMultiBody* getMultiBody();
        
        //! A method used to set if self collision is enabled for the whole multibody.
        void setSelfCollision(bool enabled);

        //! A method informing if the self collisions are enabled for the whole multibody.
        bool hasSelfCollision() const;

        //! A method used to set the display mode of the links.
        /*!
         \param m a flag that defines the display style.
         */
        void setDisplayMode(DisplayMode m);
        
        //! A method used to add the multibody to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
        //! A method used to add the multibody to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         \param origin the transformation of the multibody base in the world frame
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method used to remove the multibody from the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void RemoveFromSimulation(SimulationManager* sm);

        //! A method used to respawn the multibody at new position.
        /*!
         \param origin the transformation of the multibody base in the world frame
         */
        void Respawn(const Transform& origin);
        
        //! A method implementing the rendering of the multibody.
        std::vector<Renderable> Render();
        
        //! A method returning the extents of the body axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max);
        
        //! A method returning the type of the entity.
        EntityType getType() const;
        
    private:
        btMultiBody* multiBody;
        std::vector<FeatherstoneLink> links;
        std::vector<FeatherstoneJoint> joints;
        bool baseRenderable;
    };
}

#endif
