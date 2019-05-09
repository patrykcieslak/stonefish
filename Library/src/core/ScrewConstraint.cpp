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
//  ScrewConstraint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/04/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "core/ScrewConstraint.h"

namespace sf
{

Scalar ScrewConstraint::getAngularPosition()
{
    this->calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
    const Vector3 axisA0 = m_calculatedTransformA.getBasis().getColumn(1);
    const Vector3 axisA1 = m_calculatedTransformA.getBasis().getColumn(2);
    const Vector3 axisB0 = m_calculatedTransformB.getBasis().getColumn(1);
    return btAtan2(axisB0.dot(axisA1), axisB0.dot(axisA0));
}

//////////////////////////////////////////////////
Scalar ScrewConstraint::getLinearPosition()
{
    this->calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
    return this->m_depth[0];
}

//////////////////////////////////////////////////
btRigidBody& ScrewConstraint::getRigidBodyA()
{
    return this->m_rbA;
}

//////////////////////////////////////////////////
btRigidBody& ScrewConstraint::getRigidBodyB()
{
    return this->m_rbB;
}

//////////////////////////////////////////////////
void ScrewConstraint::_getInfo2NonVirtual(btConstraintInfo2* info, const Transform& transA, const Transform& transB,
                                          const Vector3& linVelA, const Vector3& linVelB, Scalar rbAinvMass, Scalar rbBinvMass)
{
    /// This is a copy of btSliderConstraint::getInfo2NonVirtual(...)
    /// with minor changes to the ax1 direction constraint.
    /// Mainly, the axial limit constraint is always on and is
    /// changed to a screw constraint.
    
    /// First, always turn on
    const Transform& trA = getCalculatedTransformA();
    const Transform& trB = getCalculatedTransformB();
    
    btAssert(!m_useSolveConstraintObsolete);
    int i, s = info->rowskip;
    
    Scalar signFact = m_useLinearReferenceFrameA ?
    Scalar(1.0f) : Scalar(-1.0f);
    
    // difference between frames in WCS
    Vector3 ofs = trB.getOrigin() - trA.getOrigin();
    // now get weight factors depending on masses
    Scalar miA = rbAinvMass;
    Scalar miB = rbBinvMass;
    bool hasStaticBody = (miA < SIMD_EPSILON) || (miB < SIMD_EPSILON);
    Scalar miS = miA + miB;
    Scalar factA, factB;
    if (miS > Scalar(0.f))
    {
        factA = miB / miS;
    }
    else
    {
        factA = Scalar(0.5f);
    }
    factB = Scalar(1.0f) - factA;
    Vector3 ax1, p, q;
    Vector3 ax1A = trA.getBasis().getColumn(0);
    Vector3 ax1B = trB.getBasis().getColumn(0);
    if (m_useOffsetForConstraintFrame)
    {
        // get the desired direction of slider axis
        // as weighted sum of X-orthos of frameA and frameB in WCS
        ax1 = ax1A * factA + ax1B * factB;
        ax1.normalize();
        // construct two orthos to slider axis
        btPlaneSpace1(ax1, p, q);
    }
    else
    { // old way - use frameA
        ax1 = trA.getBasis().getColumn(0);
        // get 2 orthos to slider axis (Y, Z)
        p = trA.getBasis().getColumn(1);
        q = trA.getBasis().getColumn(2);
    }
    // make rotations around these orthos equal
    // the slider axis should be the only unconstrained
    // rotational axis, the angular velocity of the two bodies perpendicular to
    // the slider axis should be equal. thus the constraint equations are
    //    p*w1 - p*w2 = 0
    //    q*w1 - q*w2 = 0
    // where p and q are unit vectors normal to the slider axis, and w1 and w2
    // are the angular velocity vectors of the two bodies.
    info->m_J1angularAxis[0] = p[0];
    info->m_J1angularAxis[1] = p[1];
    info->m_J1angularAxis[2] = p[2];
    info->m_J1angularAxis[s+0] = q[0];
    info->m_J1angularAxis[s+1] = q[1];
    info->m_J1angularAxis[s+2] = q[2];
    
    info->m_J2angularAxis[0] = -p[0];
    info->m_J2angularAxis[1] = -p[1];
    info->m_J2angularAxis[2] = -p[2];
    info->m_J2angularAxis[s+0] = -q[0];
    info->m_J2angularAxis[s+1] = -q[1];
    info->m_J2angularAxis[s+2] = -q[2];
    // compute the right hand side of the constraint equation. set relative
    // body velocities along p and q to bring the slider back into alignment.
    // if ax1A,ax1B are the unit length slider axes as computed from bodyA and
    // bodyB, we need to rotate both bodies along the axis u = (ax1 x ax2).
    // if "theta" is the angle between ax1 and ax2, we need an angular velocity
    // along u to cover angle erp*theta in one step :
    //   |angular_velocity| = angle/time = erp*theta / stepsize
    //                      = (erp*fps) * theta
    //    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
    //                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
    // ...as ax1 and ax2 are unit length. if theta is smallish,
    // theta ~= sin(theta), so
    //    angular_velocity  = (erp*fps) * (ax1 x ax2)
    // ax1 x ax2 is in the plane space of ax1, so we project the angular
    // velocity to p and q to find the right hand side.
    //  Scalar k = info->fps * info->erp * getSoftnessOrthoAng();
    Scalar currERP = (m_flags & BT_SLIDER_FLAGS_ERP_ORTANG) ?
    m_softnessOrthoAng : m_softnessOrthoAng * info->erp;
    Scalar k = info->fps * currERP;
    
    Vector3 u = ax1A.cross(ax1B);
    info->m_constraintError[0] = k * u.dot(p);
    info->m_constraintError[s] = k * u.dot(q);
    if (m_flags & BT_SLIDER_FLAGS_CFM_ORTANG)
    {
        info->cfm[0] = m_cfmOrthoAng;
        info->cfm[s] = m_cfmOrthoAng;
    }
    
    // last filled row
    int nrow = 1;
    int srow;
    Scalar limit_err;
    int limit;
    int powered;
    
    // next two rows.
    // we want: velA + wA x relA == velB + wB x relB ... but this would
    // result in three equations, so we project along two orthos to the
    // slider axis
    
    Transform bodyA_trans = transA;
    Transform bodyB_trans = transB;
    nrow++;
    int s2 = nrow * s;
    nrow++;
    int s3 = nrow * s;
    Vector3 tmpA(0, 0, 0), tmpB(0, 0, 0), relA(0, 0, 0),
    relB(0, 0, 0), c(0, 0, 0);
    if (m_useOffsetForConstraintFrame)
    {
        // get vector from bodyB to frameB in WCS
        relB = trB.getOrigin() - bodyB_trans.getOrigin();
        // get its projection to slider axis
        Vector3 projB = ax1 * relB.dot(ax1);
        // get vector directed from bodyB to slider axis (and orthogonal to it)
        Vector3 orthoB = relB - projB;
        // same for bodyA
        relA = trA.getOrigin() - bodyA_trans.getOrigin();
        Vector3 projA = ax1 * relA.dot(ax1);
        Vector3 orthoA = relA - projA;
        // get desired offset between frames A and B along slider axis
        Scalar sliderOffs = m_linPos - m_depth[0];
        // desired vector from projection of center of bodyA to projection of
        // center of bodyB to slider axis
        Vector3 totalDist = projA + ax1 * sliderOffs - projB;
        // get offset vectors relA and relB
        relA = orthoA + totalDist * factA;
        relB = orthoB - totalDist * factB;
        // now choose average ortho to slider axis
        p = orthoB * factA + orthoA * factB;
        Scalar len2 = p.length2();
        if (len2 > SIMD_EPSILON)
        {
            p /= btSqrt(len2);
        }
        else
        {
            p = trA.getBasis().getColumn(1);
        }
        // make one more ortho
        q = ax1.cross(p);
        // fill two rows
        tmpA = relA.cross(p);
        tmpB = relB.cross(p);
        for (i = 0; i < 3; ++i)
        {
            info->m_J1angularAxis[s2+i] = tmpA[i];
            info->m_J2angularAxis[s2+i] = -tmpB[i];
        }
        tmpA = relA.cross(q);
        tmpB = relB.cross(q);
        if (hasStaticBody && getSolveAngLimit())
        { // to make constraint between static and dynamic objects more rigid
            // remove wA (or wB) from equation if angular limit is hit
            tmpB *= factB;
            tmpA *= factA;
        }
        for (i = 0; i < 3; ++i)
        {
            info->m_J1angularAxis[s3+i] = tmpA[i];
            info->m_J2angularAxis[s3+i] = -tmpB[i];
            info->m_J1linearAxis[s2+i] = p[i];
            info->m_J1linearAxis[s3+i] = q[i];
            info->m_J2linearAxis[s2+i] = -p[i];
            info->m_J2linearAxis[s3+i] = -q[i];
        }
    }
    else
    {
        // old way - maybe incorrect if bodies are not on the slider axis
        // see discussion "Bug in slider constraint"
        // http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=4024&start=0
        c = bodyB_trans.getOrigin() - bodyA_trans.getOrigin();
        Vector3 tmp = c.cross(p);
        for (i = 0; i < 3; ++i)
        {
            info->m_J1angularAxis[s2+i] = factA*tmp[i];
            info->m_J2angularAxis[s2+i] = factB*tmp[i];
        }
        tmp = c.cross(q);
        for (i = 0; i < 3; ++i)
        {
            info->m_J1angularAxis[s3+i] = factA*tmp[i];
            info->m_J2angularAxis[s3+i] = factB*tmp[i];
            info->m_J1linearAxis[s2+i] = p[i];
            info->m_J1linearAxis[s3+i] = q[i];
            info->m_J2linearAxis[s2+i] = -p[i];
            info->m_J2linearAxis[s3+i] = -q[i];
        }
    }
    // compute two elements of right hand side
    
    //  k = info->fps * info->erp * getSoftnessOrthoLin();
    currERP = (m_flags & BT_SLIDER_FLAGS_ERP_ORTLIN) ?
    m_softnessOrthoLin : m_softnessOrthoLin * info->erp;
    k = info->fps * currERP;
    
    Scalar rhs = k * p.dot(ofs);
    info->m_constraintError[s2] = rhs;
    rhs = k * q.dot(ofs);
    info->m_constraintError[s3] = rhs;
    if (m_flags & BT_SLIDER_FLAGS_CFM_ORTLIN)
    {
        info->cfm[s2] = m_cfmOrthoLin;
        info->cfm[s3] = m_cfmOrthoLin;
    }
    
    // Screw Constraint (coupled linear and angular motion)
    {
        nrow++;
        srow = nrow * info->rowskip;
        info->m_J1linearAxis[srow+0] = -ax1[0] * this->threadPitch;
        info->m_J1linearAxis[srow+1] = -ax1[1] * this->threadPitch;
        info->m_J1linearAxis[srow+2] = -ax1[2] * this->threadPitch;
        
        info->m_J1angularAxis[srow+0] = ax1[0];
        info->m_J1angularAxis[srow+1] = ax1[1];
        info->m_J1angularAxis[srow+2] = ax1[2];
        
        info->m_J2linearAxis[srow+0] = ax1[0] * this->threadPitch;
        info->m_J2linearAxis[srow+1] = ax1[1] * this->threadPitch;
        info->m_J2linearAxis[srow+2] = ax1[2] * this->threadPitch;
        
        info->m_J2angularAxis[srow+0] = -ax1[0];
        info->m_J2angularAxis[srow+1] = -ax1[1];
        info->m_J2angularAxis[srow+2] = -ax1[2];
        
        // correction
        // rhs = k * ax1.dot(ofs);  // from hinge constraint
        Scalar lin_disp = ax1.dot(ofs);
        Scalar ang_pos = this->getAngularPosition();
        info->m_constraintError[srow] =
        -k * (lin_disp * this->threadPitch - ang_pos);
        info->cfm[srow] = -m_cfmOrthoLin;
        
        // debug, set cfm to 0
        // info->cfm[srow] = 0;
        // debug, set error correction to 0
        // info->m_constraintError[srow] = 0.0;
    }
    
    // notes: below enforces
    // joint limit
    // powered joint
    // rotation along slider axis
    
    // check linear limits
    limit_err = Scalar(0.0);
    limit = 0;
    if (getSolveLinLimit())
    {
        limit_err = getLinDepth() *  signFact;
        limit = (limit_err > Scalar(0.0)) ? 2 : 1;
    }
    powered = 0;
    if (getPoweredLinMotor())
    {
        powered = 1;
    }
    // if the slider has joint limits or motor, add in the extra row
    if (limit || powered)
    {
        nrow++;
        srow = nrow * info->rowskip;
        info->m_J1linearAxis[srow+0] = ax1[0];
        info->m_J1linearAxis[srow+1] = ax1[1];
        info->m_J1linearAxis[srow+2] = ax1[2];
        info->m_J2linearAxis[srow+0] = -ax1[0];
        info->m_J2linearAxis[srow+1] = -ax1[1];
        info->m_J2linearAxis[srow+2] = -ax1[2];
        // linear torque decoupling step:
        //
        // we have to be careful that the linear constraint forces (+/- ax1)
        // applied to the two bodies
        // do not create a torque couple. in other words, the points that the
        // constraint force is applied at must lie along the same ax1 axis.
        // a torque couple will result in limited slider-jointed free
        // bodies from gaining angular momentum.
        if (m_useOffsetForConstraintFrame)
        {
            // this is needed only when bodyA and bodyB are both dynamic.
            if (!hasStaticBody)
            {
                tmpA = relA.cross(ax1);
                tmpB = relB.cross(ax1);
                info->m_J1angularAxis[srow+0] = tmpA[0];
                info->m_J1angularAxis[srow+1] = tmpA[1];
                info->m_J1angularAxis[srow+2] = tmpA[2];
                info->m_J2angularAxis[srow+0] = -tmpB[0];
                info->m_J2angularAxis[srow+1] = -tmpB[1];
                info->m_J2angularAxis[srow+2] = -tmpB[2];
            }
        }
        else
        { // The old way. May be incorrect if bodies are not on the slider axis
            // Linear Torque Decoupling vector (a torque)
            Vector3 ltd;
            ltd = c.cross(ax1);
            info->m_J1angularAxis[srow+0] = factA*ltd[0];
            info->m_J1angularAxis[srow+1] = factA*ltd[1];
            info->m_J1angularAxis[srow+2] = factA*ltd[2];
            info->m_J2angularAxis[srow+0] = factB*ltd[0];
            info->m_J2angularAxis[srow+1] = factB*ltd[1];
            info->m_J2angularAxis[srow+2] = factB*ltd[2];
        }
        // right-hand part
        Scalar lostop = getLowerLinLimit();
        Scalar histop = getUpperLinLimit();
        
        // issue #1104:
        // if (limit && (lostop == histop)) raises warnings, using
        // a warning-less implementation.
        if (limit && (std::fabs(lostop - histop) <= static_cast<Scalar>(std::numeric_limits<double>::epsilon())))
        {
            // the joint motor is ineffective
            powered = 0;
        }
        info->m_constraintError[srow] = 0.;
        info->m_lowerLimit[srow] = 0.;
        info->m_upperLimit[srow] = 0.;
        currERP = (m_flags & BT_SLIDER_FLAGS_ERP_LIMLIN) ?
        m_softnessLimLin : info->erp;
        if (powered)
        {
            if (m_flags & BT_SLIDER_FLAGS_CFM_DIRLIN)
            {
                info->cfm[srow] = m_cfmDirLin;
            }
            Scalar tag_vel = getTargetLinMotorVelocity();
            Scalar mot_fact = getMotorFactor(m_linPos, m_lowerLinLimit,
                                             m_upperLinLimit, tag_vel, info->fps * currERP);
            info->m_constraintError[srow] -=
            signFact * mot_fact * getTargetLinMotorVelocity();
            info->m_lowerLimit[srow] += -getMaxLinMotorForce() * info->fps;
            info->m_upperLimit[srow] += getMaxLinMotorForce() * info->fps;
        }
        if (limit)
        {
            k = info->fps * currERP;
            info->m_constraintError[srow] += k * limit_err;
            if (m_flags & BT_SLIDER_FLAGS_CFM_LIMLIN)
            {
                info->cfm[srow] = m_cfmLimLin;
            }
            // issue #1104:
            // if (lostop == histop) raises warnings, using
            // a warning-less implementation.
            if (std::fabs(lostop - histop) <= static_cast<Scalar>(std::numeric_limits<double>::epsilon()))
            {
                // limited low and high simultaneously
                info->m_lowerLimit[srow] = -SIMD_INFINITY;
                info->m_upperLimit[srow] = SIMD_INFINITY;
            }
            else if (limit == 1)
            {
                // low limit
                info->m_lowerLimit[srow] = -SIMD_INFINITY;
                info->m_upperLimit[srow] = 0;
            }
            else
            { // high limit
                info->m_lowerLimit[srow] = 0;
                info->m_upperLimit[srow] = SIMD_INFINITY;
            }
            // bounce (we'll use slider parameter abs(1.0 - m_dampingLimLin)
            //   for that)
            Scalar bounce = btFabs(Scalar(1.0) - getDampingLimLin());
            if (bounce > Scalar(0.0))
            {
                Scalar vel = linVelA.dot(ax1);
                vel -= linVelB.dot(ax1);
                vel *= signFact;
                // only apply bounce if the velocity is incoming, and if the
                // resulting c[] exceeds what we already have.
                if (limit == 1)
                {  // low limit
                    if (vel < 0)
                    {
                        Scalar newc = -bounce * vel;
                        if (newc > info->m_constraintError[srow])
                        {
                            info->m_constraintError[srow] = newc;
                        }
                    }
                }
                else
                { // high limit - all those computations are reversed
                    if (vel > 0)
                    {
                        Scalar newc = -bounce * vel;
                        if (newc < info->m_constraintError[srow])
                        {
                            info->m_constraintError[srow] = newc;
                        }
                    }
                }
            }
            info->m_constraintError[srow] *= getSoftnessLimLin();
        }
        // line above is the end of if (limit)
    }
    // line above is the end of if linear limit
    
    // printf("tp: %f\n", this->threadPitch);
    
    // check angular limits
    limit_err = Scalar(0.0);
    limit = 0;
    if (getSolveAngLimit())
    {
        limit_err = getAngDepth();
        limit = (limit_err > Scalar(0.0)) ? 1 : 2;
    }
    // if the slider has joint limits, add in the extra row
    powered = 0;
    if (getPoweredAngMotor())
    {
        powered = 1;
    }
    if (limit || powered)
    {
        nrow++;
        srow = nrow * info->rowskip;
        info->m_J1angularAxis[srow+0] = ax1[0];
        info->m_J1angularAxis[srow+1] = ax1[1];
        info->m_J1angularAxis[srow+2] = ax1[2];
        
        info->m_J2angularAxis[srow+0] = -ax1[0];
        info->m_J2angularAxis[srow+1] = -ax1[1];
        info->m_J2angularAxis[srow+2] = -ax1[2];
        
        Scalar lostop = getLowerAngLimit();
        Scalar histop = getUpperAngLimit();
        // issue #1104:
        // if (limit && (lostop == histop)) raises warnings, using
        // a warning-less implementation.
        if (limit && (std::fabs(lostop - histop) <= static_cast<Scalar>(std::numeric_limits<double>::epsilon())))
        {  // the joint motor is ineffective
            powered = 0;
        }
        currERP = (m_flags & BT_SLIDER_FLAGS_ERP_LIMANG) ?
        m_softnessLimAng : info->erp;
        if (powered)
        {
            if (m_flags & BT_SLIDER_FLAGS_CFM_DIRANG)
            {
                info->cfm[srow] = m_cfmDirAng;
            }
            Scalar mot_fact = getMotorFactor(m_angPos, m_lowerAngLimit,
                                             m_upperAngLimit, getTargetAngMotorVelocity(), info->fps * currERP);
            info->m_constraintError[srow] = mot_fact * getTargetAngMotorVelocity();
            info->m_lowerLimit[srow] = -getMaxAngMotorForce() * info->fps;
            info->m_upperLimit[srow] = getMaxAngMotorForce() * info->fps;
        }
        if (limit)
        {
            k = info->fps * currERP;
            info->m_constraintError[srow] += k * limit_err;
            if (m_flags & BT_SLIDER_FLAGS_CFM_LIMANG)
            {
                info->cfm[srow] = m_cfmLimAng;
            }
            // issue #1104:
            // if (lostop == histop) raises warnings, using
            // a warning-less implementation.
            if (std::fabs(lostop - histop) <= static_cast<Scalar>(std::numeric_limits<double>::epsilon()))
            {
                // limited low and high simultaneously
                info->m_lowerLimit[srow] = -SIMD_INFINITY;
                info->m_upperLimit[srow] = SIMD_INFINITY;
            }
            else if (limit == 1)
            { // low limit
                info->m_lowerLimit[srow] = 0;
                info->m_upperLimit[srow] = SIMD_INFINITY;
            }
            else
            { // high limit
                info->m_lowerLimit[srow] = -SIMD_INFINITY;
                info->m_upperLimit[srow] = 0;
            }
            // bounce (we'll use slider parameter abs(1.0 - m_dampingLimAng)
            // for that)
            Scalar bounce = btFabs(Scalar(1.0) - getDampingLimAng());
            if (bounce > Scalar(0.0))
            {
                Scalar vel = m_rbA.getAngularVelocity().dot(ax1);
                vel -= m_rbB.getAngularVelocity().dot(ax1);
                // only apply bounce if the velocity is incoming, and if the
                // resulting c[] exceeds what we already have.
                if (limit == 1)
                {  // low limit
                    if (vel < 0)
                    {
                        Scalar newc = -bounce * vel;
                        if (newc > info->m_constraintError[srow])
                        {
                            info->m_constraintError[srow] = newc;
                        }
                    }
                }
                else
                {  // high limit - all those computations are reversed
                    if (vel > 0)
                    {
                        Scalar newc = -bounce * vel;
                        if (newc < info->m_constraintError[srow])
                        {
                            info->m_constraintError[srow] = newc;
                        }
                    }
                }
            }
            info->m_constraintError[srow] *= getSoftnessLimAng();
        }
        // line above is the end of if (limit)
    }
    // line above is the end of if angular limit or powered
}

void ScrewConstraint::_getInfo1NonVirtual(btConstraintInfo1* info)
{
    /// this is a modified version of
    /// void btSliderConstraint::getInfo1(btConstraintInfo1* info)
    /// with the rotational limits always turned on.
    /// The rotational limit constraint is modified to be a screw constraint
    /// in btScrewConstraint::_getInfo2NonVirtual
    /// which is a copy of btSliderConstraint::getInfo2NonVirtual.
    
    // info->m_numConstraintRows = 6;
    // Fixed 2 linear + 2 angular + 1 limit (even if not used)
    // info->nub = 0;
    
    if (m_useSolveConstraintObsolete)
    {
        info->m_numConstraintRows = 0;
        info->nub = 0;
    }
    else
    {
        // Fixed 2 linear + 2 angular
        info->m_numConstraintRows = 4;
        info->nub = 2;
        
        // Add constraint for screw motion, coupling linear and angular motion.
        info->m_numConstraintRows++;
        info->nub++;
        
        // prepare constraint
        calculateTransforms(
                            m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
        testLinLimits();
        if (getSolveLinLimit() || getPoweredLinMotor())
        {
            // limit 3rd linear as well
            info->m_numConstraintRows++;
            info->nub--;
        }
        testAngLimits();
        if (getSolveAngLimit() || getPoweredAngMotor())
        {
            // limit 3rd angular as well
            info->m_numConstraintRows++;
            info->nub--;
        }
    }
    // printf("m: %d\n", info->m_numConstraintRows);
}

}
