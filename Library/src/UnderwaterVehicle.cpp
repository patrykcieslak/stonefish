//
//  UnderwaterVehicle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterVehicle.h"
#include "SimulationApp.h"

UnderwaterVehicle::UnderwaterVehicle(std::string uniqueName, SolidEntity* bodySolid) : SystemEntity(uniqueName)
{
    vehicleBody = new FeatherstoneEntity(uniqueName + "/FE", 1, bodySolid, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), false);
    showInternals = false;
    lastLinearVel = btVector3(0,0,0);
    lastAngularVel = btVector3(0,0,0);
    linearAcc = btVector3(0,0,0);
    angularAcc = btVector3(0,0,0);
}

UnderwaterVehicle::~UnderwaterVehicle()
{
    delete vehicleBody;
    
    for(unsigned int i=0; i<thrusters.size(); ++i)
        delete thrusters[i];
    
    for(unsigned int i=0; i<sensors.size(); ++i)
        delete sensors[i];
        
    thrusters.clear();
    sensors.clear();
}

SystemType UnderwaterVehicle::getSystemType()
{
    return SYSTEM_UNDERWATER_VEHICLE;
}

btTransform UnderwaterVehicle::getTransform() const
{
    return vehicleBody->getLinkTransform(0);
}

FeatherstoneEntity* UnderwaterVehicle::getVehicleBody()
{
	return vehicleBody;
}

void UnderwaterVehicle::AddThruster(Thruster* thruster, const btTransform& location)
{
    thruster->AttachToSolid(vehicleBody, 0, location);
    thruster->setSetpoint(0.0);
    thrusters.push_back(thruster);
}

Pressure* UnderwaterVehicle::AddPressureSensor(const btTransform& location, btScalar updateFrequency)
{
    Pressure* press = new Pressure(getName() + "/Pressure", vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(press);
    return press;
}

DVL* UnderwaterVehicle::AddDVL(const btTransform& location, btScalar updateFrequency)
{
    DVL* dvl = new DVL(getName() + "/DVL", vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(dvl);
    return dvl;
}

FOG* UnderwaterVehicle::AddFOG(const btTransform& location, btScalar updateFrequency)
{
    FOG* fog = new FOG(getName() + "/FOG", vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(fog);
    return fog;
}

IMU* UnderwaterVehicle::AddIMU(const btTransform& location, btScalar updateFrequency)
{
    IMU* imu = new IMU(getName() + "/IMU", vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(imu);
    return imu;
}

GPS* UnderwaterVehicle::AddGPS(const btTransform& location, btScalar homeLatitude, btScalar homeLongitude, btScalar updateFrequency)
{
    GPS* gps = new GPS(getName() + "/GPS", homeLatitude, homeLongitude, vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(gps);
    return gps;
}

Odometry* UnderwaterVehicle::AddOdometry(const btTransform& location, btScalar updateFrequency)
{
    Odometry* odom = new Odometry(getName() + "/Odometry", vehicleBody->getLink(0).solid, location, updateFrequency);
    sensors.push_back(odom);
    return odom;
}

void UnderwaterVehicle::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    vehicleBody->AddToDynamicsWorld(world, worldTransform);
}

void UnderwaterVehicle::UpdateAcceleration(btScalar dt)
{
   	btVector3 linearVel = vehicleBody->getLinkLinearVelocity(0);
	linearAcc = (linearVel - lastLinearVel)/dt;
	lastLinearVel = linearVel;
		
	btVector3 angularVel = vehicleBody->getLinkAngularVelocity(0);
	angularAcc = (angularVel - lastAngularVel)/dt;
	lastAngularVel = angularVel;
		
	//std::cout << "Acc: " << linearAcc.x() << ", " << linearAcc.y() << ", " << linearAcc.z() << std::endl;
}

void UnderwaterVehicle::UpdateSensors(btScalar dt)
{
    for(unsigned int i=0; i<sensors.size(); ++i)
        sensors[i]->Update(dt);
}

void UnderwaterVehicle::UpdateControllers(btScalar dt)
{
}

void UnderwaterVehicle::UpdateActuators(btScalar dt)
{
    for(unsigned int i=0; i<thrusters.size(); ++i)
        thrusters[i]->Update(dt);
}

void UnderwaterVehicle::ApplyGravity(const btVector3& g)
{
    vehicleBody->ApplyGravity(g);
}

void UnderwaterVehicle::ApplyDamping()
{
}

std::vector<Renderable> UnderwaterVehicle::Render()
{
    std::vector<Renderable> items = vehicleBody->Render();
    
    for(unsigned int i=0; i<thrusters.size(); ++i)
    {
        std::vector<Renderable> th = thrusters[i]->Render();
        items.insert(items.end(), th.begin(), th.end());
    }
    
    for(unsigned int i=0; i<sensors.size(); ++i)
    {
        std::vector<Renderable> sens = sensors[i]->Render();
        items.insert(items.end(), sens.begin(), sens.end());
    }
    
    return items;
}

void UnderwaterVehicle::GetAABB(btVector3& min, btVector3& max)
{
    vehicleBody->GetAABB(min, max);
}

void UnderwaterVehicle::SetThrusterSetpoint(unsigned int index, btScalar s)
{
    if(index < thrusters.size())
        thrusters[index]->setSetpoint(s);
}

btScalar UnderwaterVehicle::GetThrusterSetpoint(unsigned int index)
{
    if(index < thrusters.size())
        return thrusters[index]->getSetpoint();
    else
        return btScalar(0);
}

btScalar UnderwaterVehicle::GetThrusterVelocity(unsigned int index)
{
    if(index < thrusters.size())
        return thrusters[index]->getOmega();
    else
        return btScalar(0);
}