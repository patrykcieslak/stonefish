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
//  SimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationManager__
#define __Stonefish_SimulationManager__

#include "StonefishCommon.h"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"
#include "entities/SolidEntity.h"
#include "utils/PerformanceMonitor.h"
#include "BulletSoftBody/btSoftMultiBodyDynamicsWorld.h"

namespace sf
{
    class NameManager;
    class MaterialManager;
    class Console;
    class NED;
    class Robot;
    class Entity;
    class StaticEntity;
    class AnimatedEntity;
    class FeatherstoneEntity;
    class Joint;
    class Actuator;
    class Sensor;
    class Comm;
    class Contact;
    class OpenGLTrackball;
    class OpenGLDebugDrawer;
    
    //! An enum designating the type of solver used for physics computation
    enum class Solver {SI, DANTZIG, PGS, LEMKE, NNCG};
    
    //! An enum designating the approach to collision detection
    enum class CollisionFilter {INCLUSIVE, EXCLUSIVE};
    
    //! A structure used to define collision pairs
    struct Collision
    {
        Entity* A;
        Entity* B;
    };
    
    //! An abstract class managing the simulation world, the solver settings and implementing custom physics callbacks.
    class SimulationManager
    {
        friend class OpenGLPipeline;
        
    public:
        //! A constructor.
        /*!
         \param stepsPerSecond number of simulation steps per second (inverse of sample time)
         \param st type of solver that should be used
         \param cft type of collision filtering used
         \param ht type of hydrodynamics computations
         */
        SimulationManager(Scalar stepsPerSecond = Scalar(60), Solver st = Solver::SI, CollisionFilter cft = CollisionFilter::EXCLUSIVE);
        
        //! A destructor.
        virtual ~SimulationManager();
        
        //! A method used to construct simulation scenario. This has to be implemented by the subclass.
        virtual void BuildScenario() = 0;
        
        //! A method called after the simulation step is completed. Useful to implement interaction with outside code.
        /*!
         \param timeStep amount of time that passed in the simulation world
         */
        virtual void SimulationStepCompleted(Scalar timeStep);

        //! A method returning the current simulation clock time in us (overriding allows for external time source).
        virtual uint64_t getSimulationClock() const;

        //! A method sleeping for a given simulation clock time (overriding allows for external time source).
        /*!
         \param us time to sleep in simulation time [us]
         */
        virtual void SimulationClockSleep(uint64_t us);

        //! A method solving the initial conditions problem.
        bool SolveICProblem();
        
        //! A method cleaning the simulation world.
        virtual void DestroyScenario();
        
        //! A method which starts the simulation.
        bool StartSimulation();
        
        //! A method which stops the simulation.
        void StopSimulation();
        
        //! A method that resumes the paused simulation.
        void ResumeSimulation();
        
        //! A method which restarts the simulation.
        void RestartScenario();
        
        //! A method that steps the simulation based on real time.
        void AdvanceSimulation();

        //! A method that performs on simulation step of specified period.
        void StepSimulation(Scalar timeStep);
        
        //! A method updating the drawing queue (thread safe)
        void UpdateDrawingQueue();
        
        //! A method that adds any type of entity to the simulation world.
        /*!
         \param ent a pointer to the entity
         */
        void AddEntity(Entity* ent);
        
        //! A method that adds a robotic system to the simulation world.
        /*!
         \param robot a pointer to the robot object
         \param origin a pose of the robot in the world frame
         */
        void AddRobot(Robot* robot, const Transform& origin);
        
        //! A method that adds a static body to the simulation world.
        /*!
         \param ent a pointer to the static body object
         \param origin a pose of the body in the world frame
         */
        void AddStaticEntity(StaticEntity* ent, const Transform& origin);
        
        //! A method that adds an animated rigid body to the simulation world.
        /*!
         \param ent a pointer to the animated object
         */
        void AddAnimatedEntity(AnimatedEntity* ent);
        
        //! A method that adds a dynamic rigid body to the simulation world.
        /*!
         \param ent a pointer to the dynamic body object
         \param origin a pose of the body in the world frame
         */
        void AddSolidEntity(SolidEntity* ent, const Transform& origin);

        //! A method that removes a dynamic rigid body from the simulation world.
        /*!
         \param ent a pointer to the dynamic body object
         */
        void RemoveSolidEntity(SolidEntity* ent);

        //! A method that adds a rigid multibody to the simulation world.
        /*!
         \param ent a pointer to the multibody object
         \param origin a pose of the multibody base link in the world frame
         */
        void AddFeatherstoneEntity(FeatherstoneEntity* ent, const Transform& origin);

        //! A method that removes a rigid multibody from the simulation world.
        /*!
         \param ent a pointer to the multibody object
         */
        void RemoveFeatherstoneEntity(FeatherstoneEntity* ent);
        
        //! A method that adds a discrete joint to the simulation world.
        /*!
         \param jnt a pointer to the joint object
         */
        void AddJoint(Joint* jnt);

        //! A method that removes a discrete joint from the simulation world.
        /*!
         \param jnt a pointer to the joint object
         */
        void RemoveJoint(Joint* jnt);
        
        //! A method that adds an actuator to the simulation world.
        /*!
         \param act a pointer to the actuator object
         */
        void AddActuator(Actuator* act);
        
        //! A method that adds a sensor to the simulation world.
        /*!
         \param sens a pointer to the sensor object
         */
        void AddSensor(Sensor* sens);
        
        //! A method that adds a communication device to the simulation world.
        /*!
         \param comm a pointer to the comm object
         */
        void AddComm(Comm* comm);
        
        //! A method that adds contact monitoring between two entities.
        /*!
          \param a pointer to the contact object
         */
        void AddContact(Contact* cnt);
        
        //! A method that enables collision between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         */
        void EnableCollision(const Entity* entA, const Entity* entB);
        
        //! A method that disables collision between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         */
        void DisableCollision(const Entity* entA, const Entity* entB);
        
        //! A method that checks if collision is enabled between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         \return
         */
        int CheckCollision(const Entity* entA, const Entity* entB);
        
        //! A method used to enable ocean simulation.
        /*!
         \param waves the state of the ocean (waves enabled when >0)
         \param f a pointer to a liquid that will feel the ocean (if left blank defaults to water)
         */
        void EnableOcean(Scalar waves = Scalar(0), Fluid f = Fluid());
        
        //! A method used to enable atmosphere simulation.
        void EnableAtmosphere();
        
        //! A method used to pick an entity by shooting a camera ray.
        /*!
         \param eye the position of the camera eye in the world frame
         \param ray a unit vector representing the ray generated in the world frame
         \return a pointer to the hit entity and the index of the child collision shape
         */
        std::pair<Entity*, int> PickEntity(Vector3 eye, Vector3 ray);
        
        //! A method that sets new valve for the amount of simulation steps in a second.
        /*!
         \param steps number steps of simulation per second
         */
        void setStepsPerSecond(Scalar steps);

        //! A method to set a flag that enables automatic calling of the SimulationStepCompleted method.
        void setCallSimulationStepCompleted(bool call);

        //! A method that directly sets the fluid dynamics prescaler.
        /*!
         \param presc a prescaler used to compute the update frequency of fluid dynamics computations
         */
        void setFluidDynamicsPrescaler(unsigned int presc);

        //! A method that sets how simulation time relates to real time.
        /*!
         \param f a multiple of real time (1.0 = real time)
         */
        void setRealtimeFactor(Scalar f);
        
        //! A method used to setup the initial conditions solver.
        /*!
         \param useGravity specifies if gravity should be enabled during IC solving
         \param timeStep a time step used during IC solving
         \param maxIterations a maximum number of iterations simulated
         \param maxTime a maximum time of solving
         \param linearTolerance a tolerance of change of position between two steps
         \param angularTolerance a tolerance of change of angles between two steps
         */
        void setICSolverParams(bool useGravity, Scalar timeStep = Scalar(0.001), unsigned int maxIterations = 100000,
                               Scalar maxTime = BT_LARGE_FLOAT, Scalar linearTolerance = Scalar(1e-6), Scalar angularTolerance = Scalar(1e-6));
        
        //! A method used to change some global solver params for stability tuning.
        /*!
         \param erp error reduction for constraint solving
         \param stopErp error reduction for constraint limit solving
         \param erp2 error reduction for contact solving
         \param globalDamping damping added globally to all dynamic bodies
         \param globalFriction friction added globally to all dynamic bodies
         \param linearSleepingThreshold a linear velocity below which the dynamic bodies will sleep [m/s]
         \param angularSleepingThreshold an angular velocity below which the dynamic bodies will sleep [rad/s]
        */
        void setSolverParams(Scalar erp, Scalar stopErp, Scalar erp2, Scalar globalDamping, Scalar globalFriction, 
                                Scalar linearSleepingThreshold, Scalar angularSleepingThreshold);

        //! A method that sets the display mode of dynamical rigid bodies.
        /*!
         \param m a flag that defines the display style of dynamical bodies
         */
        void setSolidDisplayMode(DisplayMode m);
        
        //! A method that returns the display style of dynamical podies.
        /*!
         \return flag defining the display style
         */
        DisplayMode getSolidDisplayMode() const;
        
        //! A method returning the usage of the CPU by the physics computation in percent.
        Scalar getCpuUsage() const;
        
        //! A method returning the current number of steps per second used.
        Scalar getStepsPerSecond() const;

        //! A method returning the flag that enables automatic calling of the SimulationStepCompleted method.
        bool getCallSimulationStepCompleted() const;
        
        //! A method returning the axis-aligned bounding box of the simulation world.
        /*!
         \param min a position of the minimum corner
         \param max a position of the maximum corner
         */
        void getWorldAABB(Vector3& min, Vector3& max);
        
        //! A method returning the collision filtering used in simulation.
        CollisionFilter getCollisionFilter() const;
        
        //! A method returning the type of solver used.
        Solver getSolver() const;

        //! A method returning soft body world information.
        btSoftBodyWorldInfo& getSoftBodyWorldInfo();
        
        //! A method returning a robot by index
        /*!
         \param index an id of the robot
         \return a pointer to a robot object
         */
        Robot* getRobot(unsigned int index);
        
        //! A method returning a robot by name.
        /*!
         \param name a name of the robot
         \return a pointer to a robot object
         */
        Robot* getRobot(const std::string& name);
        
        //! A method returning an entity by index.
        /*!
         \param index an id of the entity
         \return a pointer to an entity object
         */
        Entity* getEntity(unsigned int index);
        
        //! A method returning an entity by name.
        /*!
         \param name a name of the entity
         \return a pointer to an entity object
         */
        Entity* getEntity(const std::string& name);
        
        //! A method returning a joint by index.
        /*!
         \param index an id of the joint
         \return a pointer to an joint object
         */
        Joint* getJoint(unsigned int index);
        
        //! A method returning a joint by name.
        /*!
         \param name a name of the joint
         \return a pointer to a joint object
         */
        Joint* getJoint(const std::string& name);
        
        //! A method returning a contact by index.
        /*!
         \param index an id of the contact
         \return a pointer to a contact object
         */
        Contact* getContact(unsigned int index);
        
        //! A method returning a contact by name.
        /*!
         \param name a name of the contact
         \return a pointer to a contact object
         */
        Contact* getContact(const std::string& name);
        
        //! A method returning a contavt by entity pair.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the sencond entity
         \return a pointer to a contact object
         */
        Contact* getContact(Entity* entA, Entity* entB);
        
        //! A method returning an actuator by index.
        /*!
         \param index an id of the actuator
         \return a pointer to an actuator object
         */
        Actuator* getActuator(unsigned int index);
        
        //! A method returning an actuator by name.
        /*!
         \param name a name of the actuator
         \return a pointer to an actuator object
         */
        Actuator* getActuator(const std::string& name);
        
        //! A method returning a sensor by index.
        /*!
         \param index an id of the sensor
         \return a pointer to a sensor object
         */
        Sensor* getSensor(unsigned int index);
        
        //! A method returning a sensor by name.
        /*!
         \param name a name of the sensor
         \return a pointer to a sensor object
         */
        Sensor* getSensor(const std::string& name);
        
        //! A method returning a communication device by index.
        /*!
         \param index an id of the communication device
         \return a pointer to a comm object
         */
        Comm* getComm(unsigned int index);
        
        //! A method returning a communication device by name.
        /*!
         \param name a name of the communication device
         \return a pointer to a comm object
         */
        Comm* getComm(const std::string& name);
        
        //! A method returning a pointer to the NED object.
        NED* getNED();
        
        //! A method returning a pointer to the ocean object.
        Ocean* getOcean();
        
        //! A method returning a pointer to the atmosphere object.
        Atmosphere* getAtmosphere();
        
        //! A method setting the gravity constant used in the simulation.
        void setGravity(Scalar gravityConstant);
        
        //! A method returning the gravity vector.
        Vector3 getGravity() const;
        
        //! A method returning the simulation time in seconds.
        /*! 
         \param applyOffset a flag deciding if the offset between simulation time and real time should be applied
         \return the time of simulation in seconds
         */
        Scalar getSimulationTime(bool applyOffset = false) const;
        
        //! A method informing about the relation between the simulated time and real time.
        Scalar getRealtimeFactor() const;
        
        //! A method returning a pointer to the material manager.
        MaterialManager* getMaterialManager();
        
        //! A method returning a pointer to the name manager.
        NameManager* getNameManager();
        
        //! A method returning a reference to the performance monitor.
        PerformanceMonitor& getPerformanceMonitor();

        //! A method returning a pointer to the trackball view.
        OpenGLTrackball* getTrackball();
        
        //! A method informing if the simulation is freshly started.
        bool isSimulationFresh() const;
        
        //! A method informing if the ocean is enabled in the simulation.
        bool isOceanEnabled() const;
        
        //! A method returning a pointer to the Bullet dynamics world.
        btSoftMultiBodyDynamicsWorld* getDynamicsWorld();

        //! A method returning the simulation sleeping settings.
        void getSleepingThresholds(Scalar& linear, Scalar& angular) const;

        //! A method returning the simulation setup related to joint constraints.
        void getJointErp(Scalar& erp, Scalar& stopErp) const;
        
        //------ Aliases created to shorten the code needed to build the scenario ------
        
        //! A method that creates a new material.
        /*!
         \param uniqueName a name for the material
         \param density a density of the material [kg*m^-3]
         \param restitution a restitution factor <0,1>
         \return a name of the created material
         */
        std::string CreateMaterial(const std::string& uniqueName, Scalar density, Scalar restitution);
        
        //! A method that sets interaction between a pair of materials.
        /*!
         \param firstMaterialName a name of the first material
         \param secondMaterialName a name of the second material
         \param staticFricCoeff a coefficient of static friction between materials
         \param dynamicFricCoeff a coefficient of dynamic friction between materials
         \return was the interaction was set properly?
         */
        bool SetMaterialsInteraction(const std::string& firstMaterialName, const std::string& secondMaterialName, Scalar staticFricCoeff, Scalar dynamicFricCoeff);
        
        //! A method used to create a rendering look.
        /*!
         \param name the name of the look
         \param color a color of the material
         \param roughness how smooth the material looks
         \param metalness how metallic the material looks
         \param reflectivity how reflective the material is
         \param albedoTexturePath a path to a texture specifying albedo color
         \param normalTexturePath a path to a texture specifying surface normal (bump mapping)
         \param temperatureTexturePath a path to a texture specifying temperature distribution
         \param temperatureRange a range of temperatures represented by the texture values
         \return the actual name of the created look
         */
        std::string CreateLook(const std::string& name, Color color, float roughness, float metalness = 0.f, float reflectivity = 0.f, 
                               const std::string& albedoTexturePath = "", const std::string& normalTexturePath = "", 
                               const std::string& temperatureTexturePath = "", const std::pair<float, float>& temperatureRange = std::make_pair(20.f, 20.f));
        
    protected:
        static void SolveICTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static void SimulationTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static void SimulationPostTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);
        static bool ContactInfoUpdateCallback(btManifoldPoint& cp, void* body0, void* body1);
        static bool ContactInfoDestroyCallback(void* userPersistentData);

        btSoftMultiBodyDynamicsWorld* dynamicsWorld;
        btMultiBodyConstraintSolver* mbSolver;
        btSoftBodySolver* sbSolver;
        btSoftBodyWorldInfo sbInfo;
        btCollisionDispatcher* dwDispatcher;
        btBroadphaseInterface* dwBroadphase;
        btDefaultCollisionConfiguration* dwCollisionConfig;
        
        MaterialManager* materialManager;
        
    private:
        void RenderBulletDebug();
        void InitializeSolver();
        void InitializeScenario();
        
        // State
        Scalar simulationTime; // Time of simulation run in seconds
        uint64_t currentTime;  // Current system time in us
        uint64_t timeOffset;   // Offset between simulation time and system time in us
        uint64_t ssus;         // Simulation step time in us
        bool simulationFresh;
        bool callSimulationStepCompleted;

        // Performance
        PerformanceMonitor perfMon;
        Scalar realtimeFactor;
        Scalar cpuUsage;
        unsigned int fdPrescaler;
        unsigned int fdCounter;
        
        // Threading
        SDL_mutex* simSettingsMutex;
        SDL_mutex* simInfoMutex;
        SDL_mutex* simHydroMutex;
        
        // IC solver settings
        bool icUseGravity;
        Scalar icTimeStep;
        unsigned int icMaxIter;
        Scalar icMaxTime;
        Scalar icLinTolerance;
        Scalar icAngTolerance;
        unsigned int mlcpFallbacks;
        bool icProblemSolved;

        // Sover settings
        Solver solver;
        CollisionFilter collisionFilter;
        Scalar sps;
        Scalar linSleepThreshold;
        Scalar angSleepThreshold;
        Scalar jointErp;
        Scalar jointLimitErp;

        // Scenario
        NameManager* nameManager;
        std::vector<Robot*> robots;
        std::vector<Entity*> entities;
        std::vector<Joint*> joints;
        std::vector<Sensor*> sensors;
        std::vector<Actuator*> actuators;
        std::vector<Comm*> comms;
        std::vector<Contact*> contacts;
        std::vector<Collision> collisions;
        NED* ned;
        Ocean* ocean;
        Atmosphere* atmosphere;
        Scalar g;
        DisplayMode sdm;
        
        // Graphics
        OpenGLTrackball* trackball;
        OpenGLDebugDrawer* debugDrawer;
    };
}

#endif
