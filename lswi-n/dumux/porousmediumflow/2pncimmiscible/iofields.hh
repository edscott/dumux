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
 * \brief Adds io fields specific to the twop-nc immiscible model
 */
#ifndef DUMUX_TWOP_NC_IMMISCIBLE_VTK_OUTPUT_FIELDS_HH
#define DUMUX_TWOP_NC_IMMISCIBLE_VTK_OUTPUT_FIELDS_HH

#include <dumux/common/properties.hh>
#include <dumux/porousmediumflow/2p/iofields.hh>

namespace Dumux
{

/*!
 * \ingroup TwoPNCModel
 * \brief Adds io fields specific to the TwoPNC model
 * Replaces fancy TwoPNIIOFields in simpler way.
 */
//#warning "Now adding simple iofields to 2p model"
class TwoPNCImmiscibleIOFields
{
public:

    template <class ModelTraits, class FluidSystem, class SolidSystem = void>
    static std::string primaryVariableName(int pvIdx, int state = 0)
    {
        if (ModelTraits::priVarFormulation() == TwoPFormulation::p0s1)
            return pvIdx == 0 ? IOName::pressure<FluidSystem>(FluidSystem::phase0Idx)
                              : IOName::saturation<FluidSystem>(FluidSystem::phase1Idx);
        else
            return pvIdx == 0 ? IOName::pressure<FluidSystem>(FluidSystem::phase1Idx)
                              : IOName::saturation<FluidSystem>(FluidSystem::phase0Idx);
    }
    template <class OutputModule>
    static void initOutputModule(OutputModule& out)
    {
        using VolumeVariables = typename OutputModule::VolumeVariables;
        using FluidSystem = typename VolumeVariables::FluidSystem;

        // use default fields from the 2p model
        TwoPIOFields::initOutputModule(out);

        //output additional to TwoP output:
        /*for (int phaseIdx = 0; phaseIdx < VolumeVariables::numPhases(); ++phaseIdx){
            for (int j = 0; j < VolumeVariables::numComponents(); ++j){
                DBG("VTK: Phase %d, Adding mole fraction for component %d\n", phaseIdx, j);
                out.addVolumeVariable([phaseIdx,j](
                    const auto& v){ return v.moleFraction(phaseIdx,j); },
                    "x_"+ FluidSystem::phaseName(phaseIdx) + "^" + FluidSystem::componentName(j));
            }
        }*/
        for ( int j = 0; j < VolumeVariables::numComponents()-1; ++j){
            int phaseIdx=FluidSystem::multicomponentPhaseIdx;
            DBG("VTK: Phase %d, Adding mole fraction for component %d\n", phaseIdx, j);
            out.addVolumeVariable([phaseIdx,j](
                const auto& v){ return v.moleFraction(phaseIdx,j); },
                "x_"+ FluidSystem::phaseName(phaseIdx) + "^" + FluidSystem::componentName(j));
            
        }
        for (int j = 0; j < VolumeVariables::numComponents()-1; ++j){
                DBG("VTK: Phase %d, Adding molarity for component %d\n", FluidSystem::multicomponentPhaseIdx, j);
            out.addVolumeVariable([j](
                const auto& v){ return v.molarity(FluidSystem::multicomponentPhaseIdx,j); },
                "m_"+ FluidSystem::phaseName(FluidSystem::multicomponentPhaseIdx) + "^" + FluidSystem::componentName(j));
        }
        for (int phaseIdx = 0; phaseIdx < VolumeVariables::numPhases(); ++phaseIdx){
                DBG("VTK: Phase %d, Adding viscosity\n", phaseIdx);
            out.addVolumeVariable([phaseIdx](
                        const auto& v){ return v.viscosity(phaseIdx); },
                        FluidSystem::phaseName(phaseIdx) + "^viscosity" );
        }
       
        
    }
};

} // end namespace Dumux

#endif
