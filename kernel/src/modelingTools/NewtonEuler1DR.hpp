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
/*! \file NewtonEuler1DR.hpp

 */
#ifndef NEWTONEULERIMPACT_H
#define NEWTONEULERIMPACT_H

#include "NewtonEulerDS.hpp"
#include "NewtonEulerR.hpp"

/**
   This class is an interface for a relation with impact.  It
   implements the computation of the jacoboian of h from the points of
   contacts and the normal.  Use this class consists in overloading
   the method computeh, by setting the member pc1, pc2, nc and y.  The
   matrix jachq is used both for the building of the OSNSP (with T)
   and for the predictor of activation of deactivation of the Interaction.

*/
class NewtonEuler1DR : public NewtonEulerR {
protected:
  ACCEPT_SERIALIZATION(NewtonEuler1DR);

  /** Current Contact Points, may be updated within Newton loop based
   * on _relPc1, _relPc2. */
  SP::SiconosVector _Pc1;
  SP::SiconosVector _Pc2;

  /** Contact Points in coordinates relative to attached DS->q.  Set
   * these if _Pc1/_Pc2 are not calculated within the Newton loop. */
  SP::SiconosVector _relPc1;
  SP::SiconosVector _relPc2;

  /** Inward Normal at the contact.
   *  \todo The meaning of "Inward" has to be explained carefully.
   */
  SP::SiconosVector _Nc;

  /** _Nc must be calculated relative to q2 */
  SP::SiconosVector _relNc;

  /** Rotation matrix converting the absolute coordinate to the contact frame
   *  coordinate. This matrix contains the unit vector(s)of the contact frame in
   *  row.
   */
  SP::SimpleMatrix _rotationAbsoluteToContactFrame;

  /** Matrix converting */
  SP::SimpleMatrix _rotationBodyToAbsoluteFrame;

  /** Cross product matrices that correspond the lever arm from
   *  contact point to center of mass*/
  SP::SimpleMatrix _NPG1;
  SP::SimpleMatrix _NPG2;

  /*buffer matrices*/
  SP::SimpleMatrix _AUX1;
  SP::SimpleMatrix _AUX2;

  /** Set the coordinates of first contact point.  Must only be done
   *  in a computeh() override.
   *
   *  \param npc new coordinates
   */
  void setpc1(SP::SiconosVector npc) { _Pc1 = npc; };

  /** Set the coordinates of second contact point.  Must only be done
   *  in a computeh() override.
   *
   *  \param npc new coordinates
   */
  void setpc2(SP::SiconosVector npc) { _Pc2 = npc; };

  /** Set the coordinates of inside normal vector at the contact point.
   *  Must only be done in a computeh() override.
   *
   *  \param nnc new coordinates
   */
  void setnc(SP::SiconosVector nnc) { _Nc = nnc; };

private:
  void NIcomputeJachqTFromContacts(SP::SiconosVector q1);
  void NIcomputeJachqTFromContacts(SP::SiconosVector q1, SP::SiconosVector q2);

public:
  /** V.A. boolean _isOnCOntact ?? Why is it public members ?
   *  seems parametrize the projection algorithm
   *  the projection is done on the surface  \f$ y=0 \f$  or on  \f$ y \geq 0 \f$
   */
  bool _isOnContact = false;

  /** constructor
   */
  NewtonEuler1DR()
      : NewtonEulerR(), _Pc1(new SiconosVector(3)), _Pc2(new SiconosVector(3)),
        _relPc1(new SiconosVector(3)), _relPc2(new SiconosVector(3)),
        _Nc(new SiconosVector(3)), _relNc(new SiconosVector(3))
  {
    /*_ds1=nullptr;_ds2=nullptr;*/
  }

  /** destructor
   */
  virtual ~NewtonEuler1DR(){};

  void computeJachq(double time, Interaction &inter,
                            SP::BlockVector q0) override;

  void initialize(Interaction &inter) override;

  /** Default implementation consists in multiplying jachq and T (see
   *  NewtonEulerR::computeJachqT) but here we compute the operator from the the
   *  contact point locations and the local frame at contact
   *
   *  \param inter interaction that owns the relation
   *  \param q0 the block vector to the dynamical system position
   */
  void computeJachqT(Interaction &inter, SP::BlockVector q0) override;

  /**
      to compute the output y = h(t,q,z) of the Relation

      \param time current time value
      \param q coordinates of the dynamical systems involved in the relation
      \param y the resulting vector
  */
  void computeh(double time, const BlockVector &q0, SiconosVector &y) override;

  /**
      to compute the output y = h(t,q,z) of the Relation
      with the relative contact points
      \param time current time value
      \param q coordinates of the dynamical systems involved in the relation
      \param y the resulting vector
   */
  void computehFromRelativeContactPoints(double time, const BlockVector &q0, SiconosVector &y);

  /** Return the distance between pc1 and pc, with sign according to normal */
  double distance() const;

  inline SP::SiconosVector pc1() const { return _Pc1; }
  inline SP::SiconosVector pc2() const { return _Pc2; }
  inline SP::SiconosVector nc() const { return _Nc; }

  inline SP::SiconosVector relPc1() const { return _relPc1; }
  inline SP::SiconosVector relPc2() const { return _relPc2; }
  inline SP::SiconosVector relNc() const { return _relNc; }

  /** Set the coordinates of first contact point in ds1 frame.
   *  It will be used to compute _Pc1 during computeh().
   *
   *  \param npc new coordinates
   */
  void setRelPc1(SP::SiconosVector npc) { _relPc1 = npc; };

  /** Set the coordinates of second contact point in ds2 frame
   *  It will be used to compute _Pc2 during computeh().
   *
   *  \param npc new coordinates
   */
  void setRelPc2(SP::SiconosVector npc) { _relPc2 = npc; };

  /** Set the coordinates of inside normal vector at the contact point in ds2
   *  frame. It will be used to compute _Nc during computeh().
   *
   *  \param nnc new coordinates
   */
  void setRelNc(SP::SiconosVector nnc) { _relNc = nnc; };
  void display() const  override{}

  ACCEPT_STD_VISITORS();
};
#endif // NEWTONEULERRIMPACT_H
