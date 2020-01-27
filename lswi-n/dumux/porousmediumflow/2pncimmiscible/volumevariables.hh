// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   See the file COPYING for full copying permissions.                      *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \file
 * \ingroup TwoPNCModel
 * \brief Contains the quantities which are constant within a
 *        finite volume in the two-phase, n-component model.
 */
#ifndef DUMUX_2PNC_IMMISCIBLE_VOLUME_VARIABLES_HH
#define DUMUX_2PNC_IMMISCIBLE_VOLUME_VARIABLES_HH

#include <iostream>
#include <vector>

#include <dumux/common/math.hh>
#include <dumux/common/properties.hh>
//#include <dumux/discretization/methods.hh>

#include <dumux/material/fluidstates/compositional.hh>
#include <dumux/material/solidstates/updatesolidvolumefractions.hh>
#include <dumux/porousmediumflow/volumevariables.hh>
#include <dumux/porousmediumflow/nonisothermal/volumevariables.hh>

#include <dumux/porousmediumflow/2p/formulation.hh>

namespace Dumux {

/*!
 * \ingroup TwoPNCModel
 * \brief Contains the quantities which are are constant within a
 *        finite volume in the two-phase immiscible n-component model.
 */
template <class Traits>
class TwoPNCImmiscibleVolumeVariables
: public PorousMediumFlowVolumeVariables<Traits>
, public EnergyVolumeVariables<Traits, TwoPNCImmiscibleVolumeVariables<Traits> >
{
    using ThisType = TwoPNCImmiscibleVolumeVariables<Traits>;
    using ParentType = PorousMediumFlowVolumeVariables<Traits>;
    using EnergyVolVars = EnergyVolumeVariables<Traits, ThisType>;
    using Scalar = typename Traits::PrimaryVariables::value_type;
    using PermeabilityType = typename Traits::PermeabilityType;
    using FS = typename Traits::FluidSystem;
    using ModelTraits = typename Traits::ModelTraits;

    static constexpr int numFluidComps = ModelTraits::numComponents();

    enum
    {
        numMajorComponents = ModelTraits::numPhases(),
        nComp = ModelTraits::numComponents(),

        // phase indices
        phase0Idx = FS::phase0Idx,
        phase1Idx = FS::phase1Idx,

        // phase name/indices
        multicomponentPhaseIdx = FS::multicomponentPhaseIdx,
        singlecomponentPhaseIdx = FS::singlecomponentPhaseIdx,

        // component indices;
        comp0Idx = FS::comp0Idx,
        lastCompIdx = FS::lastCompIdx, // ! Set up lastCompIdx within fluidsystem

        // primary variable indices;
        pressureIdx = ModelTraits::Indices::pressureIdx,
        saturationIdx = ModelTraits::Indices::saturationIdx
    };

    static constexpr auto formulation = ModelTraits::priVarFormulation();
    static constexpr bool setFirstPhaseMoleFractions = ModelTraits::setMoleFractionsForFirstPhase();

public:
    //! export fluid state type
    using FluidState = typename Traits::FluidState;
    //! export fluid system type
    using FluidSystem = typename Traits::FluidSystem;
    //! export type of solid state
    using SolidState = typename Traits::SolidState;
    //! export type of solid system
    using SolidSystem = typename Traits::SolidSystem;

    //! return whether moles or masses are balanced
    static constexpr bool useMoles() { return Traits::ModelTraits::useMoles(); }
    //! return the two-phase formulation used here
    static constexpr TwoPFormulation priVarFormulation() { return formulation; }

    // check for permissive specifications
    static_assert(useMoles(), "use moles has to be set true in the 2pnc model");
    static_assert(ModelTraits::numPhases() == 2, "NumPhases set in the model is not two!");
    static_assert((formulation == TwoPFormulation::p0s1 || formulation == TwoPFormulation::p1s0), "Chosen TwoPFormulation not supported!");

    /*!
     * \brief Update all quantities for a given control volume
     *
     * \param elemSol A vector containing all primary variables connected to the element
     * \param problem The object specifying the problem which ought to
     *                be simulated
     * \param element An element which contains part of the control volume
     * \param scv The sub control volume
    */
    template<class ElemSol, class Problem, class Element, class Scv>
    void update(const ElemSol& elemSol,
                const Problem& problem,
                const Element& element,
                const Scv& scv)
    {
        ParentType::update(elemSol, problem, element, scv);
        completeFluidState(elemSol, problem, element, scv, fluidState_, solidState_);

        /////////////
        // calculate the remaining quantities
        /////////////
        // Second instance of a parameter cache.
        // Could be avoided if diffusion coefficients also
        // became part of the fluid state.
        typename FluidSystem::ParameterCache paramCache;
        paramCache.updateAll(fluidState_);

        using MaterialLaw = typename Problem::SpatialParams::MaterialLaw;
        const auto& matParams = problem.spatialParams().materialLawParams(element, problem, scv, elemSol);

        const int wPhaseIdx = problem.spatialParams().template wettingPhase<FluidSystem>(element, scv, elemSol);
        const int nPhaseIdx = 1 - wPhaseIdx;

        // mobilities -> require wetting phase saturation as parameter!
        mobility_[wPhaseIdx] = MaterialLaw::krw(matParams, saturation(wPhaseIdx))/fluidState_.viscosity(wPhaseIdx);
        mobility_[nPhaseIdx] = MaterialLaw::krn(matParams, saturation(wPhaseIdx))/fluidState_.viscosity(nPhaseIdx);

        // binary diffusion coefficients
        diffCoefficient_.fill(0.0);
        //std::fill(diffCoefficient_.begin(), diffCoefficient_.end(), 0.0);
        for (unsigned int compJIdx = 0; compJIdx < ModelTraits::numComponents(); ++compJIdx)
        {
            if(compJIdx != comp0Idx && compJIdx != lastCompIdx)
                setDiffusionCoefficient_( phase0Idx, compJIdx,
                                          FluidSystem::binaryDiffusionCoefficient(fluidState_,
                                                                                  paramCache,
                                                                                  phase0Idx,
                                                                                  comp0Idx,
                                                                                  compJIdx) );
        }

        // porosity and permeability
        updateSolidVolumeFractions(elemSol, problem, element, scv, solidState_, numFluidComps);
        EnergyVolVars::updateSolidEnergyParams(elemSol, problem, element, scv, solidState_);
        permeability_ = problem.spatialParams().permeability(element, scv, elemSol);
    }

    /*!
     * \brief Complete the fluid state
     *
     * \param elemSol A vector containing all primary variables connected to the element
     * \param problem The problem
     * \param element The element
     * \param scv The sub control volume
     * \param fluidState The fluid state
     *
     * Set temperature, saturations, capillary pressures, viscosities, densities and enthalpies.
     */
    template<class ElemSol, class Problem, class Element, class Scv>
    void completeFluidState(const ElemSol& elemSol,
                            const Problem& problem,
                            const Element& element,
                            const Scv& scv,
                            FluidState& fluidState,
                            SolidState& solidState)
    {
        EnergyVolVars::updateTemperature(elemSol, problem, element, scv, fluidState, solidState);
        const auto& priVars = elemSol[scv.localDofIndex()];

        using MaterialLaw = typename Problem::SpatialParams::MaterialLaw;
        const auto& materialParams = problem.spatialParams().materialLawParams(element, problem, scv, elemSol);
        const int wPhaseIdx = problem.spatialParams().template wettingPhase<FluidSystem>(element, scv, elemSol);
        fluidState.setWettingPhase(wPhaseIdx);

        if (formulation == TwoPFormulation::p0s1)
        {
            fluidState.setPressure(phase0Idx, priVars[pressureIdx]);
            if (wPhaseIdx == phase1Idx)
            {
                fluidState.setSaturation(phase1Idx, priVars[saturationIdx]);
                fluidState.setSaturation(phase0Idx, 1 - priVars[saturationIdx]);
                pc_ = MaterialLaw::pc(materialParams, fluidState.saturation(wPhaseIdx));
                fluidState.setPressure(phase1Idx, priVars[pressureIdx] - pc_);
            }
            else
            {
                const auto Sn = Traits::SaturationReconstruction::reconstructSn(problem.spatialParams(), element,
                                                                                scv, elemSol, priVars[saturationIdx]);
                fluidState.setSaturation(phase1Idx, Sn);
                fluidState.setSaturation(phase0Idx, 1 - Sn);
                pc_ = MaterialLaw::pc(materialParams, fluidState.saturation(wPhaseIdx));
                fluidState.setPressure(phase1Idx, priVars[pressureIdx] + pc_);
            }
        }
        else if (formulation == TwoPFormulation::p1s0)
        {
            fluidState.setPressure(phase1Idx, priVars[pressureIdx]);
            if (wPhaseIdx == phase1Idx)
            {
                const auto Sn = Traits::SaturationReconstruction::reconstructSn(problem.spatialParams(), element,
                                                                                scv, elemSol, priVars[saturationIdx]);
                fluidState.setSaturation(phase0Idx, Sn);
                fluidState.setSaturation(phase1Idx, 1 - Sn);
                pc_ = MaterialLaw::pc(materialParams, fluidState.saturation(wPhaseIdx));
                fluidState.setPressure(phase0Idx, priVars[pressureIdx] + pc_);
            }
            else
            {
                fluidState.setSaturation(phase0Idx, priVars[saturationIdx]);
                fluidState.setSaturation(phase1Idx, 1.0 - priVars[saturationIdx]);
                pc_ = MaterialLaw::pc(materialParams, fluidState.saturation(wPhaseIdx));
                fluidState.setPressure(phase0Idx, priVars[pressureIdx] - pc_);
            }
        }

        // calculate the phase compositions
        typename FluidSystem::ParameterCache paramCache;

        // set mole fractions for all components expect h2o and oil

        Scalar sumMoleFrac = 0.0;
        for (int compIdx = 1; compIdx < ModelTraits::numComponents()-1; ++compIdx)
        {
            fluidState.setMoleFraction(multicomponentPhaseIdx, compIdx, priVars[compIdx]);
            sumMoleFrac += priVars[compIdx];
        }
        fluidState.setMoleFraction(multicomponentPhaseIdx, comp0Idx, 1.0 - sumMoleFrac);
        fluidState.setMoleFraction(multicomponentPhaseIdx, lastCompIdx, 0.0);
        // XXX Not clean:
        // The above will have set averageMolarMass_ in fluidstates/compositional.hh
        // This is used below for fluidState.setMolarDensity(), now necessary for
        // 2pnc model.
        // Might need to clean up this act.

        for (int compIdx = 0; compIdx < -1; ++compIdx)
            fluidState.setMoleFraction(singlecomponentPhaseIdx, compIdx, 0.0);
        fluidState.setMoleFraction(singlecomponentPhaseIdx, lastCompIdx, 1.0);

        paramCache.updateAll(fluidState);
        for (int phaseIdx = 0; phaseIdx < ModelTraits::numPhases(); ++phaseIdx)
        {
            Scalar rho = FluidSystem::density(fluidState, paramCache, phaseIdx);
            Scalar mu = FluidSystem::viscosity(fluidState, paramCache, phaseIdx);
            Scalar h = EnergyVolVars::enthalpy(fluidState, paramCache, phaseIdx);

            fluidState.setDensity(phaseIdx, rho);
            fluidState.setViscosity(phaseIdx, mu);
            fluidState.setEnthalpy(phaseIdx, h);
            fluidState.setMolarDensity(phaseIdx, fluidState.averageMolarMass(phaseIdx));      
        }
    }

    /*!
     * \brief Returns the phase state for the control-volume.
     */
    const FluidState &fluidState() const
    { return fluidState_; }

    /*!
     * \brief Returns the phase state for the control volume.
     */
    const SolidState &solidState() const
    { return solidState_; }

    /*!
     * \brief Returns the saturation of a given phase within
     *        the control volume in \f$[-]\f$.
     *
     * \param phaseIdx The phase index
     */
    Scalar saturation(int phaseIdx) const
    { return fluidState_.saturation(phaseIdx); }

    /*!
     * \brief Returns the mass density of a given phase within the
     *        control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar density(int phaseIdx) const
    { return fluidState_.density(phaseIdx); }

    /*!
     * \brief Returns the kinematic viscosity of a given phase within the
     *        control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar viscosity(int phaseIdx) const
    { return fluidState_.viscosity(phaseIdx); }

    /*!
     * \brief Returns the mass density of a given phase within the
     *        control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar molarDensity(int phaseIdx) const
    { return fluidState_.molarDensity(phaseIdx); }

    /*!
     * \brief Returns the effective pressure of a given phase within
     *        the control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar pressure(int phaseIdx) const
    { return fluidState_.pressure(phaseIdx); }

    /*!
     * \brief Returns temperature inside the sub-control volume.
     *
     * Note that we assume thermodynamic equilibrium, i.e. the
     * temperature of the rock matrix and of all fluid phases are
     * identical.
     */
    Scalar temperature() const
    { return fluidState_.temperature(/*phaseIdx=*/0); }

    /*!
     * \brief Returns the effective mobility of a given phase within
     *        the control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar mobility(int phaseIdx) const
    { return mobility_[phaseIdx]; }

    /*!
     * \brief Returns the effective capillary pressure within the control volume
     *        in \f$[kg/(m*s^2)=N/m^2=Pa]\f$.
     */
    Scalar capillaryPressure() const
    { return pc_; }

    /*!
     * \brief Returns the average porosity within the control volume.
     */
    Scalar porosity() const
    { return solidState_.porosity(); }

    /*!
     * \brief Returns the permeability within the control volume.
     */
    const PermeabilityType& permeability() const
    { return permeability_; }


    /*!
     * \brief Returns the diffusion coefficient
     */
    Scalar diffusionCoefficient(int phaseIdx, int compIdx) const
    {
      if (phaseIdx==multicomponentPhaseIdx)
      {
        if (compIdx < phaseIdx)
            return diffCoefficient_[compIdx];
        else if (compIdx > phaseIdx)
            return diffCoefficient_[compIdx-1];
      }
      else if (phaseIdx==singlecomponentPhaseIdx)
        return 0.0;

      DUNE_THROW(Dune::InvalidStateException, "Diffusion coefficient called for phaseIdx = compIdx");
    }

    /*!
     * \brief Returns the molarity of a component in the phase
     *
     * \param phaseIdx the index of the fluid phase
     * \param compIdx the index of the component
     */
     Scalar molarity(int phaseIdx, int compIdx) const // [moles/m^3]
    { return fluidState_.molarity(phaseIdx, compIdx);}

     /*!
      * \brief Returns the mass fraction of a component in the phase
      *
      * \param phaseIdx the index of the fluid phase
      * \param compIdx the index of the component
      */
     Scalar massFraction(int phaseIdx, int compIdx) const
     { return fluidState_.massFraction(phaseIdx, compIdx); }

     /*!
      * \brief Returns the mole fraction of a component in the phase
      *
      * \param phaseIdx the index of the fluid phase
      * \param compIdx the index of the component
      */
     Scalar moleFraction(int phaseIdx, int compIdx) const
     { return fluidState_.moleFraction(phaseIdx, compIdx); }

     static const int numPhases(void) {return numMajorComponents;}
     static const int numComponents(void) {return nComp;}

protected:
    FluidState fluidState_;
    SolidState solidState_;

private:
    void setDiffusionCoefficient_(int phaseIdx, int compIdx, Scalar d)
    {
        if (compIdx < phaseIdx)
            diffCoefficient_[compIdx] = std::move(d);
        else if (compIdx > phaseIdx)
            diffCoefficient_[compIdx-1] = std::move(d);
        else
            DUNE_THROW(Dune::InvalidStateException, "Diffusion coefficient for phaseIdx = compIdx doesn't exist");

    }

    Scalar pc_;                     //!< The capillary pressure
    Scalar porosity_;               //!< Effective porosity within the control volume
    PermeabilityType permeability_; //!> Effective permeability within the control volume
    Scalar mobility_[ModelTraits::numPhases()]; //!< Effective mobility within the control volume
    std::array<Scalar, ModelTraits::numComponents()-1> diffCoefficient_;
};

} // end namespace Dumux

#endif
