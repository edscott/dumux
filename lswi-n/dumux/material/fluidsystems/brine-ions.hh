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
*  File(s):
 *              chemical/ions.hh
 *              components/na+.hh
 *              components/ca++.hh
 *              components/h+.hh
 *              components/mg++.hh
 *              components/hco3-.hh
 *              components/so4--.hh
 *              components/cl-.hh
 *              components/cacl+.hh
 *              components/caso4.hh
 *              components/mgcl+.hh
 *              components/naso4-.hh
 *              components/mgso4.hh
 *              components/oh-.hh
 *              components/co3--.hh
 *              components/ionBrine.hh
 *
 *  Description:
 *
 *     This template defines a fluid system with chemical reactions.
 *     Number of components involved in the chemical reactions may vary
 *     from zero to eight (transportIons) transported ions. The number
 *     of non transported components may reach up to 35 (_allComponents).
 *
 *     Dynamic change of chemical treatment may be done by definition of
 *     the following preprocessor directives:
 *
 *     SIMPLE_CACO3
 *     CACO3_CASO4
 *     CACO3_CASO4_MGCO3
 *     BRINE_OIL_CHEMISTRY
 *     BRINE_SOLID_CHEMISTRY
 *     BRINE_OIL_SOLID_CHEMISTRY (required BRINE_OIL_CHEMISTRY and BRINE_SOLID_CHEMISTRY
 *     USE_ACTIVITY_COEFICIENTS:
 *       If defined, activities are calculated from ionic strength functions. Otherwise,
 *       all equilibrium reactions are considered ideal, i.e., activity coeficients are
 *       equal to 1.
 *     ALL_IONS_FOR_IS:
 *       If defined, positive and negative ion concentrations are used for ionic strength
 *       calculation. Otherwise, only positive ions are used for calculation of ionic strength
 *
 * Reactions involved (equilibrium and reaction rate constants indexed by letter):
 * A: CaCO3(s) + H+ <--> Ca++ + HCO3-
 * B: CaSO4(s)  <--> Ca++ + SO4--
 * C: MgCO3(s) + H+ <--> Mg++ + HCO3-
 * D: CaSO4 <--> Ca++ + SO4-- (not precipitated)
 * E: H20 <--> H+ + OH-
 * F: HCO3- <--> H+ + CO3--
 * G: CO2 + H2O <--> H+ + HCO3-
 * H: CaCl- <--> Ca++ + Cl-
 * I: MgCl+ <--> Mg++ + Cl-
 * J: NaSO4- <--> Na+ + SO4--
 * K: MgSO4 <--> Mg++ + SO4--
 * L: RCOOH <--> RCOO- + H+
 * M: RCOOCa+ <--> RCOO- + Ca++
 * N: RCOOMg+ <--> RCOO- + Mg++
 * O: >CaOH + H+ <--> >CaOH2+
 * P: >CaOH2+ + SO4-- <--> >CaSO4- + H2O
 * Q: >CaOH2+ + CO3-- <--> >CaCO3- + H2O
 * R: >CO3H <--> >CO3- + H+
 * S: >CO3Ca+ <--> >CO3- + Ca++
 * T: >CO3Mg+ <--> >CO3- + Mg++
 * U: (>CaOH2+)(RCOO-) <--> >CaOH2+ + RCOO-  (id: _vCaOH2RCOO)
 * V: >CO3Ca(RCOO-) <--> >CO3- + Ca++ + RCOO- (id: _vCO3CaRCOO)
 * W: >CO3Mg(RCOO-) <--> >CO3- + Mg++ + RCOO- (id: _vCO3MgRCOO)
 * X: None
 *
 */

#ifndef DUMUX_BRINE_FLUID_SYSTEM_HH
#define DUMUX_BRINE_FLUID_SYSTEM_HH

#include <dumux/material/fluidsystems/base.hh>

// Components
#include <dumux/material/components/simpleh2o.hh>

// Transported and non transported ions
// Transported components: Na+, Ca++, H+, Mg++, HCO3-, SO4--, Cl-
// Non transported components: CaCl+, CaSO4-(suspended), MgCl+, NaSO4-
#include "material/components/anions.hh"
#include "material/components/cations.hh"
#include "material/components/inert.hh"

//#include <dumux/material/components/nacl.hh>

#include <dumux/common/valgrind.hh>
#include <dumux/common/exceptions.hh>

namespace Dumux
{
namespace FluidSystems
{

/*!
 * \ingroup Fluidsystems
 * \brief A compositional single phase fluid system consisting of two components,
 *        which are H2O and NaCl.
 */
//template <class Scalar, class H2O = Components::TabulatedComponent<Dumux::Components::H2O<Scalar>> >
template <class Scalar, class H2O = Dumux::Components::SimpleH2O<Scalar>>
class Brine : 
    public Base< Scalar, Brine<Scalar, H2O>> // to pass on static functions of Base...
{
    using ThisType = Brine<Scalar, H2O>;
    using StaticBase = Base<Scalar, ThisType>; // for static stuff with implementation
//    using Base = BaseFluidSystem<Scalar, ThisType>;

    // convenience using declarations
    using NaCl = Dumux::Components::NaCation<Scalar>;

public:
    /****************************************
     * Fluid phase related static parameters
     ****************************************/

     //! Number of phases in the fluid system
    static const int numPhases = 1;

    static constexpr int phase0Idx = 0;

    // export component indices to indicate the main component
    // of the corresponding phase at atmospheric pressure 1 bar
    // and room temperature 20°C:

    static constexpr int comp0Idx = 0;
    static constexpr int comp1Idx = 1;

    static const int numComponents = 2; // H2O, NaCl

    static constexpr int H2OIdx = comp0Idx; // 0
    static constexpr int NaClIdx = comp1Idx; // 1

    /*!
     * \brief Return the human readable name of a phase (used in indices)
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static const std::string phaseName(int phaseIdx = H2OIdx)
    {
        return H2O::name();
    }

    /*!
     * \brief Return whether a phase is liquid
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static constexpr bool isLiquid(int phaseIdx = H2OIdx)
    {
        return true;
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
/*    static bool isIdealMixture(int phaseIdx)
    {
        assert(0 <= phaseIdx && phaseIdx < numPhases);
        return true;
    }*/

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be compressible.
     *
     * Compressible means that the partial derivative of the density
     * to the fluid pressure is always larger than zero.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static bool isCompressible(int phaseIdx = H2OIdx)
    {
        return H2O::liquidIsCompressible();
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
      return false;
    }

    /****************************************
     * Component related static parameters
     ****************************************/

    /*!
     * \brief Return the human readable name of a componentEnthalpy
     *
     * \param compIdx The index of the component to consider
     */
    static std::string componentName(int compIdx)
    {
        switch (compIdx)
        {
            case H2OIdx: return H2O::name();
            case NaClIdx: return NaCl::name();
        };
        DUNE_THROW(Dune::InvalidStateException, "Invalid component index " << compIdx);
    }

    /*!
     * \brief Return the molar mass of a component in \f$\mathrm{[kg/mol]}\f$.
     *
     * \param compIdx The index of the component to consider
     */
    static Scalar molarMass(int compIdx)
    {
        switch (compIdx) {
        case H2OIdx: return H2O::molarMass();
        case NaClIdx:return NaCl::molarMass();
        };
        DUNE_THROW(Dune::InvalidStateException, "Invalid component index " << compIdx);
    }

    /****************************************
     * thermodynamic relations
     ****************************************/
    /*!
     * \brief Initialize the fluid system's static parameters generically
     *
     * If a tabulated H2O component is used, we do our best to create
     * tables that always work.
     */
     static void init()
     {
         init(/*tempMin=*/273.15,
              /*tempMax=*/623.15,
              /*numTemp=*/100,
              /*pMin=*/-10.,
              /*pMax=*/20e6,
              /*numP=*/200);
     }

    /*!
     * \brief Initialize the fluid system's static parameters using
     *        problem specific temperature and pressure ranges
     *
     * \param tempMin The minimum temperature used for tabulation of water [K]
     * \param tempMax The maximum temperature used for tabulation of water [K]
     * \param nTemp The number of ticks on the temperature axis of the  table of water
     * \param pressMin The minimum pressure used for tabulation of water [Pa]
     * \param pressMax The maximum pressure used for tabulation of water [Pa]
     * \param nPress The number of ticks on the pressure axis of the  table of water
     */
    static void init(Scalar tempMin, Scalar tempMax, unsigned nTemp,
                     Scalar pressMin, Scalar pressMax, unsigned nPress)
    {

        if (H2O::isTabulated) {
            std::cout << "Initializing tables for the H2O fluid properties ("
                      << nTemp*nPress
                      << " entries).\n";

            H2O::init(tempMin, tempMax, nTemp,
                      pressMin, pressMax, nPress);
        }
    }

    /*!
     * \brief Given all mole fractions in a phase, return the phase
     *        density [kg/m^3].
     */
    using StaticBase::density;
    template <class FluidState>
    static Scalar density(const FluidState& fluidState, int phaseIdx = H2OIdx)
    {
        if (episodeDensity > 0) return episodeDensity;
        return 1130.0;
    }

    /*!
     * \brief Return the viscosity of a phase.
     */
    using StaticBase::viscosity;
    template <class FluidState>
    static Scalar viscosity(const FluidState& fluidState, int phaseIdx = H2OIdx)
    {
        if (episodeViscosity > 0) return episodeViscosity;
        return 3.0e-04;
    }


    template <class FluidState>
    static Scalar diffusionCoefficient(const FluidState& fluidState, int phaseIdx, int compIdx)
    {
        DUNE_THROW(Dune::InvalidStateException, "Not applicable Diffusion coefficients");
    }


    //using StaticBase::binaryDiffusionCoefficient;
    template <class FluidState>
    static Scalar binaryDiffusionCoefficient(const FluidState& fluidState,
                                             int phaseIdx,
                                             int compIIdx,
                                             int compJIdx)
    {
        return 0.0;
    }

private:

};
} // end namespace FluidSystems
} // end namespace Dumux

#endif
