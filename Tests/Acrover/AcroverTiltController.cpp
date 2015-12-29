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
AcroverTiltController::AcroverTiltController(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* pendulumEnc, FakeRotaryEncoder* wheelEnc, AcroverDriveController* pendulumDrive, btScalar frequency) : FeedbackController(uniqueName, 4, frequency)
{
    imu = cartImu;
    lEnc = pendulumEnc;
    wEnc = wheelEnc;
    drive = pendulumDrive;
    tyreRadius = 0.240;
    maxTilt = 7.0/180.0*M_PI;
 
    //Initialise gain calculation
    L1 = Eigen::VectorXd(4);
    L1 << -0.000158038099130146, 0.00184509192257782, -0.00410056190334906, -0.101271867292741;

    L2 = Eigen::VectorXd(4);
    L2 <<  -0.000225644945955055, 0.000803980628878505, 0.0140514758834186, -0.0499868613968714;

    L3 = Eigen::VectorXd(4);
    L3 <<  0.000908508273857669, -0.00227118386982964, -0.000318471764552071, -0.0144395761382475;
    
    X1 = Eigen::MatrixXd(4,4);
    X1 << 3.60329239399578e-05,	-4.41748924441640e-06,	-0.000209433980135972,	0.000185011348612678,
         -4.41748924441640e-06,	0.000735396937275931,	0.000257399165075062,	-0.00476783663148970,
         -0.000209433980135972,	0.000257399165075062,	0.00158103295368960,	0.00173804942006893,
         0.000185011348612678,	-0.00476783663148970,	0.00173804942006893,	0.0955902749680996;
    
    X2 = Eigen::MatrixXd(4,4);
    X2 << 0.000140958520175255,	-0.000182202347190216,	-0.000917174780085937,	0.00100573995387221,
         -0.000182202347190216,	0.00171851510479154,	0.00166480893805223,	-0.0109686560402013,
         -0.000917174780085937,	0.00166480893805223,	0.00486445286288715,	-0.0184135843920922,
           0.00100573995387221,	-0.0109686560402013,	-0.0184135843920922,	0.0901140433984319;
    
    X3 = Eigen::MatrixXd(4,4);
    X3 << -0.000140511078456246,	0.000186839467924737,	0.000921767167983697,	-0.00117140349572639,
           0.000186839467924737,	-0.00200190867396858,	-0.00148601016869729,	0.0108522895047943,
           0.000921767167983697,	-0.00148601016869729,	-0.00462074319665384,	0.00752970674969309,
           -0.00117140349572639,	0.0108522895047943,	0.00752970674969309,	-0.0559888725121177;
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
    
    btScalar dGamma = wEnc->getLastSample().getValue(1);
    btScalar dPhi = imu->getLastSample().getValue(5);
    btScalar gc = dPhi * dGamma * tyreRadius;
    
    //Find stable alpha with iterative solution (not possible to derive direct solution)
    btScalar l = 0.254;
    btScalar r1 = 0.214;
    btScalar r2 = 0.128;
    btScalar rt = 0.015;
    btScalar m1 = 4.245;
    btScalar m2 = 1.361;
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
    
    for(unsigned int i = 0; i < 4; ++i)
        control += (measurements[i]-ref[i]) * K(i);
    
    btScalar control_e = m2*g*sin(ref[0]+ref[1])*r2;
    control += control_e;
    
    //printf("Theta_e: %1.5f Alpha_e: %1.5f ErrorTh: %1.5f ErrorAl: %1.5f Control_e: %1.5f Control: %1.5f\n", ref[0], ref[1], measurements[0]-ref[0], measurements[1]-ref[1], control_e, control);
    
    //printf("Theta_ref: %1.5f\n", ref[0]);
    
    //Apply control
    drive->setTorque(control);
}