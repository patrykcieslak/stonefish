//
//  AcroverTiltController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcroverTiltController.h"
#include "ScientificFileUtil.h"
#include "SystemUtil.h"
#include <Eigen/Dense>
#include <iostream>

#pragma mark Constructors
AcroverTiltController::AcroverTiltController(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* pendulumEnc, FakeRotaryEncoder* wheelEnc, AcroverDriveController* pendulumDrive, btScalar frequency) : FeedbackController(uniqueName, 5, frequency)
{
    imu = cartImu;
    lEnc = pendulumEnc;
    wEnc = wheelEnc;
    drive = pendulumDrive;
    tyreRadius = 0.240;
    maxTilt = 6.0/180.0*M_PI;
    errorIntegral = 0.;
 
    //Initialise gain calculation
    L1 = Eigen::VectorXd(5);
    L1 << 0.0885136145868993,
         -1.63552704630421,
         -0.323900602370817,
         20.8108677907447,
          0.0127491299974888;

    L2 = Eigen::VectorXd(5);
    L2 <<  -0.0597933721328918,
            0.991324386701120,
           -2.10196686508270,
          -11.4554539251081,
           -0.0361192048919866;

    L3 = Eigen::VectorXd(5);
    L3 <<  -0.161173778170190,
           -0.146097344668172,
            2.41837625867793,
           -2.50366568125538,
           -0.0267150533065005;
    
    X1 = Eigen::MatrixXd(5,5);
    X1 << 0.252510498433865,	-0.0462648910896191,	-1.50780782545596,	0.865536329676082,	0.0416384933283826,
        -0.0462648910896191,	0.748091279767409,	0.767311702871523,	-2.64442867651827,	-0.00812214483025762,
        -1.50780782545596,	0.767311702871523,	9.83056422459848,	-2.40331358059471,	-0.233840540028238,
        0.865536329676082,	-2.64442867651827,	-2.40331358059471,	52.2041658263613,	0.233017264000652,
        0.0416384933283826,	-0.00812214483025762,	-0.233840540028238,	0.233017264000652,	0.0651410879292080;
    
    X2 = Eigen::MatrixXd(5,5);
    X2 << 0.0199465678371572,	0.0620355872387174,	-0.0604306767107088,	-0.171719793984670,	0.00804085762777010,
          0.0620355872387174,   0.428369247015940,	0.0101400351896518,	1.63834699055254,	0.0323941865816221,
         -0.0604306767107088,	0.0101400351896518,	-1.14873386464247,	-5.72733731882222,	-0.0318066814828746,
         -0.171719793984670,	1.63834699055254,	-5.72733731882222,	-27.9788668273641,	-0.115806774017337,
         0.00804085762777010,	0.0323941865816221,	-0.0318066814828746,	-0.115806774017337,	0.00227154476711275;
    
    X3 = Eigen::MatrixXd(5,5);
    X3 << -0.237003470609880,	0.0302615740694970,	1.36541348840279,	-0.815020056535542,	-0.0407594506663631,
          0.0302615740694970,	0.00397812639405796,	-0.628555034604906,	-0.749161204700090,	0.00250507329275335,
          1.36541348840279,	-0.628555034604906,	-7.26824283323871,	7.08495657296374,	0.229834833041495,
         -0.815020056535542,	-0.749161204700090,	7.08495657296374,	12.4997251221770,	-0.116634858989935,
        -0.0407594506663631,	0.00250507329275335,	0.229834833041495,	-0.116634858989935,	-0.00659387112452618;
}

#pragma mark - Destructor
AcroverTiltController::~AcroverTiltController()
{
}

#pragma mark - Accessors
void AcroverTiltController::setDesiredTilt(btScalar tilt)
{
    tilt = tilt > maxTilt ? maxTilt : (tilt < -maxTilt ? -maxTilt: tilt);
    setReferenceValue(0, UnitSystem::SetAngle(tilt));
}

#pragma mark - Methods
void AcroverTiltController::Reset()
{
    errorIntegral = 0.;
}

void AcroverTiltController::Tick(btScalar dt)
{
    //Update desired values
    std::vector<btScalar> ref = getReferenceValues();
    
    //Read sensors
    std::vector<btScalar> measurements;
    measurements.push_back(imu->getLastSample().getValue(0)); //theta
    measurements.push_back(lEnc->getLastSample().getValue(0)); //alpha
    measurements.push_back(imu->getLastSample().getValue(3)); //dtheta
    measurements.push_back(lEnc->getLastSample().getValue(1)); //dalpha
    measurements.push_back(errorIntegral);
    
    //Calculate error integral
    errorIntegral += (ref[0]-measurements[0])*dt;
    errorIntegral = errorIntegral > 1.0 ? 1.0 : (errorIntegral < -1.0 ? -1.0 : errorIntegral);
    
    btScalar dGamma = wEnc->getLastSample().getValue(1);
    btScalar dPhi = imu->getLastSample().getValue(5);
    btScalar gc = dPhi * dGamma * tyreRadius;
    
    //Find stable alpha with iterative solution (not possible to derive direct solution)
    btScalar l = 0.254;
    btScalar r1 = 0.210;
    btScalar r2 = 0.140;
    btScalar rt = 0.015;
    btScalar m1 = 4.74;
    btScalar m2 = 1.72;
    btScalar g = 9.81;
    btScalar equality = 0.0;
    unsigned int iter = 0;
    
    ref[1] = 0;
    
    do
    {
        //It works because the angles change in the same direction and monotonically -> have to show it!
        equality = m2*r2*g*btSin(ref[0]+ref[1])-m2*r2*gc*btCos(ref[0]+ref[1])+(m1*gc*(r1-rt)+m2*gc*(l-rt))*cos(ref[0]) - btSin(ref[0]) * (m1*g*(r1-rt)+m2*g*(l-rt));           //-btSin(ref[0]+ref[1])-(m2 * r2 * centrifugalAcc * btCos(ref[0]+ref[1]) + m2*(-g*l*btSin(ref[0]) - centrifugalAcc*l*btCos(ref[0])) + m1*r1*(centrifugalAcc*btCos(ref[0])-g*btSin(ref[0])))/(m2*r2*g);
        ref[1] -= equality * 0.01;
        iter++;
    }
    while(btFabs(equality) > 0.001);
    
    
    //ref[1] = btAsin((m1*(r1-rt)*sin(ref[0]))/(m2*r2) + ((l-rt)*sin(ref[0]))/r2)-ref[0]; //without centrifugal acceleration
    
    //printf("Theta: %1.5lf Alpha: %1.5lf Equality: %1.5f Iterations: %d\n", ref[0], ref[1], equality, iter);
    //btScalar alpha_e = btAsin((m1*(r1-rt)*sin(measurements[0]))/(m2*r2) + ((l-rt)*sin(measurements[0]))/r2)-measurements[0];
    
    //Calculate gains
    btScalar rho = btScalar(1.0) - btCos(measurements[1]);
    
    Eigen::VectorXd L = Eigen::VectorXd::Zero(4);
    L = L1 + rho*L2 + rho*rho*L3;
    
    Eigen::MatrixXd X = Eigen::MatrixXd::Zero(4, 4);
    X = X1 + rho*X2 + rho*rho*X3;
    
    Eigen::RowVectorXd K = Eigen::RowVectorXd::Zero(4);
    K = L.transpose() * X.inverse();
    
    //Calculate control
    btScalar control = 0;
    
    for(unsigned int i = 0; i < 5; ++i)
        control += (ref[i]-measurements[i]) * K(i);
    
    btScalar control_e = m2*g*sin(ref[0]+ref[1])*r2;
    control += control_e;
    
    //printf("Theta_e: %1.5f Alpha_e: %1.5f ErrorTh: %1.5f ErrorAl: %1.5f Control_e: %1.5f Control: %1.5f\n", ref[0], ref[1], measurements[0]-ref[0], measurements[1]-ref[1], control_e, control);
    
    //printf("Theta_ref: %1.5f\n", ref[0]);
    
    //Apply control
    drive->setTorque(control);
}