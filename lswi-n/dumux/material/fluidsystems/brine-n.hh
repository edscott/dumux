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
 *
 *  Description:
 *
 *  Brine fluidsystem considering Na+ as a single disassociated particle.
 *  One liquid phase, two components: H2O and Na+.
 *  Cl- will be solved for each element by electroneutrality.
 */

#ifndef DUMUX_BRINE_FLUID_SYSTEM_HH
#define DUMUX_BRINE_FLUID_SYSTEM_HH

#include <dumux/material/fluidsystems/base.hh>

// Components
#include <dumux/material/components/simpleh2o.hh>

#include "dumux/material/components/anions.hh"
#include "dumux/material/components/cations.hh"

#include <dumux/common/valgrind.hh>
#include <dumux/common/exceptions.hh>

#ifndef BRINE_N_COMPONENTS
#warning "assuming single particle in brine (BRINE_N_COMPONENTS = 2)"
// components = water + particle count
# define BRINE_N_COMPONENTS 2
#endif

namespace Dumux
{

namespace FluidSystems
{

// Global static variable to have a different constant density
// for different episodes.
// A value greater than zero indicates a constant density for the
// episode.
static double episodeDensity_=-1.0;
static double episodeViscosity_=-1.0;
static particle_t *FSparticles;



/*!
 * \ingroup Fluidsystems
 * \brief A compositional single phase fluid system consisting of two components,
 *        which are H2O and Na+.
 */
template <class Scalar, class H2O = Dumux::Components::SimpleH2O<Scalar>>
class Brine : 
    public Base< Scalar, Brine<Scalar, H2O>> // to pass on static functions of Base...
{
    using ThisType = Brine<Scalar, H2O>;
    using StaticBase = Base<Scalar, ThisType>; // for static stuff with implementation

    // convenience using declarations
    using NaCation = Dumux::Components::NaCation<Scalar>;

public:
    /****************************************
     * Fluid phase related static parameters
     ****************************************/
     //! Number of phases in the fluid system
    static const int numPhases = 1;

    static constexpr int phase0Idx = 0;

    // export component indices to indicate the main component
    // of the corresponding phase at atmospheric pressure 1 bar
    // and room temperature 20C:

    // components = water + particle count
    static const int numComponents = BRINE_N_COMPONENTS; // H2O, NaCation

    static constexpr int H2OIdx = 0; // 0

    /*!
     * \brief Return the human readable name of a phase (used in indices)
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static const std::string phaseName(int phaseIdx = H2OIdx)
    {
        return "Brine-Na+";
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
     *        be compressible.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static bool isCompressible(int phaseIdx = H2OIdx)
    {
        return false;
    }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be an ideal gas.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */

    static bool isIdealGas(int phaseIdx = H2OIdx)
    {
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
        if (compIdx > BRINE_N_COMPONENTS - 1 or compIdx < 0){
            DUNE_THROW(Dune::InvalidStateException, "Component index out of range" << compIdx << " maximum = " << BRINE_N_COMPONENTS-1);
        }
        switch (compIdx)
        {
            case H2OIdx: // zero
                return H2O::name();
            default: // input fortran index, references C index
                return FSparticles[compIdx - 1].idx;
        };
    }

    /*!
     * \brief Return the molar mass of a component in \f$\mathrm{[kg/mol]}\f$.
     *
     * \param compIdx The index of the component to consider
     */
    static Scalar molarMass(int compIdx)
    {
        if (compIdx > BRINE_N_COMPONENTS - 1 or compIdx < 0){
            DUNE_THROW(Dune::InvalidStateException, "Component index out of range" << compIdx << " maximum = " << BRINE_N_COMPONENTS-1);
        }
        switch (compIdx) {
        case H2OIdx: // zero
            return H2O::molarMass();
        default: // input fortran index, references C index
            return FSparticles[compIdx - 1].molecularWeight * 1e-03;
        };
    }

    /****************************************
     * thermodynamic relations
     ****************************************/
    /*!
     * \brief Initialize the fluid system's static parameters. 
     *
     */

    static void init(Scalar initialDensity, Scalar initialViscosity, particle_t *particle_p)
    {
        episodeDensity_ = initialDensity;
        episodeViscosity_ = initialViscosity;
        FSparticles = particle_p;
    }

    /*!
     * \brief Given all mole fractions in a phase, return the phase
     *        density. \f$\mathrm{[kg/m^3]}\f$.
     *        Note: This fluidsystem will return  a specific constant
     *        density if an episode density is not specified.
     */
    template <class FluidState>
    static Scalar density(const FluidState& fluidState, int phaseIdx = H2OIdx)
    {
        if (episodeDensity_ > 0) return episodeDensity_;
        return 1130.0;
    }

    /*!
     * \brief Given all mole fractions in a phase, return the phase
     *        viscosity. \f$\mathrm{[kg/m^3]}\f$.
     *        Note: This fluidsystem will return  a specific constant
     *        viscosity if an episode viscosity is not specified.
     */
    using StaticBase::viscosity;
    template <class FluidState>
    static Scalar viscosity(const FluidState& fluidState, int phaseIdx = H2OIdx)
    {
        if (episodeViscosity_ > 0) return episodeViscosity_;
        return 3.0e-04;
    }


    /*!
     * \brief Return the diffusion coefficient of a component within a phase.
     *        This is currently not used for 2p immiscible.
     */
    template <class FluidState>
    static Scalar diffusionCoefficient(const FluidState& fluidState, int phaseIdx, int compIdx)
    {
        return 0.0;
        DUNE_THROW(Dune::InvalidStateException, "Not applicable Diffusion coefficients");
    }


    /*!
     * \brief Return the binary diffusion coefficient of a component within a phase.
     *        This is currently not used for 2p immiscible.
     */
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
