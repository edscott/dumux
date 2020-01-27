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
 * \ingroup FluidSystems
 * \brief A two-phase fluid system assuming immiscibility and n components.
 */
#ifndef DUMUX_2PNC_IMMISCIBLE_FLUID_SYSTEM_HH
#define DUMUX_2PNC_IMMISCIBLE_FLUID_SYSTEM_HH

#include <dumux/material/fluidsystems/base.hh>

#include <dumux/common/valgrind.hh>
#include <dumux/common/exceptions.hh>
    
namespace Dumux
{
namespace FluidSystems
{

/*!
 * \ingroup Fluidsystems
 * \brief A two-phase fluid system assuming immiscibility and n components
 *
 */
template <class Scalar, class MultiComponentPhase, class SingleComponentPhase>
class TwoPNCImmiscible : public Base< Scalar, TwoPNCImmiscible<Scalar, MultiComponentPhase, SingleComponentPhase> >
{
    static_assert((MultiComponentPhase::numPhases == 1), "multiComponentPhase has more than one phase");
    static_assert((SingleComponentPhase::numPhases == 1), "singleComponentPhase has more than one phase");
    //static_assert((SingleComponentPhase::numComponents == 1), "singleComponentPhase has more than one component");
    // two gaseous phases at once do not make sense physically! (but two liquids are fine)
    // And yet, OnePLiquid is obviously liquid... isLiquid() is no longer in OnePLiquid
    // duh
    // static_assert(MultiComponentPhase::isLiquid() || SingleComponentPhase::isLiquid(), "One phase has to be a liquid!");

    using ThisType = TwoPNCImmiscible<Scalar, MultiComponentPhase, SingleComponentPhase>;
    using StaticBase = Base<Scalar, ThisType>;
    

public:

    static const int numPhases = 2; //! Number of phases in the fluid system
    static constexpr int numComponents = MultiComponentPhase::numComponents + 1;

    static constexpr int phase0Idx = 0; //!< index of the first phase
    static constexpr int phase1Idx = 1; //!< index of the second phase

    static constexpr int multicomponentPhaseIdx = 0; // Just for easy reading
    static constexpr int singlecomponentPhaseIdx = 1; // Just for easy reading

    // export component indices to indicate the main component
    // of the corresponding phase at atmospheric pressure 1 bar
    // and room temperature 20°C:

    static constexpr int comp0Idx = 0;
    static constexpr int lastCompIdx = MultiComponentPhase::numComponents;

    /****************************************
     * Fluid phase related static parameters
     ****************************************/

    /*!
     * \brief Returns whether the fluids are miscible
     */
    static constexpr bool isMiscible()
    {
        return false;
    }

    /*!
     * \brief Return whether a phase is liquid
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static bool isLiquid(int phaseIdx)
    {
        assert(0 <= phaseIdx && phaseIdx < numPhases);

        if (phaseIdx == singlecomponentPhaseIdx)
            return SingleComponentPhase::isLiquid();;
        return MultiComponentPhase::isLiquid();
    }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be an ideal mixture.
     *
     * We define an ideal mixture as a fluid phase where the fugacity
     * coefficients of all components times the pressure of the phase
     * are indepent on the fluid composition. This assumtion is true
     * if Henry's law and Raoult's law apply. If you are unsure what
     * this function should return, it is safe to return false. The
     * only damage done will be (slightly) increased computation times
     * in some cases.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static bool isIdealMixture(int phaseIdx)
    {
        assert(0 <= phaseIdx && phaseIdx < numPhases);

        return true;
    }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be compressible.
     *
     * Compressible means that the partial derivative of the density
     * to the fluid pressure is always larger than zero.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static bool isCompressible(int phaseIdx)
    {
      assert(0 <= phaseIdx && phaseIdx < numPhases);

      // let the fluids decide
      if (phaseIdx == singlecomponentPhaseIdx)
          return SingleComponentPhase::liquidIsCompressible();
      return MultiComponentPhase::isCompressible();
    }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be an ideal gas.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */

    static bool isIdealGas(int phaseIdx)
    {
      assert(0 <= phaseIdx && phaseIdx < numPhases);

      // let the fluids decide
      if (phaseIdx == singlecomponentPhaseIdx)
          return SingleComponentPhase::gasIsIdeal();
      return MultiComponentPhase::isIdealGas();
    }

    /****************************************
     * Component related static parameters
     ****************************************/
    /*!
     * \brief Return the human readable name of a phase (used in indices)
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static const std::string phaseName(int phaseIdx)
    {
        assert(0 <= phaseIdx && phaseIdx < numPhases);

        if (phaseIdx == singlecomponentPhaseIdx) return SingleComponentPhase::phaseName(phaseIdx);
        return MultiComponentPhase::phaseName();
    }
    /*!
     * \brief Return the human readable name of a component
     *
     * \param compIdx index of the component
     */
    static std::string componentName(int compIdx)
    {
        assert(0 <= compIdx && compIdx < numComponents);

        if (compIdx == lastCompIdx)
            return SingleComponentPhase::name();
        return MultiComponentPhase::componentName(compIdx);
    }

    /*!
     * \brief Return the molar mass of a component in \f$\mathrm{[kg/mol]}\f$.
     *
     * \param compIdx The index of the component to consider
     */
    static Scalar molarMass(int compIdx)
    {
        assert(0 <= compIdx && compIdx < numComponents);

        if (compIdx == lastCompIdx)
            return SingleComponentPhase::molarMass();
        return MultiComponentPhase::molarMass(compIdx);
    }

    /****************************************
     * thermodynamic relations
     ****************************************/

    /*!
     * \brief Given all mole fractions in a phase, return the phase
     *        density [kg/m^3].
     */
    using StaticBase::density;
    template <class FluidState>
    static Scalar density(const FluidState& fluidState, int phaseIdx)
    {
        assert(0 <= phaseIdx && phaseIdx < numPhases);

        if (phaseIdx == multicomponentPhaseIdx){
            return MultiComponentPhase::density(fluidState);
        } else {
            return SingleComponentPhase::density(fluidState, phaseIdx);
        }
    }

    /*!
     * \brief Return the viscosity of a phase.
     */
    using StaticBase::viscosity;
    template <class FluidState>
    static Scalar viscosity(const FluidState& fluidState, int phaseIdx)
    {
        assert(0 <= phaseIdx  && phaseIdx < numPhases);
        if (phaseIdx == multicomponentPhaseIdx){
            return MultiComponentPhase::viscosity(fluidState);
        } else{
            return SingleComponentPhase::viscosity(fluidState, phaseIdx);
        }
    }



    /*!
     * \brief Given a phase's composition, temperature and pressure,
     *        return the binary diffusion coefficient \f$\mathrm{[m^2/s]}\f$ for components
     *        \f$\mathrm{i}\f$ and \f$\mathrm{j}\f$ in this phase.
     * \param fluidState The fluid state
     * \param paramCache mutable parameters
     * \param phaseIdx Index of the fluid phase
     * \param compIIdx Index of the component i
     * \param compJIdx Index of the component j
     */
    using StaticBase::binaryDiffusionCoefficient;
    template <class FluidState>
    static Scalar binaryDiffusionCoefficient(const FluidState& fluidState,
                                             int phaseIdx,
                                             int compIIdx,
                                             int compJIdx)
    {
        assert(0 <= phaseIdx  && phaseIdx < numPhases);
        assert(0 <= compIIdx  && compIIdx < numComponents);
        assert(0 <= compJIdx  && compJIdx < numComponents);

        if (phaseIdx == multicomponentPhaseIdx)
        {
            return MultiComponentPhase::binaryDiffusionCoefficient(fluidState,
                                                                     phaseIdx,
                                                                     compIIdx,
                                                                     compJIdx);
         }
         DUNE_THROW(Dune::NotImplemented, "Binary diffusion coefficient of components "
                                          << compIIdx << " and " << compJIdx
                                          << " in phase " << phaseIdx);
    }

private:

};
} // end namespace FluidSystems
} // end namespace Dumux

#endif
