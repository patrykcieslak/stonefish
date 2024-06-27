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
//  ConfigurableThrusterModels.h
//  Stonefish
//
//  Created by Roger Pi on 03/06/2024
//  Copyright (c) 2024 Stonefish. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include <memory>

namespace sf
{
namespace rm
{

class RotorDynamics
{
protected:
  Scalar last_output_;
  Scalar last_time_;

public:
  virtual Scalar f(Scalar time, Scalar sp) = 0;

  Scalar getLastOutput() const
  {
    return last_output_;
  }
  Scalar getLastTime() const
  {
    return last_time_;
  }
};

// Here we will implement several dynamic models

class ZeroOrder : public RotorDynamics
{
public:
  // implement a Zero Order model

  Scalar f(Scalar time, Scalar sp)
  {
    return sp;
  }
};

class FirstOrder : public RotorDynamics
{
private:
  Scalar tau_;

public:
  FirstOrder(Scalar tau) : tau_(tau)
  {
  }

  // implement a First Order model

  Scalar f(Scalar time, Scalar sp)
  {
    Scalar dt = time - last_time_;
    Scalar alpha = dt / tau_;

    Scalar output = alpha * sp + (1 - alpha) * last_output_;

    last_output_ = output;
    last_time_ = time;

    return output;
  }
};

class Yoerger : public RotorDynamics
{
private:
  Scalar alpha_;
  Scalar beta_;

public:
  Yoerger(Scalar alpha, Scalar beta) : alpha_(alpha), beta_(beta)
  {
  }

  Scalar f(Scalar time, Scalar sp)
  {
    Scalar dt = time - last_time_;

    //  state += dt*(beta*_cmd - alpha*state*std::abs(state));

    Scalar output = last_output_ + dt * (beta_ * sp - (alpha_ * last_output_ * std::abs(last_output_)));

    last_output_ = output;
    last_time_ = time;

    return output;
  }
};

class Bessa : public RotorDynamics
{
private:
  Scalar Jmsp_;
  Scalar Kv1_;
  Scalar Kv2_;
  Scalar Kt_;
  Scalar Rm_;

public:
  Bessa(Scalar Jmsp, Scalar Kv1, Scalar Kv2, Scalar Kt, Scalar Rm) : Jmsp_(Jmsp), Kv1_(Kv1), Kv2_(Kv2), Kt_(Kt), Rm_(Rm)
  {
  }

  Scalar f(Scalar time, Scalar sp)
  {
    Scalar dt = time - last_time_;

    Scalar output = last_output_ +
                    dt * (sp * Kt_ / Rm_ - Kv1_ * last_output_ - Kv2_ * last_output_ * std::abs(last_output_)) / Jmsp_;

    last_output_ = output;
    last_time_ = time;

    return output;
  }
};

}  // namespace rm

namespace tm
{

class ThrustModel
{
public:
  virtual Scalar f(Scalar val) = 0;
};

class BasicThrustConversion : public ThrustModel
{
protected:
  Scalar cb_;

public:
  BasicThrustConversion(Scalar cb) : cb_(cb)
  {
  }

  Scalar f(Scalar val) override
  {
    return cb_ * val * std::abs(val);
  }
};

class DeadBandConversion : public ThrustModel
{
protected:
  Scalar c_l_;
  Scalar c_r_;
  Scalar d_l_;
  Scalar d_r_;

public:
  DeadBandConversion(Scalar c_l, Scalar c_r, Scalar d_l, Scalar d_r) : c_l_(c_l), c_r_(c_r), d_l_(d_l), d_r_(d_r)
  {
  }

  Scalar f(Scalar val) override
  {
    Scalar vv = val * std::abs(val);
    if (vv < d_l_)
    {
      return c_l_ * (vv - d_l_);
    }
    else if (vv > d_r_)
    {
      return c_r_ * (vv - d_r_);
    }
    else
    {
      return 0;
    }
  }
};
}  // namespace tm
}  // namespace sf
