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
//  SolidEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright(c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SolidEntity__
#define __Stonefish_SolidEntity__

#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include "core/MaterialManager.h"
#include "entities/Entity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum designating the type of solid that the body represents.
    typedef enum {SOLID_POLYHEDRON = 0, SOLID_SPHERE, SOLID_CYLINDER, SOLID_BOX, SOLID_TORUS, SOLID_COMPOUND, SOLID_WING} SolidType;
    //! An enum designating the type of proxy shape used for fluid dynamics coefficient approximation.
    typedef enum {FD_PROXY_NONE = 0, FD_PROXY_SPHERE, FD_PROXY_CYLINDER, FD_PROXY_ELLIPSOID} FluidDynamicsProxyType;
    //! An enum used to define if the body is submerged.
    typedef enum {INSIDE_FLUID = 0, OUTSIDE_FLUID, CROSSING_FLUID_SURFACE} BodyFluidPosition;
    //! An enum defining what is the medium in which the body moves (affects which forces are computed, needed because it is not possible to change mass during simulation)
    /*!
     SURFACE_BODY -> no aerodynamics or hydrodynamics
     FLOATING_BODY -> hydrodynamics with buoyancy
     SUBMERGED_BODY -> hydrodynamics with buoyancy and added mass
     AERODYNAMIC_BODY -> aerodynamics
    */
    typedef enum {SURFACE_BODY = 0, FLOATING_BODY, SUBMERGED_BODY, AERODYNAMIC_BODY} BodyPhysicsType;
    
    struct HydrodynamicsSettings;
    class Ocean;
    class Atmosphere;
    
    //! An abstract class representing a rigid body.
    class SolidEntity : public Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param m a metarial the body is made of
         \param bpt an enum defining the type of physics computations required for the body (currently bodies cannot transfer between mediums)
         \param lookId an index of the graphical material
         \param thickness body wall thickness, if provided the body is considered a shell
         \param isBuoyant a flag to enable computation of buoyancy force
         */
        SolidEntity(std::string uniqueName, Material m, BodyPhysicsType bpt, int lookId, Scalar thickness, bool isBuoyant);
        
        //! A destructor.
        virtual ~SolidEntity();
        
        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         \param origin a pose of the body CG in the world frame
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method removing the body from the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         */
        void RemoveFromSimulation(SimulationManager* sm);
        
        //! A pure virtual method building a collision shape for the body.
        virtual btCollisionShape* BuildCollisionShape() = 0;
        
        //! A pure virtual method returning the type of the solid that the body represents.
        virtual SolidType getSolidType() = 0;
        
        //! A method updating the acceleration of the body.
        /*!
         \param dt a time step of the simulation
         */
        void UpdateAcceleration(Scalar dt);
        
        //! A method that computes fluid dynamics based on selected settings.
        /*!
         \param settings a structure holding settings of fluid dynamics computation
         \param ocn a pointer to the ocean entity
         */
        virtual void ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn);
        
        //! A static method that computes fluid dynamics when a body is crossing the fluid surface.
        /*!
         \param settings a reference to a structure holding settings of the fluid dynamics computation
         \param mesh a pointer to the body physics mesh data
         \param liquid a pointer to the fluid entity generating forces (currently only Ocean supported)
         \param T_CG a transform from the world frame to the body CG frame
         \param T_C a transform from the world frame to the physics frame
         \param linearV the linear velocity of the body in the world frame
         \param angularV the angular velocity of the body in the world frame
         \param _Fb output of the buoyancy force
         \param _Tb output of the torque induced by buoyancy force
         \param _Fds output of the damping force resulting from skin drag
         \param _Tds output of the torque induced by skin drag
         \param _Fdp output of the damping force resulting from pressure drag
         \param _Tdp output of the torque induced by pressure drag
         */
        static void ComputeHydrodynamicForcesSurface(const HydrodynamicsSettings& settings, const Mesh* mesh, Ocean* liquid, const Transform& T_CG, const Transform& T_C,
                                                     const Vector3& linearV, const Vector3& angularV, Vector3& _Fb, Vector3& _Tb, Vector3& _Fds, Vector3& _Tds, Vector3& _Fdp, Vector3& _Tdp);
        
        //! A static method that computes fluid dynamics when a body is completely submerged.
        /*!
         \param mesh a pointer to the body physics mesh data
         \param liquid a pointer to the fluid entity generating forces
         \param T_CG a transform from the world frame to the body CG frame
         \param T_C a transform from the world frame to the body physics frame
         \param linearV the linear velocity of the body in the world frame
         \param angularV the angular velocity of the body in the world frame
         \param _Fds output of the damping force resulting from skin drag
         \param _Tds output of the torque induced by skin drag
         \param _Fdp output of the damping force resulting from pressure drag
         \param _Tdp output of the torque induced by pressure drag
         */
        static void ComputeHydrodynamicForcesSubmerged(const Mesh* mesh, Ocean* liquid, const Transform& T_CG, const Transform& T_C,
                                                       const Vector3& linearV, const Vector3& angularV, Vector3& _Fds, Vector3& _Tds, Vector3& _Fdp, Vector3& _Tdp);
        
        //! A method that computes aerodynamics.
        /*!
         \param atm a pointer to the atmosphere entity
        */
        virtual void ComputeAerodynamicForces(Atmosphere* atm);
         
        //! A static method that computes aerodynamics for a body
        /*!
         \param mesh a pointer to the body physics mesh data
         \param atm a pointer to the atmosphere object
         \param T_CG a transform from the world frame to the body CG frame
         \param T_C a transform from the world frame to the body physics frame
         \param linearV the linear velocity of the body in the world frame
         \param angularV the angular velocity of the body in the world frame
         \param _Fda output of the damping force resulting from pressure drag
         \param _Tda output of the torque induced by pressure drag
         */
        static void ComputeAerodynamicForces(const Mesh* mesh, Atmosphere* atm, const Transform& T_CG, const Transform& T_C,
                                             const Vector3& linearV, const Vector3& angularV, Vector3& _Fda, Vector3& _Tda);
        
        //! A method which applies given force to the body CG.
        /*!
         \param force a force to be applied to the body, in the world frame
         */
        void ApplyCentralForce(const Vector3& force);
        
        //! A method which applies given torque to the body.
        /*!
         \param torque a torque to be applied to the body, in the world frame
         */
        void ApplyTorque(const Vector3& torque);
        
        //! A method which applies gravity to the body.
        /*!
         \param g a vector specifing the gravitational acceleration
         */
        void ApplyGravity(const Vector3& g);
        
        //! A method which applies precomputed hydrodynamic forces to the body.
        virtual void ApplyHydrodynamicForces();
        
        //! A method which applies precomputed aerodynamic forces to the body.
        virtual void ApplyAerodynamicForces();
        
        //! A method used to scale the inertia of the body by comparing actual and given mass.
        /*!
         \param mass a new mass of the body, used to compute scaling factor
         */
        void ScalePhysicalPropertiesToArbitraryMass(Scalar mass);
        
        //! A method used to set arbitrary physical properties of the body.
        /*!
         \param mass a new mass of the body
         \param inertia a vector of new inertia moments of the body
         \param CG a transformation between the body origin frame and the new CG frame
         */
        virtual void SetArbitraryPhysicalProperties(Scalar mass, const Vector3& inertia, const Transform& CG);
        
        //! A method used to enable compliant contact and set the contact response.
        /*!
         \param soft a flag to designate if compliant contact should be used
         \param stiffness a stiffness coefficient used when simulating compliant contact [N m^-1]
         \param damping a damping coefficient used when simulating compliant contact [N s m^-1]
         */
        void SetContactProperties(bool soft, Scalar stiffness = Scalar(0), Scalar damping = Scalar(0));
        
        //! A method used to set arbitrary hydrodynamic coefficients and CB of the body.
        /*!
         \param addedMass a complete added mass matrix (only diagonal element are used)
         \param damping a matrix of damping factors
         \param G2CB a transformation between the geometry frame and the CB frame
         */
        //void SetHydrodynamicProperties(const Matrix6Eigen& addedMass, const Matrix6Eigen& damping, const Transform& G2CB);
        
        //! A method to set the body pose in the world frame.
        void setCGTransform(const Transform& trans);
        
        //! A method returning the type of the entity.
        EntityType getType();
        
        //! A method returning the transformation from the CG frame to the graphics mesh origin.
        Transform getCG2GTransform() const;
        
        //! A method returning the transformation from the CG frame to the physics mesh origin.
        Transform getCG2CTransform() const;
        
        //! A method returning the transformation from the CG frame to the body origin.
        Transform getCG2OTransform() const;
        
        //! A method returning the position of the CB in the CG frame.
        Vector3 getCB() const;
        
        //! A method returning the transformation from body origin to the physics mesh origin.
        Transform getO2CTransform() const;
        
        //! A method returning the transformation from body origin to the graphics mesh origin.
        Transform getO2GTransform() const;
        
        //! A method returning the pose of the body in the world frame.
        Transform getCGTransform() const;
        
        //! A method returning the pose of the graphics mesh in the world frame (rendering).
        Transform getGTransform() const;
        
        //! A method returning the pose of the physics mesh in the world frame (hydrodynamics).
        Transform getCTransform() const;
        
        //! A method returning the pose of the hydro proxy origin in the world frame.
        Transform getHTransform() const;
        
        //! A method returning the pose of the body origin in the world frame.
        Transform getOTransform() const;
        
        //! A method returning the linear velocity of the body.
        Vector3 getLinearVelocity() const;
        
        //! A method returning the linear velocity of the body at the given point.
        Vector3 getLinearVelocityInLocalPoint(const Vector3& relPos) const;
        
        //! A method returning the angular velocity of the body.
        Vector3 getAngularVelocity() const;
        
        //! A method returning the linear acceleration of the body.
        Vector3 getLinearAcceleration() const;
        
        //! A method returning the angular acceleration of the body.
        Vector3 getAngularAcceleration() const;
        
        //! A method returning the mass of the body.
        Scalar getMass() const;
        
        //! A method returning the inertia of the body.
        Vector3 getInertia() const;
        
        //! A method returning the mass or the sum of mass and added mass (depending on type of body).
        Scalar getAugmentedMass() const;
        
        //! A method returning the inertia or the sum of inertia and added mass (depending on type of body).
        Vector3 getAugmentedInertia() const;
        
        //! A method returning the hydrodynamic added mass of the body.
        Matrix6Eigen getAddedMass() const;
        
        //! A method returning the material of the body.
        Material getMaterial() const;
        
        //! A method returning the volume of the body.
        Scalar getVolume() const;
        
        //! A method returning a copy of the vertices of the physics mesh in physics frame (memory needs to be released manually!).
        virtual std::vector<Vertex>* getMeshVertices();
        
        //! A method returning a pointer to the physics mesh.
        const Mesh* getPhysicsMesh();
        
        //! A method informing if the body is using buoyancy computation.
        bool isBuoyant() const;
        
        //! A method informing what kind of physics computations are performed for the body.
        BodyPhysicsType getBodyPhysicsType() const;
        
        //Rendering
        //! A method used to build the graphical representation of the body.
        virtual void BuildGraphicalObject();
        
        //! A method returning the elements that should be rendered.
        virtual std::vector<Renderable> Render();
        
        //! A method returning the extents of the body axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max);
        
        //! A method used to change the rendering style of the object.
        /*!
         \param newLookId an index of the graphical material that should be used to render the body
         */
        void setLook(int newLookId);
        
        //! A method used to set if the body CG should be rendered.
        void setDisplayCoordSys(bool enabled);
        
        //! A method returning the index of the graphical material used in rendering.
        int getLook() const;
        
        //! A method returning the index of the graphical object used in rendering.
        int getObject() const;
        
    protected:
        BodyFluidPosition CheckBodyFluidPosition(Ocean* ocn);
        void ComputeFluidDynamicsProxy(FluidDynamicsProxyType t);
        void ComputeProxySphere();
        void ComputeProxyCylinder();
        void ComputeProxyEllipsoid();
        void CorrectHydrodynamicForces(Ocean* ocn);
        void CorrectAerodynamicForces(Atmosphere* atm);
        static void ComputeDampingForces(Vector3 vc, Vector3 fn, Scalar A, Vector3& linear, Vector3& quadratic);
        static void ComputePhysicalProperties(Mesh* mesh, Scalar wallThickness, Material mat, Vector3& CG, Scalar& volume, Vector3& Ipri, Matrix3& Irot);
        
        Scalar LambKFactor(Scalar r1, Scalar r2);
        virtual void BuildRigidBody();
        void BuildMultibodyLinkCollider(btMultiBody* mb, unsigned int child, btMultiBodyDynamicsWorld* world);
        
        //Rigid body
        btRigidBody* rigidBody;
        btMultiBodyLinkCollider* multibodyCollider;
        
        Mesh *phyMesh; //Mesh used for physics calculation
        Material mat;
        Scalar thick;
        Scalar volume;
        
        Scalar mass;  //Mass of solid
        Vector3 Ipri; //Principal moments of inertia
        Scalar contactK; //Contact stiffness
        Scalar contactD; //Contact damping
        
        //CG is the point important for the simulation
        Transform T_CG2C; //Transform between CG and physics origin
        Transform T_CG2G; //Transform between CG and graphics origin
        Transform T_CG2O; //Transform between CG and body origin
        Vector3 P_CB; //Center of Buoyancy (in body CG frame)
        
        //O is a point important for building models
        Transform T_O2G; //Transform between body origin and graphics origin
        Transform T_O2C; //Transform between body origin and physics origin
        
        Matrix6Eigen aMass; //Hydrodynamic added mass matrix
        FluidDynamicsProxyType fdProxyType;
        std::vector<Scalar> fdProxyParams;
        Transform T_CG2H; //Transform between CG and hydrodynamic proxy frame
        
        BodyPhysicsType phyType;
        bool buoyant;
        Vector3 Fb;
        Vector3 Tb;
        Vector3 Fds;
        Vector3 Tds;
        Vector3 Fdp;
        Vector3 Tdp;
        Vector3 Fda;
        Vector3 Tda;
        
        //Motion
        Vector3 filteredLinearVel;
        Vector3 filteredAngularVel;
        Vector3 linearAcc;
        Vector3 angularAcc;
        
        //Display
        int lookId;
        int graObjectId;
        int phyObjectId;
        
    private:
        friend class FeatherstoneEntity;
        friend class FixedJoint;
        friend class RevoluteJoint;
        friend class PrismaticJoint;
        friend class CylindricalJoint;
        friend class SphericalJoint;
        friend class GearJoint;
        friend class BeltJoint;
    };
}

#endif
