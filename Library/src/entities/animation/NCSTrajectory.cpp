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
//  NCSTrajectory.cpp
//  Stonefish
//
//  Created by Roger Pi on 30/09/2026
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "entities/animation/NCSTrajectory.h"
#include <algorithm>

namespace sf
{

namespace
{
    //Coefficients of the SO(3) right Jacobian Jr(phi) = I - A*[phi]x + B*[phi]x^2 and of its derivative, (dA/dtheta)/theta and (dB/dtheta)/theta
    void JacobianCoeffs(Scalar theta, Scalar& A, Scalar& B, Scalar& dA, Scalar& dB)
    {
        Scalar t2 = theta * theta;
        if(theta < Scalar(0.01)) //Series expansion to avoid cancellation
        {
            A = Scalar(1)/Scalar(2) - t2/Scalar(24) + t2*t2/Scalar(720);
            B = Scalar(1)/Scalar(6) - t2/Scalar(120) + t2*t2/Scalar(5040);
            dA = -Scalar(1)/Scalar(12) + t2/Scalar(180);
            dB = -Scalar(1)/Scalar(60) + t2/Scalar(1260);
        }
        else
        {
            Scalar s = btSin(theta);
            Scalar c = btCos(theta);
            A = (Scalar(1) - c)/t2;
            B = (theta - s)/(t2 * theta);
            dA = (theta * s - Scalar(2) * (Scalar(1) - c))/(t2 * t2);
            dB = (theta * (Scalar(1) - c) - Scalar(3) * (theta - s))/(t2 * t2 * theta);
        }
    }

    //Jr(phi) * x
    Vector3 RightJacobianMul(const Vector3& phi, const Vector3& x)
    {
        Scalar A, B, dA, dB;
        JacobianCoeffs(phi.length(), A, B, dA, dB);
        Vector3 px = phi.cross(x);
        return x - A * px + B * phi.cross(px);
    }

    //(d/du Jr(phi(u))) * dphi, where dphi = d/du phi(u)
    Vector3 RightJacobianRate(const Vector3& phi, const Vector3& dphi)
    {
        Scalar A, B, dA, dB;
        JacobianCoeffs(phi.length(), A, B, dA, dB);
        Scalar pd = phi.dot(dphi);
        Vector3 pxd = phi.cross(dphi);
        return -dA * pd * pxd + dB * pd * phi.cross(pxd) + B * dphi.cross(pxd);
    }

    Matrix3 RightJacobian(const Vector3& phi)
    {
        Scalar A, B, dA, dB;
        JacobianCoeffs(phi.length(), A, B, dA, dB);
        Matrix3 S(0, -phi.z(), phi.y(),
                  phi.z(), 0, -phi.x(),
                  -phi.y(), phi.x(), 0);
        Matrix3 S2 = S * S;
        Matrix3 J;
        for(int k=0; k<3; ++k)
            for(int l=0; l<3; ++l)
                J[k][l] = (k == l ? Scalar(1) : Scalar(0)) - A * S[k][l] + B * S2[k][l];
        return J;
    }

    //Rotation vector of a quaternion (shortest path)
    Vector3 LogRotation(Quaternion q)
    {
        if(q.w() < Scalar(0))
            q = -q;
        Vector3 v(q.x(), q.y(), q.z());
        Scalar s = v.length();
        if(s < SIMD_EPSILON)
            return Scalar(2) * v;
        return v * (Scalar(2) * btAtan2(s, q.w())/s);
    }

    Quaternion ExpRotation(const Vector3& phi)
    {
        Scalar theta = phi.length();
        if(theta < SIMD_EPSILON)
            return Quaternion(phi.x()/Scalar(2), phi.y()/Scalar(2), phi.z()/Scalar(2), Scalar(1)).normalized();
        return Quaternion(phi/theta, theta);
    }
}

NCSTrajectory::NCSTrajectory(PlaybackMode playback) : PWLTrajectory(playback)
{
}

void NCSTrajectory::AddKeyPoint(Scalar keyTime, Transform keyTransform)
{
    //Check if time correct
    if(keyTime < Scalar(0)) return;

    //Create key point
    KeyPoint k;
    k.t = keyTime;
    k.T = keyTransform;

    //Add to the list
    auto it = std::find(points.begin(), points.end(), k);
    if(it != points.end())
        *it = k;
    else
        points.push_back(k);

    //Sort key points by time
    std::sort(points.begin(), points.end());

    //Reset
    playTime = Scalar(0);
    endTime = points.back().t;
    forward = true;

    ComputeCoefficients();
    BuildGraphicalPath();
    Interpolate();
}

void NCSTrajectory::ComputeCoefficients()
{
    a.clear();
    b.clear();
    c.clear();
    d.clear();
    rot.clear();

    size_t n = points.size();
    if(n < 3)
        return;

    //Segment durations (key point times are unique and sorted, so h > 0)
    std::vector<Scalar> h(n-1);
    for(size_t i=0; i<n-1; ++i)
        h[i] = points[i+1].t - points[i].t;

    //Solve the tridiagonal system for the second derivatives M (Thomas algorithm).
    //Natural boundary conditions: M_0 = M_{n-1} = 0. The matrix depends only on h, so all axes are solved at once.
    std::vector<Vector3> M(n, V0());
    std::vector<Scalar> cp(n, Scalar(0));
    std::vector<Vector3> dp(n, V0());
    for(size_t i=1; i<n-1; ++i)
    {
        Vector3 P0 = points[i-1].T.getOrigin();
        Vector3 P1 = points[i].T.getOrigin();
        Vector3 P2 = points[i+1].T.getOrigin();
        Vector3 rhs = Scalar(6) * ((P2 - P1)/h[i] - (P1 - P0)/h[i-1]);
        Scalar denom = Scalar(2) * (h[i-1] + h[i]) - h[i-1] * cp[i-1];
        cp[i] = h[i]/denom;
        dp[i] = (rhs - h[i-1] * dp[i-1])/denom;
    }
    for(size_t i=n-2; i>0; --i)
        M[i] = dp[i] - cp[i] * M[i+1];

    //Polynomial coefficients of each segment
    for(size_t i=0; i<n-1; ++i)
    {
        Vector3 P1 = points[i].T.getOrigin();
        Vector3 P2 = points[i+1].T.getOrigin();
        a.push_back(P1);
        b.push_back((P2 - P1)/h[i] - h[i] * (Scalar(2) * M[i] + M[i+1])/Scalar(6));
        c.push_back(M[i]/Scalar(2));
        d.push_back((M[i+1] - M[i])/(Scalar(6) * h[i]));
    }

    //Rotation: relative rotation of each segment, in local coordinates of its start key point
    //(the rotation vector is invariant under its own rotation, so it is the same in the end key point frame)
    std::vector<Vector3> Phi(n-1);
    for(size_t i=0; i<n-1; ++i)
        Phi[i] = LogRotation(points[i].T.getRotation().inverse() * points[i+1].T.getRotation());

    //Body angular velocity and acceleration at key points (quadratic fit through neighbours, zero acceleration at the ends)
    std::vector<Vector3> w(n), alpha(n);
    w[0] = Phi[0]/h[0];
    alpha[0] = V0();
    w[n-1] = Phi[n-2]/h[n-2];
    alpha[n-1] = V0();
    for(size_t i=1; i<n-1; ++i)
    {
        Vector3 beta = (Phi[i]/h[i] - Phi[i-1]/h[i-1])/(h[i] + h[i-1]);
        w[i] = Phi[i]/h[i] - beta * h[i];
        alpha[i] = Scalar(2) * beta;
    }

    //Quintic Hermite segments in local coordinates, matching the rotation, velocity and acceleration at both ends
    for(size_t i=0; i<n-1; ++i)
    {
        Scalar T = h[i];
        Vector3 v0 = w[i];
        Vector3 a0 = alpha[i];
        Matrix3 Jinv = RightJacobian(Phi[i]).inverse();
        Vector3 v1 = Jinv * w[i+1];
        Vector3 a1 = Jinv * (alpha[i+1] - RightJacobianRate(Phi[i], v1));

        std::array<Vector3, 6> r;
        r[0] = V0();
        r[1] = v0;
        r[2] = a0/Scalar(2);
        r[3] = (Scalar(20) * Phi[i] - (Scalar(8) * v1 + Scalar(12) * v0) * T - (Scalar(3) * a0 - a1) * T * T)/(Scalar(2) * T * T * T);
        r[4] = (Scalar(-30) * Phi[i] + (Scalar(14) * v1 + Scalar(16) * v0) * T + (Scalar(3) * a0 - Scalar(2) * a1) * T * T)/(Scalar(2) * T * T * T * T);
        r[5] = (Scalar(12) * Phi[i] - Scalar(6) * (v1 + v0) * T + (a1 - a0) * T * T)/(Scalar(2) * T * T * T * T * T);
        rot.push_back(r);
    }
}

Vector3 NCSTrajectory::EvalPosition(size_t seg, Scalar u) const
{
    return a[seg] + u * (b[seg] + u * (c[seg] + u * d[seg]));
}

void NCSTrajectory::Interpolate()
{
    if(points.size() < 3 || a.size() != points.size()-1)
    {
        PWLTrajectory::Interpolate();
        return;
    }

    //Find current path segment: t_i <= playTime <= t_{i+1}
    auto it = std::upper_bound(points.begin(), points.end(), playTime,
                               [](Scalar t, const KeyPoint& key){ return t < key.t; });
    size_t i = it == points.begin() ? 0 : (size_t)(it - points.begin()) - 1;
    if(i > points.size()-2)
        i = points.size()-2;

    const KeyPoint& k1 = points[i];
    Scalar u = playTime - k1.t;

    //Linear quantities (analytic derivatives with respect to time)
    interpTrans.setOrigin(EvalPosition(i, u));
    interpVel = b[i] + u * (Scalar(2) * c[i] + Scalar(3) * u * d[i]);
    interpAcc = Scalar(2) * c[i] + Scalar(6) * u * d[i];

    //Angular quantities (analytic, body frame from the right Jacobian, then rotated to world frame)
    const std::array<Vector3, 6>& r = rot[i];
    Vector3 phi = r[0] + u * (r[1] + u * (r[2] + u * (r[3] + u * (r[4] + u * r[5]))));
    Vector3 dphi = r[1] + u * (Scalar(2) * r[2] + u * (Scalar(3) * r[3] + u * (Scalar(4) * r[4] + u * Scalar(5) * r[5])));
    Vector3 ddphi = Scalar(2) * r[2] + u * (Scalar(6) * r[3] + u * (Scalar(12) * r[4] + u * Scalar(20) * r[5]));
    Quaternion q = k1.T.getRotation() * ExpRotation(phi);
    interpTrans.setRotation(q);
    Matrix3 R(q);
    interpAngVel = R * RightJacobianMul(phi, dphi);
    interpAngAcc = R * (RightJacobianMul(phi, ddphi) + RightJacobianRate(phi, dphi));

    //Reversing time flips velocities, while accelerations stay the same
    if(!forward)
    {
        interpVel = -interpVel;
        interpAngVel = -interpAngVel;
    }
}

void NCSTrajectory::BuildGraphicalPath()
{
    PWLTrajectory::BuildGraphicalPath();

    if(!a.empty())
    {
        vis[1].getDataAsPoints()->clear();
        const size_t samples = 20;
        for(size_t i=0; i<a.size(); ++i)
        {
            Scalar h = points[i+1].t - points[i].t;
            for(size_t k=0; k<samples; ++k)
                vis[1].getDataAsPoints()->push_back(glVectorFromVector(EvalPosition(i, h * Scalar(k)/Scalar(samples))));
        }
        vis[1].getDataAsPoints()->push_back(glVectorFromVector(points.back().T.getOrigin()));
    }
}

}
