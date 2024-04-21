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
//  Created by Patryk Cieslak on 29/12/12.
//  Copyright(c) 2012-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SolidEntity__
#define __Stonefish_SolidEntity__

#include "BulletDynamics/Featherstone/btMultiBodyLinkCollider.h"
#include "core/MaterialManager.h"
#include "entities/MovingEntity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum designating the type of solid that the body represents.
    enum class SolidType {POLYHEDRON, SPHERE, CYLINDER, BOX, TORUS, COMPOUND, WING};
    //! An enum designating the type of geometry approximation used for fluid dynamics coefficients predicition.
    enum class GeometryApproxType {AUTO, SPHERE, CYLINDER, ELLIPSOID};
    //! An enum used to define if the body is submerged.
    enum class BodyFluidPosition {INSIDE, OUTSIDE, CROSSING_SURFACE};
    //! An enum defining what is the medium in which the body moves (affects which forces are computed, needed because it is not possible to change mass during simulation).
    /*!
     DISABLED -> no computation of physics, zero mass and inertia
     SURFACE -> no aerodynamics or hydrodynamics
     FLOATING -> hydrodynamics with buoyancy
     SUBMERGED -> hydrodynamics with buoyancy and added mass
     AERODYNAMIC -> aerodynamics
    */
    enum class BodyPhysicsMode {DISABLED, SURFACE, FLOATING, SUBMERGED, AERODYNAMIC};
    //! A structure defining the physics computation settings for the body.
    struct BodyPhysicsSettings
    {
        BodyPhysicsMode mode;
        bool collisions;
        bool buoyancy;

        BodyPhysicsSettings() : mode(BodyPhysicsMode::SUBMERGED), collisions(true), buoyancy(true)
        {
        }
    };

    struct HydrodynamicsSettings;
    class Ocean;
    class Atmosphere;
    
    //! An abstract class representing a rigid body.
    class SolidEntity : public MovingEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param phy the specific settings of the physics computation for the body
         \param material the name of the material the body is made of
         \param look the name of the graphical material used for rendering
         \param thickness if positive the body is considered a shell instead of a solid
         */
        SolidEntity(std::string uniqueName, BodyPhysicsSettings phy, std::string material, std::string look, Scalar thickness);
        
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
        
        //! A method that corrects damping forces based on geometry approximation
        /*!
         \param ocn a pointer to the fluid entity generating forces (currently only Ocean supported)
         \param _Fdq the form drag force [N]
         \param _Tdq the torque induced by the form drag force [Nm]
         \param _Fdf the skin friction force [N]
         \param _Tdf the torque induced by the skin friction force [Nm]
        */
        void CorrectHydrodynamicForces(Ocean* ocn, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf);
        
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
         \param _Fdq output of the damping force resulting from form drag
         \param _Tdq output of the torque induced by form drag
         \param _Fdf output of the damping force resulting from skin friction
         \param _Tdf output of the torque induced by skin friction
         \param _Swet output of the wetted surface area
         \param _Vsub output of the submerged volume
         \param debug output of the debug rendering
        */
        static void ComputeHydrodynamicForcesSurface(const HydrodynamicsSettings& settings, const Mesh* mesh, Ocean* liquid, const Transform& T_CG, const Transform& T_C,
                                                     const Vector3& linearV, const Vector3& angularV, Vector3& _Fb, Vector3& _Tb, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf, 
                                                     Scalar& _Swet, Scalar& _Vsub, Renderable& debug);
        
        //! A static method that computes fluid dynamics when a body is completely submerged.
        /*!
         \param mesh a pointer to the body physics mesh data
         \param liquid a pointer to the fluid entity generating forces
         \param T_CG a transform from the world frame to the body CG frame
         \param T_C a transform from the world frame to the body physics frame
         \param linearV the linear velocity of the body in the world frame
         \param angularV the angular velocity of the body in the world frame
         \param _Fdq output of the damping force resulting from form drag
         \param _Tdq output of the torque induced by form drag
         \param _Fdf output of the damping force resulting from skin friction
         \param _Tdf output of the torque induced by skin friction
        */
        static void ComputeHydrodynamicForcesSubmerged(const Mesh* mesh, Ocean* liquid, const Transform& T_CG, const Transform& T_C,
                                                       const Vector3& linearV, const Vector3& angularV, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf);
        
        //! A method that computes aerodynamics.
        /*!
         \param atm a pointer to the atmosphere entity
        */
        virtual void ComputeAerodynamicForces(Atmosphere* atm);
        
        //! A method that corrects damping forces based on geometry shape approximation
        /*!
         \param atm a pointer to the atmosphere object
         \param _Fda input/output of the damping force resulting from pressure drag
         \param _Tda input/output of the torque induced by the pressure drag
        */ 
        void CorrectAerodynamicForces(Atmosphere* atm, Vector3& _Fda, Vector3& _Tda);
         
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

        //! A method used to set hydrodynamic coeffcients.
        /*!
         \param Cd a vector of form quadratic drag (quadratic drag) coefficients 
         \param Cf a vector of skin friction (viscous drag) coefficients
         */
        void SetHydrodynamicCoefficients(const Vector3& Cd, const Vector3& Cf);
        
        //! A method to set the body pose in the world frame.
        void setCGTransform(const Transform& trans);
        
        //! A method returning the type of the entity.
        EntityType getType() const;
        
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
        
        //! A method returning the transformation from body origin to the geometry approximation origin.
        Transform getO2HTransform() const;
        
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

        //! A method returning the force applied to the body.
        Vector3 getAppliedForce();

        //! A method returning the hydrodynamic forces computed for the body.
        /*!
         \param Fb the buoyancy force [N]
         \param Tb the buoyancy induced torque [Nm]
         \param Fd the form drag force [N]
         \param Td the form drag induced torque [Nm]
         \param Ff the skin friction force [N]
         \param Tf the skin friction induced torque [Nm]
         */
        void getHydrodynamicForces(Vector3& Fb, Vector3& Tb, Vector3& Fd, Vector3& Td, Vector3& Ff, Vector3& Tf);
        
        //! A method returning the hydrodynamic coefficents for the body.
        /*!
         \param Cd a vector of form quadratic drag (quadratic drag) coefficients 
         \param Cf a vector of skin friction (viscous drag) coefficients
         */
        void getHydrodynamicCoefficients(Vector3& Cd, Vector3& Cf) const;

        //! A method returning the wetted surface area of the body.
        Scalar getWettedSurface() const;

        //! A method returning the submerged volume of the body.
        Scalar getSubmergedVolume() const;

        //! A method returning the mass of the body.
        Scalar getMass() const;
        
        //! A method returning the inertia of the body.
        Vector3 getInertia() const;
        
        //! A method returning the mass or the sum of mass and added mass (depending on type of body).
        virtual Scalar getAugmentedMass() const;
        
        //! A method returning the inertia or the sum of inertia and added mass (depending on type of body).
        virtual Vector3 getAugmentedInertia() const;
      
		//! A method returning the hydrodynamic added mass (diagonal elements).
		Vector3 getAddedMass() const;
	  
		//! A method returning the hydrodynamic added inertia (diagonal elements).
		Vector3 getAddedInertia() const;
        
        //! A method returning the volume of the body.
        Scalar getVolume() const;

        //! A method returning the surface area of the body.
        Scalar getSurface() const;
        
        //! A method returning the parameters of the approximation of body shape
        /*!
         \param type the type of the approximation geometry
         \param params the parameters of the approximation geometry
        */
        void getGeometryApprox(GeometryApproxType& type, std::vector<Scalar>& params) const;
        
        //! A method returning a pointer to the physics mesh.
        const Mesh* getPhysicsMesh();

        //! A method that returns a copy of all physics mesh vertices in body origin frame.
        virtual std::vector<Vector3>* getMeshVertices() const;
        
        //! A method informing if the body is using buoyancy computation.
        bool isBuoyant() const;
        
        //! A method informing what kind of physics computations are performed for the body.
        BodyPhysicsMode getBodyPhysicsMode() const;
        
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
        
        //! A method used to set if the body CG should be rendered.
        void setDisplayCoordSys(bool enabled);
        
        //! A method returning the index of the physical object used in rendering.
        int getPhysicalObject() const;
        
    protected:
        BodyFluidPosition CheckBodyFluidPosition(Ocean* ocn);
        void ComputeFluidDynamicsApprox(GeometryApproxType t);
        void ComputeSphericalApprox();
        void ComputeCylindricalApprox();
        void ComputeEllipsoidalApprox();
        
        Scalar LambKFactor(Scalar r1, Scalar r2);
        virtual void BuildRigidBody();
        void BuildMultibodyLinkCollider(btMultiBody* mb, unsigned int child, btMultiBodyDynamicsWorld* world);
        
        //Body
        btMultiBodyLinkCollider* multibodyCollider;
        
        Mesh* phyMesh; //Mesh used for physics calculation
        Scalar thick;
        Scalar volume;
        Scalar surface;
        
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
        Transform T_O2H; //Transform between body origin and geometry approximation origin
        
        Vector3 aMass; //Hydrodynamic added mass
		Vector3 aI; //Hydrodynamic added inertia
        GeometryApproxType fdApproxType;
        std::vector<Scalar> fdApproxParams;
        Vector3 fdCd;
        Vector3 fdCf;
        Transform T_CG2H; //Transform between CG and hydrodynamic proxy frame
        
        BodyPhysicsSettings phy;
        Vector3 Fb;
        Vector3 Tb;
        Vector3 Fdq;
        Vector3 Tdq;
        Vector3 Fdf;
        Vector3 Tdf;
        Scalar Swet; //Wetted surface of the body
        Scalar Vsub; //Submerged part of body
        
        Vector3 Fda;
        Vector3 Tda;
        
        //Motion
        Vector3 lastV;
        Vector3 lastOmega;
        
        //Display
        int phyObjectId;
        Renderable submerged;
        
    private:
        friend class FeatherstoneEntity;
        friend class FixedJoint;
        friend class SpringJoint;
        friend class RevoluteJoint;
        friend class PrismaticJoint;
        friend class CylindricalJoint;
        friend class SphericalJoint;
        friend class GearJoint;
        friend class BeltJoint;
    };
}

#endif
