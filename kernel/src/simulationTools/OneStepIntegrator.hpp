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
/*! \file OneStepIntegrator.hpp

  Base class (i.e. common interface) for dynamical system integration over a time step.
*/

#ifndef ONESTEPINTEGRATOR_H
#define ONESTEPINTEGRATOR_H

#include "SiconosVisitor.hpp" // for VIRTUAL_ACCEPT_VISITORS
#include "SiconosException.hpp"
#include "SimulationTypeDef.hpp"
#include "OneStepIntegratorTypes.hpp"
#include "SimulationGraphs.hpp"
#include "Simulation.hpp"

/**
   Generic class to manage DynamicalSystem(s) time-integration

   This is a virtual class, interface for some specific integrators.

   At the time, available integrators are:

   - EulerMoreauOSI
   - MoreauJeanOSI
   - MoreauJeanCombinedProjectionOSI
   - MoreauJeanDirectProjectionOSI
   - MoreauJeanBilbaoOSI
   - D1MinusLinearOSI
   - SchatzmanPaoliOSI
   - LsodarOSI
   - Hem5OSI
   - NewMarkAlphaOSI
   - ZeroOrderHoldOSI
*/
class OneStepIntegrator :public std::enable_shared_from_this<OneStepIntegrator>
{

protected:

  ACCEPT_SERIALIZATION(OneStepIntegrator);

  /** type/name of the Integrator */
  OSI::TYPES _integratorType;

  /** a graph of dynamical systems to integrate
   *  For the moment, we point to the graph of dynamical systems in
   *  the topology. We use the properties "osi" to check if the dynamical
   *  system is integrated by this osi. It has to be improved by using a subgraph
   *  to avoid the use of checkOSI
   */
  SP::DynamicalSystemsGraph _dynamicalSystemsGraph;

  /** size of the memory for the integrator */
  unsigned int _sizeMem;

  /** steps of the integrator */
  unsigned int _steps;

  /** _levelMinForOutput is the minimum level for the output
   *  needed by the OneStepIntegrator
   */
  unsigned int _levelMinForOutput;

  /** _levelMaxForOutput is the maximum level for the output
   *  needed by the OneStepIntegrator
   */
  unsigned int _levelMaxForOutput;

  /** _levelMinForInput is the minimum level for the input
   *  needed by the OneStepIntegrator
   */
  unsigned int _levelMinForInput;

  /** _levelMaxForInput is the maximum level for the input
   *  needed by the OneStepIntegrator
   */
  unsigned int _levelMaxForInput;

  bool _isInitialized;

  /** boolean variable to force an explicit evaluation of the Jacobians
   *  mapping of relations only at the beginning of the time--step and
   *  not in the Newton iteration
   */

  bool _explicitJacobiansOfRelation;



  /** A link to the simulation that owns this OSI */
  SP::Simulation _simulation;

  /** basic constructor with OSI Id
   *
   *  \param type integrator type/name
   */
  OneStepIntegrator(const OSI::TYPES& type)
    : _integratorType(type), _sizeMem(1), _steps(0),
      _levelMinForOutput(0), _levelMaxForOutput(0),
      _levelMinForInput(0), _levelMaxForInput(0),
      _isInitialized(false), _explicitJacobiansOfRelation(false) {};

  /** struct to add terms in the integration. Useful for Control */
  SP::ExtraAdditionalTerms _extraAdditionalTerms;

  /**
     Compare interaction and current OSI levels for input and output.
     Reset interaction if they are not compliant.

     \param inter a reference to an Interaction
  */
  void _check_and_update_interaction_levels(Interaction& inter);

  /** initialization of the work vectors and matrices (properties) related to
   *  one dynamical system on the graph and needed by the osi -- common code.
   *
   *  \param ds the dynamical system
   */
  SP::VectorOfVectors _initializeDSWorkVectors(SP::DynamicalSystem ds);

  /** default constructor */
  OneStepIntegrator() {};

private:

  /** copy constructor, private, no copy nor pass-by value allowed */
  OneStepIntegrator(const OneStepIntegrator&);

  /** assignment (private => forbidden)
   *
   *  \param  OSI
   *  \return OneStepIntegrator&
   */
  OneStepIntegrator& operator=(const OneStepIntegrator& OSI);


public:

  /** destructor
   */
  virtual ~OneStepIntegrator() {};

  /** \return the id of the integrator (see list in OSI::TYPES enum)
   */
  inline OSI::TYPES getType() const
  {
    return _integratorType;
  }

  /**
     get the graph of dynamical systems associated with the Integrator
     warning: returns the whole ds graph, not only ds integrated by the present osi.

     \return a SP::DynamicalSystemsGraph
  */
  inline SP::DynamicalSystemsGraph dynamicalSystemsGraph() const
  {
    return _dynamicalSystemsGraph;
  };

  /**
     set the graph of dynamical systems associated with the Integrator
  */
  inline void setDynamicalSystemsGraph(SP::DynamicalSystemsGraph dsg)
  {
    _dynamicalSystemsGraph = dsg;
  };

  /** get number of internal memory vectors needed in dynamical systems integrated with this osi.
   *
   *  \return an unsigned int
   */
  inline unsigned int getSizeMem() const
  {
    return _sizeMem;
  };

  /** get the Simulation that owns the OneStepIntegrator (pointer link)
   *
   *  \return a pointer to Simulation
   */
  inline SP::Simulation simulation() const
  {
    return _simulation;
  }

  /** set the Simulation of the OneStepIntegrator
   *
   *  \param newS a pointer to Simulation
   */
  inline void setSimulationPtr(SP::Simulation newS)
  {
    _simulation = newS;
  }

  /**
     minimal level required for output var used with this integration scheme.
     var[level] is the derivative of order 'level' of var.
  */
  virtual unsigned int levelMinForOutput()
  {
    return _levelMinForOutput;
  }

  /**
      maximal level required for output var used with this integration scheme.
      var[level] is the derivative of order 'level' of var.
   */
  virtual unsigned int levelMaxForOutput()
  {
    return _levelMaxForOutput;
  }

  /**
      minimal level required for input var used with this integration scheme.
      var[level] is the derivative of order 'level' of var.
   */
  virtual unsigned int levelMinForInput()
  {
    return _levelMinForInput;
  }

  /**
      maximal level required for input var used with this integration scheme.
      var[level] is the derivative of order 'level' of var.
   */
  virtual unsigned int levelMaxForInput()
  {
    return _levelMaxForInput;
  }

  /** get the number of index sets required for the simulation
   *
   *  \return unsigned int
   */
  virtual unsigned int numberOfIndexSets() const = 0;

  inline bool isInitialized(){return _isInitialized;};

  inline void setIsInitialized( bool value) {_isInitialized = value;};

  bool explicitJacobiansOfRelation()
  {
  return  _explicitJacobiansOfRelation;
  }

  void setExplicitJacobiansOfRelation(bool newval)
  {
    _explicitJacobiansOfRelation = newval;
  };

  /** initialise the integrator
   */
  virtual void initialize();

  /**
     Initialization process of the nonsmooth problems
     linked to this OSI*/
  virtual void initialize_nonsmooth_problems(){};

  /** initialization of the work vectors and matrices (properties) related to
   *  one dynamical system on the graph and needed by the osi
   *
   *  \param t time of initialization
   *  \param ds the dynamical system
   */
  virtual void initializeWorkVectorsForDS(double t, SP::DynamicalSystem ds) = 0 ;

  /** initialization of the work vectors and matrices (properties) related to
   *  one interaction on the graph and needed by the osi
   *
   *  \param inter the interaction
   *  \param interProp the properties on the graph
   *  \param DSG the dynamical systems graph
   */
  virtual void initializeWorkVectorsForInteraction(Interaction &inter,
                             InteractionProperties& interProp,
                             DynamicalSystemsGraph & DSG) = 0 ;


  /**
     compute interaction output (y) for all levels and swaps in memory

     \param inter the interaction to update
     \param time value for output computation
     \param interaction_properties properties of the interaction, in the Interaction Graph I0
  */
  void update_interaction_output(Interaction& inter, double time, InteractionProperties& interaction_properties);

  /** compute the initial state (for dynamical system variables) of the Newton loop. */
  virtual void computeInitialNewtonState(){
    // Default behavior :  does nothing and used the current state as starting state of the Newton iteration
  }

  /** return the maximum of all norms for the discretized residus of DS
   *
   *  \return a double
   */
  virtual double computeResidu(){
    // default : error
    THROW_EXCEPTION("OneStepIntegrator::computeResidu not implemented for integrator of type " + std::to_string(_integratorType));
    return 0.0;
  }

  /**
      integrates the Dynamical System linked to this integrator, without taking constraints
      into account.
  */
  virtual void computeFreeState()
  {
    // default : error
    THROW_EXCEPTION("OneStepIntegrator::computeFreeState not implemented for integrator of type " + std::to_string(_integratorType));
  }

  /** integrates the Interaction linked to this integrator, without taking non-smooth effects into account
   *
   *  \param vertex_inter of the interaction graph
   *  \param osnsp pointer to OneStepNSProblem
   */
  virtual void computeFreeOutput(InteractionsGraph::VDescriptor& vertex_inter, OneStepNSProblem* osnsp)
  {
    // default : error
    THROW_EXCEPTION("OneStepIntegrator::computeFreeOutput not implemented for integrator of type " + std::to_string(_integratorType));
  }

  /** compute the residu of the output of the relation (y)
   *  This computation depends on the type of OSI
   *
   *  \param time time of computation
   *  \param indexSet the index set of the interaction that are concerned
   */
  virtual double computeResiduOutput(double time, SP::InteractionsGraph indexSet);
  /** compute the residu of the input of the relation (R or p)
   *  This computation depends on the type of OSI
   *
   *  \param time time of computation
   *  \param indexSet the index set of the interaction that are concerned
   */
  virtual double computeResiduInput(double time, SP::InteractionsGraph indexSet);

  /** integrate the system, between tinit and tend, with possible stop at tout
   *
   *  \param tinit start time
   *  \param tend expected end time
   *  \param tout real end time
   *  \param idid extra flag, meaningful only for OSI used in EventDriven schemes
   */
  virtual void integrate(double& tinit, double& tend, double& tout, int& idid) = 0;

  /** set to zero all the r vectors of the DynamicalSystems integrated by this OSI
   */
  void resetAllNonSmoothParts();

  /** set to zero all the r vectors of the DynamicalSystems of the present OSI for a given level
   *
   *  \param level
   */
  void resetNonSmoothPart(unsigned int level);

  /** update the state of the DynamicalSystem attached to this Integrator
   *
   *  \param level level of interest for the dynamics
   *  level is set to 0 by default since in all time-stepping schemes we update all the state
   *  whatever the value of level
   */
  virtual void updateState(const unsigned int level) = 0;

  /** update the state of the DynamicalSystem attached to this Integrator
   *  level is set to 0 by default since in all time-stepping schemes we update all the state
   *  whatever the value of level
   */
  void updateState() { updateState(0); }

  /** update the output of the Interaction attached to this Integrator
   */
  virtual void updateOutput(double time);

  /** update the input of the Interaction attached to this Integrator
   */
  virtual void updateInput(double time);

  /** update the output of the Interaction attached to this Integrator
   *
   *  \param time current time
   *  \param level level of interest for the dynamics
   */
  virtual void updateOutput(double time, unsigned int level);

  /** update the input of the Interaction attached to this Integrator
   *
   *  \param time current time
   *  \param level level of interest for the dynamics
   *  \warning VA: 27/10/2022 Whatever the level, the updateInput method loops over indexSet0
   *  This is sometimes necessary for some OSI but for some others it may burden the computational
   *  time for nothing. For instance, in standard MoreauJEANOSI, p[1] is only defined on indexSet1.
   *  we should go towards
   *  virtual void updateInput(double time, unsigned int pLevel, unsigned int indexSetLevel );
   */
  virtual void updateInput(double time, unsigned int level);

  virtual void prepareNewtonIteration(double time) = 0;

  /** print the data to the screen
   */
  virtual void display() = 0;

  /** Apply the rule to one Interaction to known if is it should be included
   *  in the IndexSet of level i
   *
   *  \param inter
   *  \param i
   *  \return bool
   */
  virtual bool addInteractionInIndexSet(SP::Interaction inter, unsigned int i)
  {
    THROW_EXCEPTION("OneStepIntegrator::addInteractionInIndexSet - Should be called at this level");
    return 0;
  }
  ;

  /** Apply the rule to one Interaction to know if is it should be removed
   *  from the IndexSet of level i
   *
   *  \param inter
   *  \param i
   *  \return bool
   */
  virtual bool removeInteractionFromIndexSet(SP::Interaction inter, unsigned int i)
  {
    THROW_EXCEPTION("OneStepIntegrator::removeInteractionFromIndexSet - Should not be called at this level");
    return 0;
  };

  /** get the ExtraAdditionalTerms.
   *
   *  \return the ExtraAdditionalTerms
   */
  inline SP::ExtraAdditionalTerms extraAdditionalTerms()
  {
    return _extraAdditionalTerms;
  }

  /** set the ExtraAdditionalTerms to add smooth terms for the integration process.
   *  Useful when a control loop is added to a DynamicalSystem.
   *
   *  \param eat the ExtraAdditionalTerms to use
   */
  inline void setExtraAdditionalTerms(SP::ExtraAdditionalTerms eat)
  {
    _extraAdditionalTerms = eat;
  }
  /**
     True if the dynamical system (a vertex in the ds graph) is integrated by this osi.

     \param dsi the iterator on the node of the graph corresponding to the dynamical system of interest.
   */
  inline bool checkOSI(DynamicalSystemsGraph::VIterator dsi)
  {
    return  (_dynamicalSystemsGraph->properties(*dsi).osi.get()) == this;
  };

  /**
     True if the dynamical system (a vertex in the ds graph) is integrated by this osi.

     \param dsgv the descriptor of the node in the graph corresponding to the dynamical system of interest.
   */
  inline bool checkOSI(DynamicalSystemsGraph::VDescriptor dsgv)
  {
    return  (_dynamicalSystemsGraph->properties(dsgv).osi.get()) == this;
  };

  /**
     True if the dynamical system (a vertex in the ds graph) is integrated by this osi.

     \param dsi the iterator on the node of the graph corresponding to the dynamical system of interest.
   */
  inline bool checkInteractionOSI(InteractionsGraph & indexSet0, InteractionsGraph::VIterator ui)
  {
    return  (indexSet0.properties(*ui).osi1.get()) == this;
  };

  virtual SiconosVector& osnsp_rhs(InteractionsGraph::VDescriptor& vertex_inter,  InteractionsGraph& indexSet) = 0;


  VIRTUAL_ACCEPT_VISITORS(OneStepIntegrator);

};

#endif // ONESTEPINTEGRATOR_H
