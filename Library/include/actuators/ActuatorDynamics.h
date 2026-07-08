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
//  Copyright (c) 2024-2026 Roger Pi and Patryk Cieslak. All rights reserved.
//

#pragma once

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
        RotorDynamics() : lastOutput_(0), outputLimit_(-1)
        {}

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        virtual Scalar Update(Scalar dt, Scalar sp) = 0;

        //! A method returning the model type.
        virtual RotorDynamicsType getType() = 0;

        //! A method used to set the limit of output.
        /*!
          \param limit the absolute limit of the output [rad/s]
        */
        void setOutputLimit(Scalar limit)
        {
            outputLimit_ = limit;
        }

    protected:
        Scalar lastOutput_;
        Scalar outputLimit_;
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
        FirstOrder(Scalar tau) : tau_(tau)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar alpha = dt / tau_;
            Scalar output = alpha * sp + (1 - alpha) * lastOutput_;
            lastOutput_ = outputLimit_ > Scalar(0) ?  btClamped(output, -outputLimit_, outputLimit_) : output;
            return lastOutput_;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::FIRST_ORDER;
        }
    
    private:
        Scalar tau_;
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
        Yoerger(Scalar alpha, Scalar beta) : alpha_(alpha), beta_(beta)
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
            Scalar output = lastOutput_ + dt * (beta_ * sp - (alpha_ * lastOutput_ * btFabs(lastOutput_)));
            lastOutput_ = outputLimit_ > Scalar(0) ? btClamped(output, -outputLimit_, outputLimit_) : output;
            return lastOutput_;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::YOEGER;
        }

    private:
        Scalar alpha_;
        Scalar beta_;
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
            : Jmsp_(Jmsp), Kv1_(Kv1), Kv2_(Kv2), Kt_(Kt), Rm_(Rm)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar output = lastOutput_ +
                dt * (sp * Kt_/Rm_ - Kv1_ * lastOutput_ - Kv2_ * lastOutput_ * btFabs(lastOutput_))/Jmsp_;
            lastOutput_ = outputLimit_ > Scalar(0) ?  btClamped(output, -outputLimit_, outputLimit_) : output;
            return lastOutput_;
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::BESSA;
        }

    private:
        Scalar Jmsp_;
        Scalar Kv1_;
        Scalar Kv2_;
        Scalar Kt_;
        Scalar Rm_;
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
            : J_(J), Kp_(Kp), Ki_(Ki), iLim_(iLim), iError_(0), damping_(0)
        {
        }

        //! A method that updates the model.
        /*!
          \param dt simulation time step [s]
          \param sp desired rotor angular velocity [rad/s]
        */
        Scalar Update(Scalar dt, Scalar sp) override
        {
            Scalar error = sp - lastOutput_;
            Scalar tau = Kp_ * error + Ki_ * iError_;
            iError_ = btClamped(iError_ + error * dt, -iLim_, iLim_);

            Scalar tauD = lastOutput_ > Scalar(0) ? damping_ : -damping_;
            Scalar output = lastOutput_ + (tau - tauD)/J_ * dt;
            lastOutput_ = outputLimit_ > Scalar(0) ?  btClamped(output, -outputLimit_, outputLimit_) : output;
            return lastOutput_;
        }

        //! A method used to update the damping torque.
        /*!
          \param damping absolute value of the damping torque [Nm]
        */
        void setDampingTorque(Scalar tau)
        {
            damping_ = btFabs(tau);
        }

        //! A method returning the model type.
        RotorDynamicsType getType()
        {
            return RotorDynamicsType::MECHANICAL_PI;
        }

    private:
        Scalar J_;
        Scalar Kp_;
        Scalar Ki_;
        Scalar iLim_;
        Scalar iError_;
        Scalar damping_;
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
        QuadraticThrust(Scalar kt) : kt_(kt)
        {
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            Scalar thrust = kt_ * input * btFabs(input);
            return std::make_pair(thrust, Scalar(0));
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::QUADRATIC;
        }
    
    protected:
        Scalar kt_;
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
        DeadbandThrust(Scalar ktn, Scalar ktp, Scalar dl, Scalar du) : ktn_(ktn), ktp_(ktp), dl_(dl), du_(du)
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
            if (vv < dl_)
            {
                thrust = ktn_ * (vv - dl_);
            }
            else if (vv > du_)
            {
                thrust = ktp_ * (vv - du_);
            }
            return std::make_pair(thrust, Scalar(0));
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::DEADBAND;
        }
        
    protected:
        Scalar ktn_;
        Scalar ktp_;
        Scalar dl_;
        Scalar du_;
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
            : inputValues_(in), outputValues_(out)
        {
            if (inputValues_.empty() || outputValues_.empty())
                throw std::runtime_error("Interpolated thrust model: input and output values must not be empty!");

            if (inputValues_.size() != outputValues_.size())
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
            auto it = std::lower_bound(inputValues_.begin(), inputValues_.end(), input);

            if (it == inputValues_.begin()) // If the value is less than the smallest input value, return the first output value
            {
                thrust = outputValues_.front();
            }
            else if (it == inputValues_.end()) // If the value is greater than the largest input value, return the last output value
            {
                thrust = outputValues_.back();
            }
            else
            {
                // Perform linear interpolation
                auto idx = std::distance(inputValues_.begin(), it);
                Scalar x0 = inputValues_[idx - 1];
                Scalar x1 = inputValues_[idx];
                Scalar y0 = outputValues_[idx - 1];
                Scalar y1 = outputValues_[idx];
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
            std::vector<Scalar> inputValues_;
            std::vector<Scalar> outputValues_;
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
            : D_(D), ktp_(ktp), ktn_(ktn), kq_(kq), RH_(RH), rho_(rho)
        {
            // TODO: Find a better way of defining alpha and beta
            alpha_ = -ktp;
            beta_ = -kq;
        }

        //! A method computing the model output.
        /*!
          \param input the input to the model
          \return a pair of thrust and torque computed by the model
        */
        std::pair<Scalar, Scalar> Update(Scalar input) override
        {
            bool backward = (RH_ && input < Scalar(0)) || (!RH_ && input > Scalar(0));
            
            /*kt and kq depend on the advance ratio J
                J = u/(omega*D), where:
                u - ambient velocity [m/s]
                n - propeller rotational rate [1/s]
                D - propeller diameter [m] */
            Scalar n = (backward ? Scalar(-1) : Scalar(1)) * btFabs(input)/(Scalar(2) * M_PI); // Accounts for propoller handedness
            
            // Thrust is the force generated by pushing the liquid through the thruster.
            Scalar kt0 = backward ? ktn_ : ktp_; //In case of non-symmetrical thrusters the coefficient may be different
            //kt(J) = kt0 + alpha * J --> approximated with linear function
            Scalar thrust = rho_ * D_*D_*D_ * btFabs(n) * (D_*kt0*n + alpha_*u_);
            
            // Torque is the loading of propeller due to liquid resistance (reaction force).
            Scalar kq0 = kq_;
            //kQ(J) = kQ0 + beta * J --> approximated with linear function
            Scalar torque = (RH_ ? Scalar(-1) : Scalar(1)) * rho_ * D_*D_*D_*D_ * btFabs(n) * (D_*kq0*n + beta_*u_);

            return std::make_pair(thrust, torque);
        }

        //! A method used to set incoming fluid velocity.
        /*!
          \param vel velocity of the fluid coming into thruster [m/s]
        */
        void setIncomingFluidVelocity(Scalar vel)
        {
            u_ = vel;
        }

        //! A method returning the type of the model.
        ThrustModelType getType() override
        {
            return ThrustModelType::FD;
        }
    
    protected:
        Scalar D_;
        Scalar ktp_;
        Scalar ktn_;
        Scalar kq_;
        Scalar u_;
        bool RH_;
        Scalar rho_;
        Scalar alpha_;
        Scalar beta_;
    };

} // namespace sf
