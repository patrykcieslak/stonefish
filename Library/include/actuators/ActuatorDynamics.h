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
//  ActuatorDynamics.h
//  Stonefish
//
//  Created by Roger Pi on 03/06/2024
//  Modified by Patryk Cieslak on 30/06/2024
//  Copyright (c) 2024 Roger Pi and Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ActuatorDynamics__
#define __Stonefish_ActuatorDynamics__

#include "StonefishCommon.h"
#include <memory>

namespace sf
{
    enum class RotorDynamicsType {ZERO_ORDER, FIRST_ORDER, YOEGER, BESSA, MECHANICAL_PI};
    
    //! An abstract class representing a mathematical model of rotor dynamics.
    class RotorDynamics
    {
    public:
        //! A constructor.
        RotorDynamics() : lastOutput(0) 
        {}

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        virtual Scalar Update(Scalar dt, Scalar sp) = 0;

        //! A method returning the model type.
        virtual RotorDynamicsType getType() = 0;

    protected:
        Scalar lastOutput;
    };

    // ---------- Implemententation of several models of rotor dynamics -----------

    //! A class representing a the zero order dynamics model - passthrough.
    class ZeroOrder : public RotorDynamics
    {
    public:
        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            return sp;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::ZERO_ORDER;
        }
    };

    //! A class representing the first order dynamics model.
    class FirstOrder : public RotorDynamics
    {
    public:
        //! A constructor
        /*!
          \param tau time constant
        */
        FirstOrder(Scalar tau) : tau(tau)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar alpha = dt / tau;
            Scalar output = alpha * sp + (1 - alpha) * lastOutput;
            lastOutput = output;
            return output;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::FIRST_ORDER;
        }
    
    private:
        Scalar tau;
    };

    //! A class representing the Yoerger dynamics model.
    class Yoerger : public RotorDynamics
    {
    public:
        //! A constructor.
        /*!
          \param alpha
          \param beta
        */
        Yoerger(Scalar alpha, Scalar beta) : alpha(alpha), beta(beta)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            // state += dt*(beta*_cmd - alpha*state*std::abs(state));
            Scalar output = lastOutput + dt * (beta * sp - (alpha * lastOutput * btFabs(lastOutput)));
            lastOutput = output;
            return output;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::YOEGER;
        }

    private:
        Scalar alpha;
        Scalar beta;
    };

    //! A class representing the Bessa dynamics model.
    class Bessa : public RotorDynamics
    {
    public:
        //! A constructor.
        /*!
          \param Jmsp
          \param Kv1
          \param Kv2
          \param Kt
          \param Rm
        */
        Bessa(Scalar Jmsp, Scalar Kv1, Scalar Kv2, Scalar Kt, Scalar Rm) 
            : Jmsp(Jmsp), Kv1(Kv1), Kv2(Kv2), Kt(Kt), Rm(Rm)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar output = lastOutput +
                dt * (sp * Kt/Rm - Kv1 * lastOutput - Kv2 * lastOutput * btFabs(lastOutput))/Jmsp;
            lastOutput = output;
            return output;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::BESSA;
        }

    private:
        Scalar Jmsp;
        Scalar Kv1;
        Scalar Kv2;
        Scalar Kt;
        Scalar Rm;
    };

    //! A classs representing a mechnical shaft model with a PI controller.
    class MechanicalPI : public RotorDynamics
    {
    public:
        //! A constructor.
        /*!
          \param J moment of inertia of the rotor [kgm2]
          \param Kp proportional gain of the PI controller [1]
          \param Ki integral gain of the PI controller [1]
          \param iLim integral limit [rad/s]
        */
        MechanicalPI(Scalar J, Scalar Kp, Scalar Ki, Scalar iLim)
            : J(J), Kp(Kp), Ki(Ki), iLim(iLim), iError(0), damping(0)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar error = sp - lastOutput;
            Scalar tau = Kp * error + Ki * iError;
            iError = btClamped(iError + error * dt, -iLim, iLim);

            Scalar tauD = lastOutput > Scalar(0) ? damping : -damping;
            Scalar output = lastOutput + (tau - tauD)/J * dt;
            lastOutput = output;
            return output;
        }

        //! A method used to update the damping torque.
        /*!
          \param damping absolute value of the damping torque [Nm]
        */
        void setDampingTorque(Scalar tau)
        {
            damping = btFabs(tau);
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::MECHANICAL_PI;
        }

    private:
        Scalar J;
        Scalar Kp;
        Scalar Ki;
        Scalar iLim;
        Scalar iError;
        Scalar damping;
    };

    // ----------------------------------------------------------------

    enum class ThrustModelType {QUADRATIC, DEADBAND, LINTERP, FD};

    //! An abstract class representing a mathematical model of thrust and torque generated by thrusters and propellers.
    class ThrustModel
    {
    public:
        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        virtual std::pair<Scalar, Scalar> Update(Scalar input) = 0;

        //! A method returning the type of the model.
        virtual ThrustModelType getType() = 0;
    };

    // ---------- Implemententation of several models of thrust generation -----------

    //! A class representing a basic quadratic model.
    class QuadraticThrust : public ThrustModel
    {
    public:
        //! A constructor.
        /*!
          \param kt thrust coefficient
        */
        QuadraticThrust(Scalar kt) : kt(kt)
        {
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            Scalar thrust = kt * input * btFabs(input);
            return std::make_pair(thrust, Scalar(0));
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::QUADRATIC;
        }
    
    protected:
        Scalar kt;
    };

    //! A class representing a dead band model.
    class DeadbandThrust : public ThrustModel
    {
    public:
        //! A constructor.
        /*!
          \param ktn negative thrust coefficient
          \param ktp positive thrust coefficient
          \param dl lower limit of the deadband
          \param du upper limit of the deadband
        */
        DeadbandThrust(Scalar ktn, Scalar ktp, Scalar dl, Scalar du) : ktn(ktn), ktp(ktp), dl(dl), du(du)
        {
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            Scalar vv = input * btFabs(input);
            Scalar thrust(0);
            if (vv < dl)
            {
                thrust = ktn * (vv - dl);
            }
            else if (vv > du)
            {
                thrust = ktp * (vv - du);
            }
            return std::make_pair(thrust, Scalar(0));
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::DEADBAND;
        }
        
    protected:
        Scalar ktn;
        Scalar ktp;
        Scalar dl;
        Scalar du;
    };
    
    //! A class representing a model based on linear interpolation of data points.
    class InterpolatedThrust : public ThrustModel
    {
    public:
        //! A constructor.
        /*!
          \param in list of angular velocity data points
          \param out list of thrust data points
        */
        InterpolatedThrust(const std::vector<Scalar>& in, const std::vector<Scalar>& out)
            : inputValues(in), outputValues(out)
        {
            if (inputValues.empty() || outputValues.empty())
                throw std::runtime_error("Interpolated thrust model: input and output values must not be empty!");

            if (inputValues.size() != outputValues.size())
                throw std::runtime_error("Interpolated thrust model: input and output values must be same size!");
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            Scalar thrust(0);

            // Ensure the input values are sorted
            auto it = std::lower_bound(inputValues.begin(), inputValues.end(), input);

            if (it == inputValues.begin()) // If the value is less than the smallest input value, return the first output value
            {
                thrust = outputValues.front();
            }
            else if (it == inputValues.end()) // If the value is greater than the largest input value, return the last output value
            {
                thrust = outputValues.back();
            }
            else
            {
                // Perform linear interpolation
                auto idx = std::distance(inputValues.begin(), it);
                Scalar x0 = inputValues[idx - 1];
                Scalar x1 = inputValues[idx];
                Scalar y0 = outputValues[idx - 1];
                Scalar y1 = outputValues[idx];
                thrust = y0 + (input - x0) * (y1 - y0) / (x1 - x0);
            }
            return std::make_pair(thrust, Scalar(0));
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::LINTERP;
        }

        protected:
            std::vector<Scalar> inputValues, outputValues;
    };

    //! A class representing a realistic model based of fluid dynamics.
    class FDThrust : public ThrustModel
    {
    public:
        //! A constructor.
        /*!
          \param D diameter of the propeller [m]
          \param ktp positive thrust coefficient
          \param ktn negative thrust coefficient
          \param kq torque coefficient
          \param RH flag informing if the propeller is right-handed
        */
        FDThrust(Scalar D, Scalar ktp, Scalar ktn, Scalar kq, bool RH, Scalar rho)
            : D(D), ktp(ktp), ktn(ktn), kq(kq), RH(RH), rho(rho)
        {
            // TODO: Find a better way of defining alpha and beta
            alpha = -ktp;
            beta = -kq;
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            bool backward = (RH && input < Scalar(0)) || (!RH && input > Scalar(0));
            
            /*kt and kq depend on the advance ratio J
                J = u/(omega*D), where:
                u - ambient velocity [m/s]
                n - propeller rotational rate [1/s]
                D - propeller diameter [m] */
            Scalar n = (backward ? Scalar(-1) : Scalar(1)) * btFabs(input)/(Scalar(2) * M_PI); // Accounts for propoller handedness
            
            // Thrust is the force generated by pushing the liquid through the thruster.
            Scalar kt0 = backward ? ktn : ktp; //In case of non-symmetrical thrusters the coefficient may be different
            //kt(J) = kt0 + alpha * J --> approximated with linear function
            Scalar thrust = rho * D*D*D * btFabs(n) * (D*kt0*n + alpha*u);
            
            // Torque is the loading of propeller due to liquid resistance (reaction force).
            Scalar kq0 = kq;
            //kQ(J) = kQ0 + beta * J --> approximated with linear function
            Scalar torque = (RH ? Scalar(-1) : Scalar(1)) * rho * D*D*D*D * btFabs(n) * (D*kq0*n + beta*u);

            return std::make_pair(thrust, torque);
        }

        //! A method used to set incoming fluid velocity.
        /*!
          \param vel velocity of the fluid coming into thruster [m/s]
        */
        void setIncomingFluidVelocity(Scalar vel)
        {
            u = vel;
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::FD;
        }
    
    protected:
        Scalar D;
        Scalar ktp;
        Scalar ktn;
        Scalar kq;
        Scalar u;
        bool RH;
        Scalar rho;
        Scalar alpha;
        Scalar beta;
    };

} // namespace sf

#endif