//
//  Atmosphere.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Atmosphere.h"

namespace sf
{
    
Atmosphere::Atmosphere(std::string uniqueName, Fluid* f) : ForcefieldEntity(uniqueName)
{
    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    
    Scalar size(10000);
    Vector3 halfExtents = Vector3(size/Scalar(2), size/Scalar(2), size/Scalar(2));
    ghost->setWorldTransform(Transform(Quaternion::getIdentity(), Vector3(0,0,-size/Scalar(2)))); //Above ocean surface
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    gas = f;
    glAtmosphere = NULL;
}
    
Atmosphere::~Atmosphere()
{
    if(glAtmosphere != NULL) delete glAtmosphere;
}
    
OpenGLAtmosphere* Atmosphere::getOpenGLAtmosphere()
{
    return glAtmosphere;
}
    
ForcefieldType Atmosphere::getForcefieldType()
{
    return ForcefieldType::FORCEFIELD_ATMOSPHERE;
}

void Atmosphere::InitGraphics(const RenderSettings& s)
{
    glAtmosphere = new OpenGLAtmosphere(s.atmosphere, s.shadows);
}

void Atmosphere::SetupSunPosition(Scalar longitudeDeg, Scalar latitudeDeg, std::tm& utc)
{
    if(glAtmosphere == NULL) return;
    
    Scalar latitude = latitudeDeg/Scalar(180) * M_PI;
    Scalar longitude = longitudeDeg/Scalar(180) * M_PI;
    Scalar meridian(0); //GMT
    Scalar decimalHours = (Scalar)utc.tm_hour + (Scalar)utc.tm_min/Scalar(60) + (Scalar)utc.tm_sec/Scalar(60);
    std::tm utc0 = utc;
    utc0.tm_mday = 1; //1
    utc0.tm_mon = 0; //January
    int J = JulianDay(utc) - JulianDay(utc0);
    Scalar solarTime = decimalHours + Scalar(0.17)*btSin(Scalar(4)*M_PI*(J - 80)/Scalar(373)) - Scalar(0.129)*btSin(Scalar(2)*M_PI*(J - 8)/Scalar(355)) + Scalar(12)*(meridian - longitude)/M_PI;
    Scalar declination = Scalar(0.4093)*btSin(Scalar(2)*M_PI*(J - 81)/Scalar(368));
    Scalar elevation = btAsin(btSin(latitude)*btSin(declination) - btCos(latitude)*btCos(declination)*btCos(M_PI*solarTime/Scalar(12)));
    Scalar azimuth = btAtan2(btSin(M_PI*solarTime/Scalar(12)), btCos(M_PI*solarTime/Scalar(12))*btSin(latitude) - btTan(declination)*btCos(latitude));
    
    glAtmosphere->SetSunPosition((float)(azimuth/M_PI*Scalar(180)), (float)(elevation/M_PI*Scalar(180)));
}
    
void Atmosphere::SetupSunPosition(Scalar azimuthDeg, Scalar elevationDeg)
{
    if(glAtmosphere == NULL) return;
    
    glAtmosphere->SetSunPosition((float)azimuthDeg, (float)elevationDeg);
}
    
void Atmosphere::GetSunPosition(Scalar &azimuthDeg, Scalar &elevationDeg)
{
    if(glAtmosphere == NULL)
        azimuthDeg = elevationDeg = btScalar(0);
    else
    {
        float az, elev;
        glAtmosphere->GetSunPosition(az, elev);
        azimuthDeg = Scalar(az);
        elevationDeg = Scalar(elev);
    }
}

int Atmosphere::JulianDay(std::tm& tm)
{
    int m = tm.tm_mon + 1;
    int y = tm.tm_year + 1900;
    int d = tm.tm_mday;
    
    Scalar X = (m + 9) / 12.0;
    int A = 4716 + y + (int)trunc(X);
    Scalar Y = 275 * m / 9.0;
    Scalar V = 7 * A / 4.0;
    Scalar B = 1729279.5 + 367 * y + trunc(Y) - trunc(V) + d;
    Scalar Q = (A + 83) / 100.0;
    Scalar W = 3 * (trunc(Q) + 1) / 4.0;
    return B + 38 - (int)trunc(W);
}

}
