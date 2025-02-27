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
/*! \file
  MoreauJeanOSI Time-Integrator for Dynamical Systems for Combined Projection
  Algorithm
*/

#ifndef MOREAUCOMBINEDPROJECTIONOSI_H
#define MOREAUCOMBINEDPROJECTIONOSI_H

#include "MoreauJeanOSI.hpp"
#include "OneStepIntegrator.hpp"
#include "SimpleMatrix.hpp"

const unsigned int MOREAUCOMBINEDPROJECTIONOSISTEPSINMEMORY = 1;

/**
    One Step time Integrator for First Order Dynamical Systems  for
    mechanical Systems (LagrangianDS and NewtonEulerDS) with  Combined
    Projection Algorithm

    This class reimplement a special activation of constraints
    in the MoreauJeanOSI for the Combined Projection Algorithm

    References :

    V. Acary. Projected event-capturing time-stepping schemes for nonsmooth
    mechanical systems with unilateral contact and coulomb’s friction. Computer
    Methods in Applied Mechanics and Engineering, 256:224 – 250, 2013. ISSN
    0045-7825. URL
    http://www.sciencedirect.com/science/article/pii/S0045782512003829.

*/
class MoreauJeanCombinedProjectionOSI : public MoreauJeanOSI {
protected:
  ACCEPT_SERIALIZATION(MoreauJeanCombinedProjectionOSI);

  /** Default constructor
   */
  MoreauJeanCombinedProjectionOSI(){};

public:
  /** constructor from theta value only
   *
   *  \param theta value for all these DS.
   */
  explicit MoreauJeanCombinedProjectionOSI(double theta);

  /** destructor
   */
  virtual ~MoreauJeanCombinedProjectionOSI(){};

  // --- OTHER FUNCTIONS ---

  /** initialization of the work vectors and matrices (properties) related to
   *  one dynamical system on the graph and needed by the osi
   *
   *  \param t time of initialization
   *  \param ds the dynamical system
   */
  void initializeWorkVectorsForDS(double t, SP::DynamicalSystem ds) override;

  /** initialization of the work vectors and matrices (properties) related to
   *  one interaction on the graph and needed by the osi
   *
   *  \param inter the interaction
   *  \param interProp the properties on the graph
   *  \param DSG the dynamical systems graph
   */
  void initializeWorkVectorsForInteraction(Interaction &inter,
                                           InteractionProperties &interProp,
                                           DynamicalSystemsGraph &DSG) override;

  /** get the number of index sets required for the simulation
   *
   *  \return unsigned int
   */
  unsigned int numberOfIndexSets() const override { return 3; };

  /** Apply the rule to one Interaction to known if is it should be included
   *  in the IndexSet of level i
   *
   *  \param inter concerned interaction
   *  \param i level
   *  \return bool
   */
  bool addInteractionInIndexSet(SP::Interaction inter, unsigned int i) override;

  /** Apply the rule to one Interaction to known if is it should be removed
   *  in the IndexSet of level i
   *
   *  \param inter concerned interaction
   *  \param i level
   *  \return bool
   */
  bool removeInteractionFromIndexSet(SP::Interaction inter, unsigned int i) override;

  ACCEPT_STD_VISITORS();
};

#endif // MOREAUCOMBINEDPROJECTIONOSI_H
