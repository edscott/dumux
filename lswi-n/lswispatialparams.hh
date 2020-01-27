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
 * \ingroup TwoPNCTests
 * \brief Definition of the spatial parameters for the low-salinity waterflooding
 *        problem which uses the immiscible non-insothermal 2pnc box model
 */

#ifndef LSWI_SPATIAL_PARAMS_HH
#define LSWI_SPATIAL_PARAMS_HH

#include <dumux/porousmediumflow/properties.hh>
#include <dumux/material/spatialparams/fv.hh>

#include <dumux/material/fluidmatrixinteractions/2p/linearmaterial.hh>

// function interpolation
#include "dumux/material/fluidmatrixinteractions/2p/functioninterpolation/regularizedmodifiedbrookscoreyfi.hh"
#include "dumux/material/fluidmatrixinteractions/2p/functioninterpolation/efftoabslawmodifiedbrookscoreyfi.hh"


namespace Dumux
{

// The problem class is forward declared:
template<class TypeTag> 
class LSWF2pncSpatialParams;


template<class TypeTag>
class LSWF2pncSpatialParams : 
    protected LswiData<TypeTag>,
    public FVSpatialParams< GetPropType<TypeTag, Properties::GridGeometry>,
                            GetPropType<TypeTag, Properties::Scalar>,
                            LSWF2pncSpatialParams<TypeTag> >
/*
    public FVSpatialParams< typename GET_PROP_TYPE(TypeTag, FVGridGeometry),
                            typename GET_PROP_TYPE(TypeTag, Scalar),
                            LSWF2pncSpatialParams<TypeTag> >*/
{
    using Scalar = GetPropType<TypeTag, Properties::Scalar>;
//    using Scalar = typename GET_PROP_TYPE(TypeTag, Scalar);
  
    using Problem = GetPropType<TypeTag, Properties::Problem>;
//    using Problem = typename GET_PROP_TYPE(TypeTag, Problem);
  
    using GridView = GetPropType<TypeTag, Properties::GridView>;
//    using GridView = typename GET_PROP_TYPE(TypeTag, GridView);
  
    using FluidSystem = GetPropType<TypeTag, Properties::FluidSystem>;
//    using FluidSystem = typename GET_PROP_TYPE(TypeTag, FluidSystem);

    using GridGeometry = GetPropType<TypeTag, Properties::GridGeometry>;
    using SubControlVolume = typename GridGeometry::SubControlVolume;

    using Element = typename GridView::template Codim<0>::Entity;
    using GlobalPosition = typename Element::Geometry::GlobalCoordinate;

    using ThisType = LSWF2pncSpatialParams<TypeTag>;
    using ParentType = FVSpatialParams<GridGeometry, Scalar, ThisType>;
    using EffectiveLaw = RegularizedModifiedBrooksCoreyFI<Scalar>;

    static constexpr int dimWorld = GridView::dimensionworld;
    mutable int episode_;
    mutable int step_;
public:
    void setEpisode(int value){
        DBG("****setEpisode***** spatial params set episode to %d (%s)\n", value, this->episodeName(value).c_str());
        episode_ = value;
    }
    void setStep(int value){
        DBG("**setStep** spatial params set step to %d\n", value);
        step_ = value;
    }
    void dump(void) const {
        DBG("MatrixSwr:      Hi = %le  Low = %le \n", 
                materialParams_.swr_HS(),materialParams_.swr_LS()); 
        DBG("MatrixSnr:      Hi = %le  Low = %le \n",
                materialParams_.snr_HS(),materialParams_.snr_LS()); 
        materialParams_.dump();
    }

    using PermeabilityType = Scalar;
    using MaterialLaw = EffToAbsLaw<EffectiveLaw>;
    using MaterialLawParams = typename MaterialLaw::Params;

    /*!
     * \brief The constructor
     *
     * \param gridView The grid view
     */
    LSWF2pncSpatialParams(std::shared_ptr<const GridGeometry> fvGridGeometry)
    : ParentType(fvGridGeometry)
    {
        episode_ = 0;
        int i=0;
        // Initial high salinity values.
        materialParams_.setPe_HS(this->getValue("MatrixPe"));
        materialParams_.setLambda_HS(this->getValue("MatrixLambda"));
        materialParams_.setK0rw_HS(this->getValue("MatrixKrwMax"));
        materialParams_.setK0rn_HS(this->getValue("MatrixKrnMax"));
        materialParams_.setNw_HS(this->getValue("Matrixnw"));
        materialParams_.setNn_HS(this->getValue("Matrixnn"));
        materialParams_.setSwr_HS(this->getValue("MatrixSwr"));
        materialParams_.setSnr_HS(this->getValue("MatrixSnr"));


        //materialParams_.setHS(this->getValue("xNa"));
        materialParams_.setHS(this->xParticleInitialTotal());

        // Initial low salinity values 
        materialParams_.setPe_LS(this->getValue("MatrixPe"));
        materialParams_.setLambda_LS(this->getValue("MatrixLambda"));
        materialParams_.setK0rw_LS(this->getValue("MatrixKrwMax"));
        materialParams_.setK0rn_LS(this->getValue("MatrixKrnMax"));
        materialParams_.setNw_LS(this->getValue("Matrixnw"));
        materialParams_.setNn_LS(this->getValue("Matrixnn"));
        materialParams_.setSwr_LS(this->getValue("MatrixSwr"));
        materialParams_.setSnr_LS(this->getValue("MatrixSnr"));
        //materialParams_.setLS(this->getValue("xNa"));
        materialParams_.setLS(this->xParticleInitialTotal());
            
        // Set the initial salinity to initial conditions.
        //materialParams_.setS(this->getValue("xNa"));
        materialParams_.setS(this->xParticleInitialTotal());
    }

    /*!
    * \brief Define the permeability \f$[m^2]\f$ of the spatial parameters
     *
     * \param globalPos The global position
     */
     Scalar permeabilityAtPos(const GlobalPosition& globalPos) const
     {
         return this->get(episode_, "MatrixPermeability");
     }

    /*!
     * \brief Define the porosity \f$[-]\f$ of the spatial parameters
     *
     * \param globalPos The global position
     */
    Scalar porosityAtPos(const GlobalPosition& globalPos) const
    {
         // Episode defined value not available...
         // Problem considers constant porosity (no geomechanics)
        static Scalar initialPhi = this->getValue("MatrixPorosity");
        return initialPhi;
    }

    /*!
     * \brief return the parameter object for the Van Genuchten material law which depends on the position
     *
     * \param globalPos The global position
     */

    int stage(int k){
        return this->stageNumber(k);
    }

    template<class ElementSolution, class Problem>
    const MaterialLawParams& materialLawParams(const Element& element,
                                           const Problem& problem,
                                           const SubControlVolume& scv,
                                           const ElementSolution& elemSol) const
   {
        static int episode = -1;

        // First of all, if episode has not changed, then there is no
        // need to reset any parameter (with BCM only).
        bool episodeSwitch = (this->episode_ != episode);
        if (this->useBCM_ and not episodeSwitch) return materialParams_;

        episode = this->episode_;
        if (episodeSwitch) DBG("***---   SpatialParams at episode %d, stage=%d\n", 
                    episode, this->stageNumber(episode));
                
        int i = episode;

        // Low salinity: use input current values for
        // both BCM and BCMV.
        //
        if (episodeSwitch){
            materialParams_.setPe_LS(this->get(i, "MatrixPe"));
            materialParams_.setLambda_LS(this->get(i, "MatrixLambda"));
            materialParams_.setK0rw_LS(this->get(i, "MatrixKrwMax"));
            materialParams_.setK0rn_LS(this->get(i, "MatrixKrnMax"));
            materialParams_.setNw_LS(this->get(i, "Matrixnw"));
            materialParams_.setNn_LS(this->get(i, "Matrixnn"));
            materialParams_.setSwr_LS(this->get(i, "MatrixSwr"));
            materialParams_.setSnr_LS(this->get(i, "MatrixSnr"));

            //  Salinity set from single particle (Particle.1)
            //        Here we should change salinity for ionic
            //        strength in chemical model.
            materialParams_.setLS(this->xParticleTotal(i));
        } 

        
            
        // High Salinity
        if (this->useBCM_){
            // No interpolation here
            // Use input current values.
            //
            materialParams_.setPe_HS(this->get(i, "MatrixPe"));
            materialParams_.setLambda_HS(this->get(i, "MatrixLambda"));
            materialParams_.setK0rw_HS(this->get(i, "MatrixKrwMax"));
            materialParams_.setK0rn_HS(this->get(i, "MatrixKrnMax"));
            materialParams_.setNw_HS(this->get(i, "Matrixnw"));
            materialParams_.setNn_HS(this->get(i, "Matrixnn"));
            materialParams_.setSwr_HS(this->get(i, "MatrixSwr"));
            materialParams_.setSnr_HS(this->get(i, "MatrixSnr"));
            materialParams_.setHS(this->xParticleTotal(i));
            // Salinity is set to input current value.
            materialParams_.setS(this->xParticleTotal(i));
            return materialParams_;
        }
        

       // Interpolate between high and low values
       //
       // If input salinity has not changed, keep 
       // going with the initial values.
        

        // High value will be either initial values
        // on the first episode or the values of 
        // the previous when an input salinity change
        // is specified.
        //
        if (episodeSwitch){
            int thisStage;
            static Scalar lastInputSalinity = -1;
            if (lastInputSalinity < 0) {
                lastInputSalinity = this->xParticleTotal(episode);
            }
            auto inputSalinity = this->xParticleTotal(episode);
            if (inputSalinity != lastInputSalinity) {
                // Reset last input salinity.
                DBG("***************************\n*********************\n");
                DBG("reset input salinity.. %le --> %le\n", lastInputSalinity,inputSalinity);
                lastInputSalinity = inputSalinity;

                // Since salinity has changed, reset HS values to the 
                // LS values of the previous stage.
                //
                thisStage = this->stageNumber(episode);// this stage.
                int j = thisStage-1; // previous stage
                materialParams_.setPe_HS(this->getFromStage(j, "MatrixPe"));
                materialParams_.setLambda_HS(this->getFromStage(j, "MatrixLambda"));
                materialParams_.setK0rw_HS(this->getFromStage(j, "MatrixKrwMax"));
                materialParams_.setK0rn_HS(this->getFromStage(j, "MatrixKrnMax"));
                materialParams_.setNw_HS(this->getFromStage(j, "Matrixnw"));
                materialParams_.setNn_HS(this->getFromStage(j, "Matrixnn"));
                materialParams_.setSwr_HS(this->getFromStage(j, "MatrixSwr"));
                materialParams_.setSnr_HS(this->getFromStage(j, "MatrixSnr"));

                DBG("MatrixSnr=%lf MatrixKrnMax=%lf\n",this->getFromStage(j, "MatrixSnr"),
                    this->getFromStage(j, "MatrixKrnMax"));

                materialParams_.setHS(this->xParticleTotalFromStageNumber(j));           
            }
        }
        

        // Salinity is set to solution salinity value at element.
        // Case with single particle (NaCl): XXX.
        /*static constexpr int NaClIdx = FluidSystem::comp0Idx+1;
        const auto salinity = elemSol[scv.indexInElement()][NaClIdx];//XXX
        materialParams_.setS(salinity);
        TRACE("high=%le, low=%le, current=%le\n", 
                this->getValue("xNa"), this->get(i, "xNa"), salinity);*/
       
        Scalar totalSalinity = 0.0;      
        for (int compIdx = 1; compIdx < FluidSystem::numComponents-1; ++compIdx)
        {
            totalSalinity += elemSol[scv.indexInElement()][compIdx];
        }
        materialParams_.setS(totalSalinity);


        if (episodeSwitch) 
        {
            DBG("*** salinity = %le, totalSalinity = %le\n", 
                    elemSol[scv.indexInElement()][1], totalSalinity);
            this->dump();
        }
        return materialParams_;
   }

    template<class FS>
    int wettingPhaseAtPos(const GlobalPosition& globalPos) const
    {
      return FS::phase0Idx;
    }

private:
    mutable MaterialLawParams materialParams_;

};

}//end namespace

#endif
