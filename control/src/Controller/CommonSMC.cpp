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

#include "ModelingTools.hpp"
#include "SiconosAlgebraProd.hpp"
#include "SiconosAlgebraScal.hpp"
#include "SimulationTools.hpp"
#include "CommonSMC.hpp"
#include "ControlSensor.hpp"
#include "FirstOrderR_helpers.hpp"
#include "InvertMatrix.hpp" // for invertMatrix ...
#include "SolverOptions.h"
#include <string>

// #define DEBUG_NOCOLOR
// #define DEBUG_STDOUT
// #define DEBUG_MESSAGES
#include "siconos_debug.h"

void CommonSMC::initialize(const NonSmoothDynamicalSystem & nsds, const Simulation & s)
{
  DEBUG_BEGIN("CommonSMC::initialize(const NonSmoothDynamicalSystem & nsds, const Simulation & s)\n");
  if(!_Csurface && _pluginhName.empty())
  {
    THROW_EXCEPTION("CommonSMC::initialize - you have to set either _Csurface or h(.) before initializing the Actuator");
  }
  else
  {
    if(_Csurface && !_u)
      _u.reset(new SiconosVector(_Csurface->size(0), 0));

    Actuator::initialize(nsds,s);
  }
  // We can only work with FirstOrderNonLinearDS, FirstOrderLinearDS and FirstOrderLinearTIDS
  // We can use the Visitor mighty power to check if we have the right type
  SP::DynamicalSystem DS = _sensor->getDS();
  Type::Siconos dsType;
  dsType = Type::value(*DS);

  // create the DS for the controller
  // if the DS we use is different from the DS we are controlling
  // when we want for instant to see how well the controller behaves
  // if the plant model is not exact, we can use the setSimulatedDS
  // method
  if(dsType == Type::FirstOrderNonLinearDS)
  {
    _DS_SMC.reset(new FirstOrderNonLinearDS(*(std::static_pointer_cast<FirstOrderNonLinearDS>(DS))));
  }
  else if(dsType == Type::FirstOrderLinearDS)
  {
    _DS_SMC.reset(new FirstOrderLinearDS(*(std::static_pointer_cast<FirstOrderLinearDS>(DS))));
    std::static_pointer_cast<FirstOrderLinearDS>(_DS_SMC)->setComputebFunction(nullptr);
    // We have to reset the _pluginb
    SP::SiconosVector dummyb(new SiconosVector(_DS_SMC->n(), 0));
    std::static_pointer_cast<FirstOrderLinearDS>(_DS_SMC)->setbPtr(dummyb);
  }
  else if(dsType == Type::FirstOrderLinearTIDS)
  {
    _DS_SMC.reset(new FirstOrderLinearTIDS(*(std::static_pointer_cast<FirstOrderLinearTIDS>(DS))));
    // We have to reset the _pluginb
    SP::SiconosVector dummyb(new SiconosVector(_DS_SMC->n(), 0));
    std::static_pointer_cast<FirstOrderLinearDS>(_DS_SMC)->setbPtr(dummyb);
  }
  else
  {
    THROW_EXCEPTION("LinearSMC is not yet implemented for system of type" + std::to_string(dsType));
  }
  _DS_SMC->setNumber(999999);
  _DS_SMC->initMemory(1);
  _DS_SMC->swapInMemory();


  // Get the dimension of the output
  // XXX What if there is more than one sensor ...
  double t0 = nsds.t0();
  double T = nsds.finalT() + _td->currentTimeStep(0);
  // create the SMC Model
  _nsdsSMC.reset(new NonSmoothDynamicalSystem(t0, T));
  // Set up the simulation
  _simulationSMC.reset(new TimeStepping(_nsdsSMC,_td));

  unsigned int sDim = _u->size();
  // create the interaction
  if(!_plugingName.empty())
  {
    if(_pluginhName.empty())
      THROW_EXCEPTION("LinearSMC::initialize - the Controller has a function g set but _pluginhName is not set\n You must supply a function to compute y=h(x,...)");
    if(!_pluginJacgxName.empty())  // Is the relation the most generic NL one ?
    {
      DEBUG_PRINT("A FirstOrderNonLinearR is created for the _relationSMC\n");
      _relationSMC.reset(new FirstOrderNonLinearR());
      _relationSMC->setComputeJacgxFunction(SSLH::getPluginName(_pluginJacgxName), SSLH::getPluginFunctionName(_pluginJacgxName));

      FirstOrderRHelpers::JacglambdaSetter(*_relationSMC, _B, _pluginJacglambdaName);
      FirstOrderRHelpers::JachxSetter(*_relationSMC, _Csurface, _pluginJachxName);
      if(_pluginJachlambdaName.empty() && !_D)
        _D.reset(new SimpleMatrix(sDim, sDim, 0));
      FirstOrderRHelpers::JachlambdaSetter(*_relationSMC, _D, _pluginJachlambdaName);

    }
    else if(!_pluginJachlambdaName.empty() || _D)  // Type2R ?
    {
      DEBUG_PRINT("A FirstOrderType2R is created for the _relationSMC\n");
      _relationSMC.reset(new FirstOrderType2R());
      FirstOrderRHelpers::JacglambdaSetter(*_relationSMC, _B, _pluginJacglambdaName);
      FirstOrderRHelpers::JachxSetter(*_relationSMC, _Csurface, _pluginJachxName);
      FirstOrderRHelpers::JachlambdaSetter(*_relationSMC, _D, _pluginJachlambdaName);
    }
    else // Type1R
    {
      DEBUG_PRINT("A FirstOrderType1R is created for the _relationSMC\n");
      _relationSMC.reset(new FirstOrderType1R());
      FirstOrderRHelpers::JacglambdaSetter(*_relationSMC, _B, _pluginJacglambdaName);
      FirstOrderRHelpers::JachxSetter(*_relationSMC, _Csurface, _pluginJachxName);
    }

    _relationSMC->setComputehFunction(SSLH::getPluginName(_pluginhName), SSLH::getPluginFunctionName(_pluginhName));
    _relationSMC->setComputegFunction(SSLH::getPluginName(_plugingName), SSLH::getPluginFunctionName(_plugingName));

    if(_computeResidus)
    {
      _simulationSMC->setComputeResiduY(true);
      _simulationSMC->setComputeResiduR(true);
      _simulationSMC->setUseRelativeConvergenceCriteron(false);
    }
  }
  else
  {
    if(!_plugineName.empty())
    {
      DEBUG_PRINT("A FirstOrderLinearR is created for the _relationSMC\n");
      _relationSMC.reset(new FirstOrderLinearR(_Csurface, _B));
      _relationSMC->setComputeEFunction(SSLH::getPluginName(_plugineName), SSLH::getPluginFunctionName(_plugineName));
    }
    else
    {
      DEBUG_PRINT("A FirstOrderLinearTIR is created for the _relationSMC\n");
      _relationSMC.reset(new FirstOrderLinearTIR(_Csurface, _B));
    }
    std::static_pointer_cast<FirstOrderLinearTIR>(_relationSMC)->setDPtr(_D);
  }

  // _nsLawSMC and the OSNSP can be defined in derived classes, like twisting
  if(!_nsLawSMC)  _nsLawSMC.reset(new RelayNSL(sDim, -_alpha, _alpha));
  if(!_OSNSPB_SMC) _OSNSPB_SMC.reset(new Relay(_numericsSolverId));

  _interactionSMC.reset(new Interaction(_nsLawSMC, _relationSMC));

  if(dsType == Type::FirstOrderNonLinearDS)
  {
    _integratorSMC.reset(new EulerMoreauOSI(_thetaSMC));
  }
  else
  {
    _integratorSMC.reset(new ZeroOrderHoldOSI());
  }

  _nsdsSMC->insertDynamicalSystem(_DS_SMC);
  _nsdsSMC->setName(_DS_SMC, "plant_SMC");
  _nsdsSMC->link(_interactionSMC, _DS_SMC);
  _nsdsSMC->setControlProperty(_interactionSMC, true);
  _nsdsSMC->topology()->setName(_interactionSMC, "Sgn_SMC");

  _simulationSMC->setName("linear sliding mode controller simulation");
  _simulationSMC->insertIntegrator(_integratorSMC);
  _simulationSMC->associate(_integratorSMC, _DS_SMC);

  // OneStepNsProblem
  _OSNSPB_SMC->numericsSolverOptions()->dparam[SICONOS_DPARAM_TOL] = _precision;
  _simulationSMC->insertNonSmoothProblem(_OSNSPB_SMC);
  // Finally we can initialize everything ...
  _simulationSMC->associate(_integratorSMC,_DS_SMC);

  // _SMC->setSimulation(_simulationSMC);
  _simulationSMC->initialize();

  // Handy
  _eventsManager = _simulationSMC->eventsManager();
  _lambda.reset(new SiconosVector(sDim));
  _lambda = _interactionSMC->lambda(0);
  _us.reset(new SiconosVector(sDim));
  _ueq.reset(new SiconosVector(sDim));

  if(_Csurface)
  {
    SP::SimpleMatrix tmpM(new SimpleMatrix(_Csurface->size(0), _B->size(1)));
    _invCB.reset(new SimpleMatrix(*tmpM));
    prod(*_Csurface, *_B, *tmpM);
    InvertMatrix(*tmpM->dense(), *_invCB->dense());
  }
  DEBUG_END("CommonSMC::initialize(const NonSmoothDynamicalSystem & nsds, const Simulation & s)\n");
}

void CommonSMC::computeUeq()
{
  DEBUG_BEGIN("void CommonSMC::computeUeq()\n");
  assert(Type::value(*_DS_SMC) != Type::FirstOrderNonLinearDS && "CommonSMC::computeUeq the DS should be linear");
  assert(_Csurface && "CommonSMC::computeUeq the sliding variable should be linear subpsace of the state");
  FirstOrderLinearDS& LinearDS_SMC = *std::static_pointer_cast<FirstOrderLinearDS>(_DS_SMC);
  unsigned int n = LinearDS_SMC.A()->size(1);
  // equivalent part, explicit contribution
  SP::SimpleMatrix tmpM1(new SimpleMatrix(_Csurface->size(0), n));
  SP::SimpleMatrix tmpN(new SimpleMatrix(n, n));
  SP::SimpleMatrix quasiProjB_A(new SimpleMatrix(_invCB->size(0), n));
  SP::SimpleMatrix tmpW(new SimpleMatrix(n, n, 0));
  SP::SiconosVector xTk(new SiconosVector(_sensor->y()));
  tmpW->eye();
  prod(*_Csurface, *LinearDS_SMC.A(), *tmpM1);
  // compute (CB)^{-1}CA
  prod(*_invCB, *tmpM1, *quasiProjB_A);
  prod(_thetaSMC-1, *quasiProjB_A, *xTk, *_ueq);

  // equivalent part, implicit contribution
  // XXX when to call this ?
  ZeroOrderHoldOSI& zoh = *std::static_pointer_cast<ZeroOrderHoldOSI>(_integratorSMC);
  zoh.updateMatrices(_DS_SMC);

  // tmpN = B^{*}(CB)^{-1}CA
  prod(zoh.Bd(_DS_SMC), *quasiProjB_A, *tmpN, true);
  // W = I + \theta B^{*})CB)^{-1}CA
  scal(_thetaSMC, *tmpN, *tmpW, false);
  // compute e^{Ah}x_k
  prod(zoh.Ad(_DS_SMC), *xTk, *xTk);
  // xTk = (e^{Ah}-(1-\theta)\Psi_k\Pi_B A)x_k
  prod(_thetaSMC-1, *tmpN, _sensor->y(), *xTk, false);
  // compute the solution x_{k+1} of the system W*x_{k+1} = x_k
  tmpW->Solve(*xTk);
  // add the contribution from the implicit part to ueq
  prod(-_thetaSMC, *quasiProjB_A, *xTk, *_ueq, false);
  DEBUG_END("void CommonSMC::computeUeq()\n");

}

void CommonSMC::setCsurface(SP::SimpleMatrix newC)
{
  // check dimensions ...
  _Csurface = newC;
}

void CommonSMC::setSaturationMatrix(SP::SimpleMatrix newSat)
{
  // check dimensions ...
  if(newSat->size(1) != _B->size(1))
  {
    THROW_EXCEPTION("CommonSMC::setSaturationMatrixPtr - inconstency between the dimension of the state space and D");
  }
  else
  {
    _D = newSat;
  }
}


void CommonSMC::setTimeDiscretisation(const TimeDiscretisation& td)
{
  _td.reset(new TimeDiscretisation(td));
}

void CommonSMC::sete(const std::string& plugin)
{
  _plugineName = plugin;
}

void CommonSMC::seth(const std::string& plugin)
{
  _pluginhName = plugin;
}
void CommonSMC::setJachx(const std::string& plugin)
{
  _pluginJachxName = plugin;
}
void CommonSMC::setJachlambda(const std::string& plugin)
{
  _pluginJachlambdaName = plugin;
}
void CommonSMC::setg(const std::string& plugin)
{
  _plugingName = plugin;
}
void CommonSMC::setJacgx(const std::string& plugin)
{
  _pluginJacgxName = plugin;
}
void CommonSMC::setJacglambda(const std::string& plugin)
{
  _pluginJacglambdaName = plugin;
}
