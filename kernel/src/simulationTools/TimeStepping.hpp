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
/*! \file TimeStepping.hpp
 *  \brief Time-Stepping simulation
 */
#ifndef TimeStepping_H
#define TimeStepping_H

#include "Simulation.hpp"

/** type of function used to post-treat output info from solver. */
typedef void (*CheckSolverFPtr)(int, Simulation *);

/** \brief Event-capturing Time-Stepping simulation
 *
 * This class implements the basic algorithm for Event-capturing Time-Stepping
 * simulations.
 *
 * References :
 *
 * V. Acary and B. Brogliato. Numerical Methods for Nonsmooth Dynamical Systems:
 * Applications in Mechanics and Electronics, volume 35 of Lecture Notes in
 * Applied and Computational Mechanics. Springer Verlag, 2008.
 *
 */

#define SICONOS_TS_LINEAR 1
#define SICONOS_TS_LINEAR_IMPLICIT 2
#define SICONOS_TS_NONLINEAR 3

class TimeStepping : public Simulation {
protected:
  ACCEPT_SERIALIZATION(TimeStepping);

  /** Default Newton tolerance used in call of run() of ComputeOneStep() */
  double _newtonTolerance;
  
  /** Default maximum number of Newton iteration*/
  unsigned int _newtonMaxIteration;

  /** Number of steps performed in the Newton Loop */
  unsigned int _newtonNbIterations;

  /** Cumulative number of steps performed in the Newton Loops */
  unsigned int _newtonCumulativeNbIterations;

  /** unsigned int  _newtonOptions
   *  option in the Newon iteration
   *  SICONOS_TS_LINEAR or SICONOS_TS_LINEAR_IMPLICIT SICONOS_TS_NONLINEAR will
   *  force a single iteration of the Newton Solver SICONOS_TS_NONLINEAR
   *  (default) will perform the newton iteration up to convergence
   */
  unsigned int _newtonOptions;

  /** Maximum Residual for the Dynamical system */
  double _newtonResiduDSMax;

  /** Maximum Residual for the output of the relation */
  double _newtonResiduYMax;

  /** Maximum Residual for the input of the relation */
  double _newtonResiduRMax;

  /** boolean variable to know whether the ResiduY has to be computed or not
   *  if true, the ResiduY is computed and the convergence is checked
   */
  bool _computeResiduY;

  /** boolean variable to know whether the ResiduR has to be computed or not
   *  if true, the ResiduR is computed and the convergence is checked
   */
  bool _computeResiduR;

  /** boolean variable to know whether Newton iterations converge or not
   */
  bool _isNewtonConverge;

  /** boolean variable indicating whether interactions should be
   *  updated within the Newton loop.
   */
  bool _newtonUpdateInteractionsPerIteration;

  /** boolean variable to display Newton info
   */
  bool _displayNewtonConvergence;

  /** boolean variable to display warning on non-convergence
   */
  bool _newtonWarningOnNonConvergence;

  /** boolean variable to display warning if osnspb is not correcltys olved
   */
  bool _warningNonsmoothSolver;

  /** boolean variable to resetAllLamda at each step (default true)
   */
  bool _resetAllLambda;

  /** boolean variable to skip  updateOutput at the end of the step (default
   *  false)
   */
  bool _skip_last_updateOutput;

  /** boolean variable to skip  updateInput at the end of the step (default
   *  false) useful for Global integrators that do not need to compute input in
   *  the linear case
   */
  bool _skip_last_updateInput;

  /** boolean variable to skip  resetLambdas (default false)
   */
  bool _skip_resetLambdas;

  /** Default Constructor
   */
  TimeStepping()
      : _computeResiduY(false), _computeResiduR(false),
        _isNewtonConverge(false){};

  /** newton algorithm
   *
   *  \param criterion convergence criterion
   *  \param maxStep maximum number of Newton steps
   */
  virtual void newtonSolve(double criterion, unsigned int maxStep);

public:
  /** initialisation specific to TimeStepping for OneStepNSProblem.
   */
  void initOSNS() override;

  /** Standard constructor
   *
   *  \param nsds NonSmoothDynamicalSystem to be simulated
   *  \param td pointer to a timeDiscretisation used in the integration
   *  \param osi one step integrator (default none)
   *  \param osnspb one step non smooth problem (default none)
   */
  TimeStepping(SP::NonSmoothDynamicalSystem nsds, SP::TimeDiscretisation td,
               SP::OneStepIntegrator osi, SP::OneStepNSProblem osnspb);

  /** Constructor with the time-discretisation.
   *
   *  \param nsds NonSmoothDynamicalSystem to be simulated
   *  \param td pointer to a timeDiscretisation used in the integration
   *  \param nb number of non smooth problem
   */
  TimeStepping(SP::NonSmoothDynamicalSystem nsds, SP::TimeDiscretisation td,
               int nb = 0);

  /** insert an Integrator into the simulation list of integrators
   *
   *  \param osi the OneStepIntegrator to add
   */
  void insertIntegrator(SP::OneStepIntegrator osi) override;

  /** Destructor.
   */
  virtual ~TimeStepping() noexcept = default;

  /** update indexSets[i] of the topology, using current y and lambda values of Interactions
   *
   *  \param i the number of the set to be updated
   */
  void updateIndexSet(unsigned int i) override;

  // /** Used by the updateIndexSet function in order to deactivate
  // SP::Interaction.
  //  */
  // virtual bool predictorDeactivate(SP::Interaction inter, unsigned int i);

  // /** Used by the updateIndexSet function in order to activate
  // SP::Interaction.
  //  */
  // virtual bool predictorActivate(SP::Interaction inter, unsigned int i);

  /** increment model current time according to User TimeDiscretisation and call
   *  SaveInMemory. */
  virtual void nextStep();

  /** integrates all the DynamicalSystems taking not into account nslaw,
   *  reactions (ie non-smooth part) ...
   */
  void computeFreeState();

  /** Reset all lambdas of all interactions */
  void resetLambdas();

  /** step from current event to next event of EventsManager
   */
  void advanceToEvent() override;

  /** run one time--step of the simulation
   */
  void computeOneStep();

  /** To known the number of steps performed by the Newton algorithm.
   *
   *  \return  the number of steps performed by the Newton algorithm
   */
  unsigned int getNewtonNbIterations() { return _newtonNbIterations; }

  /** To known the number of steps performed by the Newton algorithm.
   *
   *  \return  the cumulative number of steps performed by the Newton algorithm
   */
  unsigned int getNewtonCumulativeNbIterations()
  {
    return _newtonCumulativeNbIterations;
  }

  /** initialize the Newton
   *  It computes the initial residu and set the, if needed to Newton variable
   *  to start the newton algorithm.
   */
  void initializeNewtonLoop();

  void computeInitialNewtonState();
  void prepareNewtonIteration();

  /** check the convergence of Newton algorithm according to criterion
   *
   *  \param criterion convergence criterion
   *  \return bool = true if Newton method has converged
   */
  bool newtonCheckConvergence(double criterion);

  /** run the simulation, from t0 to T
   *  with default parameters if any setting has been done
   */
  void run() override;

  /** check returning value from computeOneStepNSProblem and process
   *
   *  \param info solver-specific error code return by the nonsmooth solver
   */
  void DefaultCheckSolverOutput(int info);

  /** Set CheckSolverOutput function
   *
   *  \param newF pointer to function steering the behavior of simulation when
   *  nonsmooth solver failed
   */
  void setCheckSolverFunction(CheckSolverFPtr newF);

  bool isNewtonConverge() { return _isNewtonConverge; };

  bool displayNewtonConvergence() { return _displayNewtonConvergence; };
  void setDisplayNewtonConvergence(bool newval)
  {
    _displayNewtonConvergence = newval;
  };

  void setNewtonWarningOnNonConvergence(bool newval) { _newtonWarningOnNonConvergence = newval; };
  bool newtonWarningOnNonConvergence() { return _newtonWarningOnNonConvergence; };

  void setWarningNonsmoothSolver(bool newval) {  _warningNonsmoothSolver=newval;};
  bool warningNonsmoothSolver() {  return _warningNonsmoothSolver;};
  


  
  void displayNewtonConvergenceAtTheEnd(int info, unsigned int maxStep);
  void displayNewtonConvergenceInTheLoop();

  void setResetAllLambda(bool newval) { _resetAllLambda = newval; };

  void setSkipLastUpdateOutput(bool newval)
  {
    _skip_last_updateOutput = newval;
  };
  bool skipLastUpdateOutput() { return _skip_last_updateOutput; };
  void setSkipLastUpdateInput(bool newval) { _skip_last_updateInput = newval; };
  bool skipLastUpdateInput() { return _skip_last_updateInput; };
  void setSkipResetLambdas(bool newval) { _skip_resetLambdas = newval; };
  bool skipResetLambdas() { return _skip_resetLambdas; };

  /** To specify if the output interaction residu must be computed.
   *
   *  \param v set to true when the output interaction residu must be computed
   */
  void setComputeResiduY(bool v) { _computeResiduY = v; };

  /** To know if the output interaction residu must be computed.
   *
   *  \return bool _computeResiduY
   */
  bool computeResiduY() override { return _computeResiduY; };

  /** To specify if the input interaction residu must be computed.
   *
   *  \param v set to true when the input interaction residu must be computed
   */
  void setComputeResiduR(bool v) { _computeResiduR = v; };

  /** To known if the input interaction residu must be computed.
   *
   *  \return bool _computeResiduR
   */
  bool computeResiduR() override { return _computeResiduR; };

  /** set the Default Newton tolerance
   *
   *  \param tol Newton solver tolerance
   */
  void setNewtonTolerance(double tol) { _newtonTolerance = tol; };

  /** get the Newton tolerance
   *
   *  \return default Newton solver tolerance
   */
  double newtonTolerance() { return _newtonTolerance; };

  /** set the maximum number of Newton iteration
   *
   *  \param maxStep maximum number of Newton solver iterations
   */
  void setNewtonMaxIteration(unsigned int maxStep)
  {
    _newtonMaxIteration = maxStep;
  };

  /** get the maximum number of Newton iteration
   *
   *  \return maximum number of Newton solver iterations
   */
  unsigned int newtonMaxIteration() { return _newtonMaxIteration; };

  /** set the NewtonOptions
   *
   *  \param v Newton solver options
   */
  void setNewtonOptions(unsigned int v) { _newtonOptions = v; };

  /** get the NewtonOptions
   *
   *  \return Newton solver options - SICONOS_TS_LINEAR 1,
   *  SICONOS_TS_LINEAR_IMPLICIT 2, SICONOS_TS_NONLINEAR 3
   */
  unsigned int newtonOptions() { return _newtonOptions; };

  /** accessor to _newtonResiduDSMax
   *
   *  \return _newtonResiduDSMax
   */
  double newtonResiduDSMax() { return _newtonResiduDSMax; };

  /** accessor to _newtonResiduYMax
   *
   *  \return _newtonResiduYMax
   */
  double newtonResiduYMax() { return _newtonResiduYMax; };

  /** accessor to _newtonResiduRMax
   *
   *  \return _newtonResiduRMax
   */
  double newtonResiduRMax() { return _newtonResiduRMax; };

  ACCEPT_STD_VISITORS();
};

#endif // TimeStepping_H
