/* Siconos is a program dedicated to modeling, simulation and control
 * of non smooth dynamical systems.
 *
 * Copyright 2022 INRIA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** \file CircleCircleR.hpp
 */
#ifndef CircularR_h
#define CircularR_h

#include "Interaction.hpp"
#include "LagrangianScleronomousR.hpp"
#include "MechanicsFwd.hpp"

/** 
    Two circle relation - Inherits from LagrangianScleronomousR
*/

class CircularR : public LagrangianScleronomousR {
protected:
  ACCEPT_SERIALIZATION(CircularR);

  double _r1, _r2;

public:
  /** Constructor
   *
   *  \param disk1 radius
   *  \param disk2 radius
   */
  CircularR(double r1, double r2) : _r1(r1), _r2(r2){};

  virtual ~CircularR() noexcept = default;

  double getRadius1() const { return _r1; };

  double getRadius2() const { return _r2; };

  virtual double distance(double, double, double, double, double, double) {
    assert(0);
    return (0);
  };

};
#endif /* CircularR_h */
