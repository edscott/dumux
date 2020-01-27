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
 *  \file
  */

#ifndef DUMUX_2PNC_IMMISCIBLE_MODEL_HH
#define DUMUX_2PNC_IMMISCIBLE_MODEL_HH

#include <dune/common/fvector.hh>
#include <dumux/common/properties.hh>

#include <dumux/porousmediumflow/compositional/primaryvariableswitch.hh>
#include <dumux/porousmediumflow/2pnc/model.hh>
#include <dumux/porousmediumflow/2p/saturationreconstruction.hh>

#include "volumevariables.hh"
#include "iofields.hh"
#include "indices.hh"
namespace Dumux {

template<int nComp, bool useMol, TwoPFormulation formulation, int repCompEqIdx = nComp>
struct TwoPNCImmiscibleModelTraits
{
    using Indices = TwoPNCImmiscibleIndices<nComp>;

    static constexpr int numEq() { return nComp; }
    static constexpr int numPhases() { return 2; }
    static constexpr int numComponents() { return nComp; }
    // git a9895363f
    static constexpr int replaceCompEqIdx() { return repCompEqIdx; }

    static constexpr int numFluidPhases() { return 2; }
    static constexpr int numFluidComponents() { return nComp; }
    
    static constexpr bool enableAdvection() { return true; }
    static constexpr bool enableMolecularDiffusion() { return true; }
    static constexpr bool enableEnergyBalance() { return false; }

    static constexpr bool useMoles() { return useMol; }

    static constexpr TwoPFormulation priVarFormulation() { return formulation; }
};

/*!
 * \ingroup TwoPNCImmiscibleModel
 * \brief Traits class for the volume variables of the single-phase model.
 *
 * \tparam PV The type used for primary variables
 * \tparam FSY The fluid system type
 * \tparam FST The fluid state type
 * \tparam PT The type used for permeabilities
 * \tparam MT The model traits
 * \tparam SR Saturation reconstruction type
 */
template< class PV, class FSY, class FST,
          class SSY, class SST, class PT, class MT, class SR >
struct TwoPNCImmiscibleVolumeVariablesTraits
{
    using PrimaryVariables = PV;
    using FluidSystem = FSY;
    using FluidState = FST;
    using SolidSystem = SSY;
    using SolidState = SST;
    using PermeabilityType = PT;
    using ModelTraits = MT;
    using SaturationReconstruction = SR;
};

namespace Properties {
//////////////////////////////////////////////////////////////////
// Type tags
//////////////////////////////////////////////////////////////////
// Create new type tags
namespace TTag {
struct TwoPNCImmiscible { using InheritsFrom = std::tuple<TwoPNC>; };
} // end namespace TTag

//////////////////////////////////////////////////////////////////
// Properties for the isothermal 2pnc model
//////////////////////////////////////////////////////////////////
//! The primary variables vector for the 2pnc model
template<class TypeTag>
struct PrimaryVariables<TypeTag, TTag::TwoPNCImmiscible>
{
public:
    using type = Dune::FieldVector<GetPropType<TypeTag, Properties::Scalar>,
                                   GetPropType<TypeTag, Properties::ModelTraits>::numEq()>;
};

/* no switchable variable, always 2 phases...*/
/*
template<class TypeTag>
struct PrimaryVariableSwitch<TypeTag, TTag::TwoPNCImmiscible> { using type = NoPrimaryVariableSwitch; };  //!< The primary variable switch for the 2pnc model
*/

template<class TypeTag>
struct IOFields<TypeTag, TTag::TwoPNCImmiscible> { using type = TwoPNCImmiscibleIOFields; };

//! Set the volume variables property
template<class TypeTag>
struct VolumeVariables<TypeTag, TTag::TwoPNCImmiscible>
{
private:
    using PV = GetPropType<TypeTag, Properties::PrimaryVariables>;
    using FSY = GetPropType<TypeTag, Properties::FluidSystem>;
    using FST = GetPropType<TypeTag, Properties::FluidState>;
    using SSY = GetPropType<TypeTag, Properties::SolidSystem>;
    using SST = GetPropType<TypeTag, Properties::SolidState>;
    using MT = GetPropType<TypeTag, Properties::ModelTraits>;
    using PT = typename GetPropType<TypeTag, Properties::SpatialParams>::PermeabilityType;

    static constexpr auto DM = GetPropType<TypeTag, Properties::FVGridGeometry>::discMethod;
    static constexpr bool enableIS = getPropValue<TypeTag, Properties::EnableBoxInterfaceSolver>();
    // class used for scv-wise reconstruction of non-wetting phase saturations
    using SR = TwoPScvSaturationReconstruction<DM, enableIS>;

    using Traits = TwoPNCImmiscibleVolumeVariablesTraits<PV, FSY, FST, SSY, SST, PT, MT, SR>; // DUDA ¿?
public:
    using type = TwoPNCImmiscibleVolumeVariables<Traits>;
};

//! Set the model traits
template<class TypeTag>
struct ModelTraits<TypeTag, TTag::TwoPNCImmiscible>
{
private:
    //! we use the number of components specified by the fluid system here
    using FluidSystem = GetPropType<TypeTag, Properties::FluidSystem>;
    static_assert(FluidSystem::numPhases == 2, "Only fluid systems with 2 fluid phases are supported by the 2p-nc immiscible model!");
public:
    using type = TwoPNCImmiscibleModelTraits<FluidSystem::numComponents,
                                   getPropValue<TypeTag, Properties::UseMoles>(),
                                   getPropValue<TypeTag, Properties::Formulation>()>;
};

} // end namespace Properties
} // end namespace Dumux

#endif
