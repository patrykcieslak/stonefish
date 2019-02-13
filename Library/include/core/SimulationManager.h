//
//  SimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationManager__
#define __Stonefish_SimulationManager__

#include <SDL2/SDL_mutex.h>
#include "StonefishCommon.h"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"

namespace sf
{
    class NameManager;
    class MaterialManager;
    class Console;
    class Robot;
    class ResearchDynamicsWorld;
    class ResearchConstraintSolver;
    class Entity;
    class StaticEntity;
    class SolidEntity;
    class FeatherstoneEntity;
    class Joint;
    class Actuator;
    class Sensor;
    class Contact;
    class OpenGLTrackball;
    class OpenGLDebugDrawer;
    
    //! An enum designating the type of solver used for physics computation
    typedef enum {SOLVER_SI, SOLVER_DANTZIG, SOLVER_PGS, SOLVER_LEMKE, SOLVER_NNCG} SolverType;
    
    //! An enum designating the approach to collision detection
    typedef enum {COLLISION_INCLUSIVE, COLLISION_EXCLUSIVE} CollisionFilteringType;
    
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
        SimulationManager(Scalar stepsPerSecond = Scalar(60), SolverType st = SOLVER_SI, CollisionFilteringType cft = COLLISION_EXCLUSIVE, HydrodynamicsType ht = GEOMETRY_BASED);
        
        //! A destructor.
        virtual ~SimulationManager();
        
        //! A method used to construct simulation scenario. This has to be implemented by the subclass.
        virtual void BuildScenario() = 0;
        
        //! A method loading a simulation world from an XML file.
        /*!
         \param path a path to the XML file describing the simulation scenario
         \return was the file successfully parsed?
         */
        bool LoadSDF(const std::string& path);
        
        //! A method called after the simulation step is completed. Useful to implement interaction with outside code.
        virtual void SimulationStepCompleted();
        
        //! A method solving the initial conditions problem.
        bool SolveICProblem();
        
        //! A method cleaning the simulation world.
        void DestroyScenario();
        
        //! A method which starts the simulation.
        bool StartSimulation();
        
        //! A method which stops the simulation.
        void StopSimulation();
        
        //! A method that resumes the paused simulation.
        void ResumeSimulation();
        
        //! A method which restarts the simulation.
        void RestartScenario();
        
        //! A method computing the next simulation step.
        void AdvanceSimulation();
        
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
        
        //! A method that adds a dynamic rigid body to the simulation world.
        /*!
         \param ent a pointer to the dynamic body object
         \param origin a pose of the body in the world frame
         */
        void AddSolidEntity(SolidEntity* ent, const Transform& origin);
        
        //! A method that adds a rigid multibody to the simulation world.
        /*!
         \param ent a pointer to the multibody object
         \param origin a pose of the multibody base link in the world frame
         */
        void AddFeatherstoneEntity(FeatherstoneEntity* ent, const Transform& origin);
        
        //! A method that adds a discrete joint to the simulation world.
        /*!
         \param jnt a pointer to the joint object
         */
        void AddJoint(Joint* jnt);
        
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
        
        //! A method that adds contact monitoring between two entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         \param historyLength a length of the history buffer
         \return a pointer to the contact sensor
         */
        Contact* AddContact(Entity* entA, Entity* entB, size_t historyLength = 1);
        
        //! A method that enables collision between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         */
        void EnableCollision(Entity* entA, Entity* entB);
        
        //! A method that disables collision between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         */
        void DisableCollision(Entity* entA, Entity* entB);
        
        //! A method that checks if collision is enabled between specified entities.
        /*!
         \param entA a pointer to the first entity
         \param entB a pointer to the second entity
         \return
         */
        int CheckCollision(Entity* entA, Entity* entB);
        
        //! A method used to enable ocean simulation.
        /*!
         \param waves a flag that informs if geometric waves should be simulated
         \param f a pointer to a liquid that will feel the ocean (if left blank defaults to water)
         */
        void EnableOcean(bool waves = false, Fluid* f = NULL);
        
        //! A method used to enable atmosphere simulation.
        void EnableAtmosphere();
        
        //! A method used to pick an entity by shooting a camera ray.
        /*!
         \param eye the position of the camera eye in the world frame
         \param ray a unit vector representing the ray generated in the world frame
         \return a pointer to the hit entity
         */
        Entity* PickEntity(Vector3 eye, Vector3 ray);
        
        //! A method that sets new valve for the amount of simulation steps in a second.
        /*!
         \param steps number steps of simulation per second
         */
        void setStepsPerSecond(Scalar steps);
        
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
        
        //! A method returning the physics computation time in ms.
        Scalar getPhysicsTimeInMiliseconds();
        
        //! A method returning the current number of steps per second used.
        Scalar getStepsPerSecond();
        
        //! A method returning the axis-aligned bounding box of the simulation world.
        /*!
         \param min a position of the minimum corner
         \param max a position of the maximum corner
         */
        void getWorldAABB(Vector3& min, Vector3& max);
        
        //! A method returning the collision filtering used in simulation.
        CollisionFilteringType getCollisionFilter();
        
        //! A method returning the type of solver used.
        SolverType getSolverType();
        
        //! A method returning the type of hydrodynamic computations used.
        HydrodynamicsType getHydrodynamicsType();
        
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
        Robot* getRobot(std::string name);
        
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
        Entity* getEntity(std::string name);
        
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
        Joint* getJoint(std::string name);
        
        //! A method returning a contact by index.
        /*!
         \param index an id of the contact
         \return a pointer to a contact object
         */
        Contact* getContact(unsigned int index);
        
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
        Actuator* getActuator(std::string name);
        
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
        Sensor* getSensor(std::string name);
        
        //! A method returning a pointer to the ocean object.
        Ocean* getOcean();
        
        //! A method returning a pointer to the atmosphere object.
        Atmosphere* getAtmosphere();
        
        //! A method setting the gravity constant used in the simulation.
        void setGravity(Scalar gravityConstant);
        
        //! A method returning the gravity vector.
        Vector3 getGravity();
        
        //! A method returning the simulation time in seconds.
        Scalar getSimulationTime();
        
        //! A method informing about the relation between the simulated time and real time.
        Scalar getRealtimeFactor();
        
        //! A method returning a pointer to the material manager.
        MaterialManager* getMaterialManager();
        
        //! A method returning a pointer to the name manager.
        NameManager* getNameManager();
        
        //! A method returning a pointer to the trackball view.
        OpenGLTrackball* getTrackball();
        
        //! A method informing if the simulation is freshly started.
        bool isSimulationFresh();
        
        //! A method informing if the ocean is enabled in the simulation.
        bool isOceanEnabled();
        
        //! A method returning a pointer to the Bullet dynamics world.
        btMultiBodyDynamicsWorld* getDynamicsWorld();
        
        //! A method used to create a rendering look.
        /*!
         \param color a color of the material
         \param roughness how smooth the material looks
         \param metalness how metallic the material looks
         \param reflectivity how reflective the material is
         \param texturePath a path to a texture
         \return an id of the created look
         */
        static int CreateLook(Color color, float roughness, float metalness = 0.f, float reflectivity = 0.f, std::string texturePath = "");
        
    protected:
        static void SolveICTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static void SimulationTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static void SimulationPostTickCallback(btDynamicsWorld* world, Scalar timeStep);
        static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1);
        
        btMultiBodyDynamicsWorld* dynamicsWorld;
        btMultiBodyConstraintSolver* dwSolver;
        btCollisionDispatcher* dwDispatcher;
        btBroadphaseInterface* dwBroadphase;
        btDefaultCollisionConfiguration* dwCollisionConfig;
        
        MaterialManager* materialManager;
        
    private:
        void RenderBulletDebug();
        void InitializeSolver();
        void InitializeScenario();
        
        SolverType solver;
        CollisionFilteringType collisionFilter;
        HydrodynamicsType hydroType;
        Scalar sps;
        Scalar realtimeFactor;
        unsigned int hydroPrescaler;
        unsigned int hydroCounter;
        SDL_mutex* simSettingsMutex;
        SDL_mutex* simInfoMutex;
        SDL_mutex* simHydroMutex;
        
        Scalar simulationTime;
        uint64_t currentTime;
        uint64_t physicsTime;
        uint64_t ssus;
        bool icUseGravity;
        Scalar icTimeStep;
        unsigned int icMaxIter;
        Scalar icMaxTime;
        Scalar icLinTolerance;
        Scalar icAngTolerance;
        unsigned int mlcpFallbacks;
        bool icProblemSolved;
        bool simulationFresh;
        
        NameManager* nameManager;
        std::vector<Robot*> robots;
        std::vector<Entity*> entities;
        std::vector<Joint*> joints;
        std::vector<Sensor*> sensors;
        std::vector<Actuator*> actuators;
        std::vector<Contact*> contacts;
        std::vector<Collision> collisions;
        Ocean* ocean;
        Atmosphere* atmosphere;
        Scalar g;
        
        //graphics
        OpenGLTrackball* trackball;
        OpenGLDebugDrawer* debugDrawer;
    };
}

#endif
