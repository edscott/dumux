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
 * \brief Definition of a problem for low-salinity water flooding.
 */
#ifndef LSWI_PROBLEM_HH
#define LSWI_PROBLEM_HH

#include <dune/grid/yaspgrid.hh>

#include <dumux/discretization/elementsolution.hh>

#include <dumux/discretization/box.hh>
#include <dumux/discretization/evalsolution.hh>
#include <dumux/discretization/evalgradients.hh>
#include <dumux/porousmediumflow/problem.hh>
#include <dumux/material/fluidsystems/1pliquid.hh>

// Local implementations of second order templates:
#include "dumux/porousmediumflow/2pncimmiscible/model.hh"
#include "dumux/material/fluidsystems/2pncimmiscible.hh"

#include "dumux/material/fluidsystems/brine-n.hh"
#include "dumux/material/components/oil.hh"

namespace Dumux {

/*!
 * \ingroup TwoPNCTests
 * \brief Definition of a base problem for 2pnc immiscible model.
 */
// The problem class is forward declared:
template <class TypeTag>
class LSWF2pncProblem;

// We enter the namespace Properties, which is a sub-namespace of the namespace Dumux:
namespace Properties
{
// Create new type tags
namespace TTag {
struct LSWF2pncTypeTag { using InheritsFrom = std::tuple<TwoPNCImmiscible>; };
struct LSWFBoxTypeTag { using InheritsFrom = std::tuple<LSWF2pncTypeTag, BoxModel>; };
} // end namespace TTag

// Set the grid type. We use a structured 2D grid.
template<class TypeTag>
struct Grid<TypeTag, TTag::LSWF2pncTypeTag> { using type = Dune::YaspGrid<2>; };

// Set the problem property
// The problem class specifies initial and boundary conditions:
template<class TypeTag>
struct Problem<TypeTag, TTag::LSWF2pncTypeTag> { using type = LSWF2pncProblem<TypeTag>; };

// We define the spatial parameters for our simulation:
// This by setting the spatial parameters property.
template<class TypeTag>
struct SpatialParams<TypeTag, TTag::LSWF2pncTypeTag> { 
    using type = LSWF2pncSpatialParams<TypeTag>; 
};

// Set the primary variable combination for the 2pnc model
template<class TypeTag>
struct Formulation<TypeTag, TTag::LSWF2pncTypeTag>
{
public:
  static constexpr TwoPFormulation value = TwoPFormulation::p1s0;
};

#define SCALAR_ GetPropType<TypeTag, Properties::Scalar>

template <class TypeTag>
struct FluidSystem<TypeTag, TTag::LSWF2pncTypeTag> {
    using type = FluidSystems::TwoPNCImmiscible<
        SCALAR_,  // First template argument
        FluidSystems::Brine<SCALAR_>,  // second template argument: MultiComponentPhase
        // third template argument: SingleComponentPhase, this will in turn
        // have a template argument specifying the component
        FluidSystems::OnePLiquid<SCALAR_, Components::Oil<SCALAR_> > >;
};

// Define whether mole(true) or mass (false) fractions are used
template<class TypeTag>
struct UseMoles<TypeTag, TTag::LSWF2pncTypeTag> { static constexpr bool value = true; };

} // end namespace Properties

template <class TypeTag>
class LSWF2pncProblem : 
    public PorousMediumFlowProblem<TypeTag>
    ,protected LswiData<TypeTag>
{
    using ParentType = PorousMediumFlowProblem<TypeTag>;

    using Scalar = GetPropType<TypeTag, Properties::Scalar>;
    using Indices = typename GetPropType<TypeTag, Properties::ModelTraits>::Indices;
    using FluidSystem = GetPropType<TypeTag, Properties::FluidSystem>;
    using BoundaryTypes = GetPropType<TypeTag, Properties::BoundaryTypes>;
    using PrimaryVariables = GetPropType<TypeTag, Properties::PrimaryVariables>;
    using NumEqVector = GetPropType<TypeTag, Properties::NumEqVector>;
    using ElementVolumeVariables = typename GetPropType<TypeTag, Properties::GridVolumeVariables>::LocalView;
    using ElementGeometry = typename GetPropType<TypeTag, Properties::GridGeometry>::LocalView;
    using SubControlVolume = typename ElementGeometry::SubControlVolume;
    using SubControlVolumeFace = typename ElementGeometry::SubControlVolumeFace;
    using GridView = GetPropType<TypeTag, Properties::GridView>;
    using Element = typename GridView::template Codim<0>::Entity;
    using GridGeometry = GetPropType<TypeTag, Properties::GridGeometry>;
    using SolutionVector = GetPropType<TypeTag, Properties::SolutionVector>;
    using VolumeVariables = GetPropType<TypeTag, Properties::VolumeVariables>;

    enum { numComponents = FluidSystem::numComponents };

    // privar indices
    enum
    {
        pressureIdx = Indices::pressureIdx,
        saturationIdx = Indices::saturationIdx,

        // indices of the equations
        contiH2OEqIdx = Indices::conti0EqIdx + FluidSystem::comp0Idx,
        contiOilEqIdx = Indices::conti0EqIdx + FluidSystem::lastCompIdx,

        //components Index
        H2OIdx = FluidSystem::comp0Idx,
        OilIdx = FluidSystem::lastCompIdx,

        //Phase Index
        BrinePhaseIdx = FluidSystem::phase0Idx,
        OilPhaseIdx = FluidSystem::phase1Idx
    };
    // Component indexes in [FluidSystem::comp0Idx+1, FluidSystem::lastCompIdx-1]
    // Equation indexes in [contiH2OEqIdx+1, contiOilEqIdx-1]

    static constexpr int dim = GridView::dimension;

    static constexpr bool useMoles = getPropValue<TypeTag, Properties::UseMoles>();
    //static constexpr bool useMoles = true; // deprecated, only use moles
    static constexpr bool isBox = GridGeometry::discMethod == DiscretizationMethod::box;
    static constexpr int dimWorld = GridView::dimensionworld;
    using GlobalPosition = Dune::FieldVector<Scalar, dimWorld>;

    enum { dofCodim = isBox ? dim : 0 };
    mutable int currentEpisode_;

public:
    bool shouldWriteRestartFile() const
    {
        return true;
    }

    int currentEpisode(void) const {return currentEpisode_;}
    Scalar oilDensity(int episodeIdx) const {
        return this->oilDensity_[episodeIdx];
    }

    std::string episodeStageName(int i){ 
        return this->episodeName(i);
    }

    int stageCount(void){ 
        return this->episodes_;
    }

    int episodeCount(void){ 
        return this->episodes_;
    }

    Scalar getTEnd(void){
        return this->TEnd();
    }
    
    Scalar getLowerTimeStepBoundary(int episodeIndex){ 
        return  this->lowerTimeStepBoundary(episodeIndex);
    }

    Scalar getUpperTimeStepBoundary(int episodeIndex){ 
        if (episodeIndex < 0) return this->TEnd_;
        return  this->upperTimeStepBoundary(episodeIndex);
    }
    Scalar getDtInitial(int episodeIndex){ 
        return  this->DtInitial(episodeIndex);
    }

    Scalar getMaxTimeStepSize(void){ 
        return  this->maxTimeStepSize();
    }

    Scalar getMaxTimeStepSize(int episodeIndex){ 
        return  this->maxTimeStepSize(episodeIndex);
    }

    Scalar getTarget(int episodeIdx){ 
        return this->target(episodeIdx);
    }
    
    /*!
     * \brief The constructor
     *
     * \param timeManager The time manager
     * \param gridView The grid view
     */
    LSWF2pncProblem(std::shared_ptr<const GridGeometry> fvGridGeometry)
    : ParentType(fvGridGeometry)
    {
        // Consider constant porosity throughout the run 
        // (no geomechanics).
        static Scalar porosity = this->getValue("MatrixPorosity");
        // Consider the initial Swr for recovery calculation.
        static Scalar Swr = this->getValue("MatrixSwr"); 
        // Consider initial density and viscosity as well.
        // (hmmm...)
        static Scalar brineDensity = this->getValue("BrineDensity");
        static Scalar brineViscosity = this->getValue("BrineViscosity");

        if (!useMoles){
            DBG("!useMoles is deprecated\n");
            exit(1);
        }
        // initial brine densities and viscosities           
        recovery_ = "gnuplot.dat";
        currentEpisode_ = 0;

        // Total area of all top scvf.area() in two dimensions
        simulationArea_ = this->upperRight_[0] * 1.0; 
        // Area of test sample in three dimensions:
        cylinderArea_ = pow(this->upperRight_[0]/2.0,2) * 3.14159; // cylinder (not applicable)
        cylinderOpenVolume_ = cylinderArea_ * this->upperRight_[1]* porosity ;
        this->effectiveVolume_ = simulationArea_ * this->upperRight_[1]* porosity ;
        this->oilVolume_ = (1.0 - Swr) * this->effectiveVolume_;
        
        // initial brine densities and viscosities           
        //initialize Brine fluid system
            DBG("initialize Brine fluid system\n");
        FluidSystems::Brine<GetPropType<TypeTag, Properties::Scalar>>::init(brineDensity, brineViscosity,this->particles_);

        // write caption into output file
            DBG("write caption into output file\n");
        std::ofstream outputFile;
        std::string fileName = this->name_ + "_OilRecovery" + ".log";
        outputFile.open(fileName, std::ios::out);
        outputFile << "Step | Current Time [s] | Step Size [s] | Step Recovery [m3/s] | Total recovery [m3] | Percent recovery\n";
        outputFile.close();
            DBG("LSWF2pncProblem constructor OK\n");

    }

    /*!
     * \name Problem parameters
     */

    /*!
     * \brief The problem name.
     *
     * This is used as a prefix for files generated by the simulation.
     */
    const std::string& name() const
    { return this->name_; }

    /*!
     * \brief Returns the temperature within the domain.
     *
     * This problem assumes a temperature of 90 degrees Celsius.
     */
    Scalar temperature() const {
        // Input file temperature is in Celcius, so we change to SI (K).
        // Episode defined value not available.
        static Scalar initialT = this->getValue("Temperature") + 273.15;
        return initialT; 
    } 


    //! \copydoc Dumux::FVProblem::source()
    NumEqVector source(const Element& element,
                   const ElementGeometry& fvGeometry,
                   const ElementVolumeVariables& elemVolVars,
                   const SubControlVolume& scv) const
    { return NumEqVector(0.0); }
    
    void setDensityViscosity(int episodeIdx){
        // set constant densities and viscosities (called from timeloop)
        FluidSystems::episodeDensity_ = this->injectionDensity(episodeIdx);
        FluidSystems::episodeViscosity_ = this->injectionViscosity(episodeIdx);
//        FluidSystems::episodeViscosity_ = this->brineViscosity_[episodeIdx];
        DBG("Problem now setting episode %d density=%le viscosity=%le\n", episodeIdx, FluidSystems::episodeDensity_, FluidSystems::episodeViscosity_);
    }

      const Scalar injectionDensity(int epIdx){ 
              return this->brineDensity(epIdx); 
      }

      const Scalar injectionViscosity(int epIdx){ 
              return this->brineViscosity(epIdx);
           
      }

    /*!
     * \name Boundary conditions
     */

    /*!
     * \brief Specifies which kind of boundary condition should be
     *        used for which equation on a given boundary segment
     *
     * \param globalPos The global position
     */
    BoundaryTypes boundaryTypesAtPos(const GlobalPosition& globalPos) const
    {
        BoundaryTypes bcTypes;
        bcTypes.setAllNeumann();

        return bcTypes;
    }

    /*!
     * \brief Evaluates the boundary conditions for a Dirichlet
     *        boundary segment
     *
     * \param globalPos The global position
     */
    PrimaryVariables dirichletAtPos(const GlobalPosition& globalPos) const
    {
        DUNE_THROW(Dune::NotImplemented, "Dirichlet BCs for this test.");
    }

    /*!
     * \brief Evaluate the boundary conditions for a neumann
     *        boundary segment.
     */
    PrimaryVariables neumann(const Element& element,
                             const ElementGeometry& fvGeometry,
                             const ElementVolumeVariables& elemVolVars,
                             const SubControlVolumeFace& scvf) const
    {
      // set a fixed pressure on the Outlet
      const Scalar dirichletPressure = this->InitialPressure();
 
      PrimaryVariables flux(0.0);
      const auto& ipGlobal = scvf.ipGlobal();
      const auto& volVars = elemVolVars[scvf.insideScvIdx()];

      // no-flow everywhere except at the Outlet/Inlet)
      if(ipGlobal[1] < this->fvGridGeometry().bBoxMax()[1] - eps_ && ipGlobal[1] > this->fvGridGeometry().bBoxMin()[1] + eps_)
      {
          return flux;
      }

      if ( ipGlobal[1] < this->fvGridGeometry().bBoxMin()[1] + eps_)
      {
          // calculate the flux

          const Scalar brinedensity = useMoles ? volVars.molarDensity(BrinePhaseIdx) : volVars.density(BrinePhaseIdx);
          Scalar brinetpfaFlux = brinedensity * this->InjectionVelocity(currentEpisode_);

          // ///////////////   influx  ///////////////
          int epIdx;
          for (epIdx=0; epIdx < this->episodes_; epIdx++){
              TRACE( "test: %le <= %le < %le\n", 
                      this->lowerTimeStepBoundary(epIdx),this->time_ ,
                      this->upperTimeStepBoundary(epIdx));
            if (this->time_ >= this->lowerTimeStepBoundary(epIdx)
                    &&
                    this->time_ < this->upperTimeStepBoundary(epIdx)
                 )
            {
                TRACE("episode %d\n", epIdx);
                break;
            }
          }
          

          if (currentEpisode_ != epIdx) {
              DBG("Problem now setting episode %d density=%le viscosity=%le\n", epIdx, FluidSystems::episodeDensity_, FluidSystems::episodeViscosity_);

              DBG("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n"); 
              DBG("influx brine molar density = %lf brine mass density = %lf injection velocity[%d] = %le\n", 
                volVars.molarDensity(BrinePhaseIdx), volVars.density(BrinePhaseIdx),
                epIdx, this->InjectionVelocity(epIdx));
              for (int i=0; i<numComponents-2; i++) {
                  DBG("influx Boundary condition switch switch at %lf s., episode=%d --> %d: particle-%d flux= %le -->%le,\n", 
                      time_, currentEpisode_, epIdx, i,
                      -brinetpfaFlux *this->xParticle(i, currentEpisode_), 
                      -brinetpfaFlux *this->xParticle(i, epIdx));
                  }
              DBG("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n"); 
              currentEpisode_ = epIdx;
          }
          Scalar moleFracSum = 0;
          for (int i=0; i<numComponents-2; i++){
              Scalar moleFrac = this->xParticle(i, currentEpisode_);
              flux[i + contiH2OEqIdx + 1] = -brinetpfaFlux * moleFrac;
              moleFracSum += moleFrac;       
          }

          flux[contiH2OEqIdx] = -brinetpfaFlux * ( 1-moleFracSum  );

          return flux;
      }

      // construct the element solution
      const auto elemSol = [&]()
      {
          auto sol = elementSolution(element, elemVolVars, fvGeometry);

          if(isBox)
          {
              for(auto&& scvf : scvfs(fvGeometry))
                  if(scvf.center()[1] > this->fvGridGeometry().bBoxMax()[1] - eps_)
                  {
                      sol[fvGeometry.scv(scvf.insideScvIdx()).indexInElement()][pressureIdx] = dirichletPressure;
                  }
          }
          return sol;
      }();

      // evaluate the gradient
      const auto gradient = [&]()->GlobalPosition
      {
          if(isBox)
          {
              const auto grads = evalGradients(element, element.geometry(), fvGeometry.fvGridGeometry(), elemSol, ipGlobal);
              return grads[pressureIdx];
          }

          else
          {
              const auto& scvCenter = fvGeometry.scv(scvf.insideScvIdx()).center();
              const Scalar scvCenterPresureSol = elemSol[0][pressureIdx];
              auto grad = ipGlobal - scvCenter;
              grad /= grad.two_norm2();
              grad *= (dirichletPressure - scvCenterPresureSol);
              return grad;
          }
      }();

      const Scalar K = volVars.permeability();


      const Scalar brinedensity = useMoles ? volVars.molarDensity(BrinePhaseIdx) : volVars.density(BrinePhaseIdx);

      const Scalar oildensity = useMoles ? volVars.molarDensity(OilPhaseIdx) : volVars.density(OilPhaseIdx);

      //////////////////  outflux  ////////////////////////
      // calculate the flux
      //
      // 
      Scalar brinetpfaFlux = gradient * scvf.unitOuterNormal();
      brinetpfaFlux *= -1.0  * K;
      brinetpfaFlux *=  brinedensity * volVars.mobility(BrinePhaseIdx);

      Scalar oiltpfaFlux = gradient * scvf.unitOuterNormal();
      oiltpfaFlux *= -1.0  * K;
      oiltpfaFlux *=  oildensity * volVars.mobility(OilPhaseIdx);

      // emulate an outflow condition for the component transport on the Outlet
      flux[contiH2OEqIdx] = brinetpfaFlux  * (useMoles ? volVars.moleFraction(BrinePhaseIdx, H2OIdx) : volVars.massFraction(BrinePhaseIdx, H2OIdx));
      flux[contiOilEqIdx] = oiltpfaFlux;
      for (int i=1; contiH2OEqIdx + i < contiOilEqIdx; i++) {
          int equationIdx = contiH2OEqIdx + i;
          int particleIdx = H2OIdx + i;
          flux[equationIdx] = brinetpfaFlux  * (useMoles ? volVars.moleFraction(BrinePhaseIdx, particleIdx) : volVars.massFraction(BrinePhaseIdx, particleIdx));
      }

      return flux;
    }

    template<class GridVolumeVariables, class SolutionVector>
    void oilRecOutput(const GridVolumeVariables& gridVolVars, const SolutionVector& x, int episodeIdx) const
    {
        static Scalar total = 0.0;
        Scalar outflux = 0.0;
        for (const auto& element : elements(this->fvGridGeometry().gridView()))
        {
            auto fvGeometry = localView(this->fvGridGeometry());
            auto elemVolVars = localView(gridVolVars);

            fvGeometry.bind(element);
            elemVolVars.bind(element, fvGeometry, x);

            for (const auto& scvf : scvfs(fvGeometry))
            {
                const auto& ipGlobal = scvf.ipGlobal();
                const auto& volVars = elemVolVars[scvf.insideScvIdx()];
                const Scalar oildensity = useMoles ? volVars.molarDensity(OilPhaseIdx) : volVars.density(OilPhaseIdx);
                if (ipGlobal[1] > this->fvGridGeometry().bBoxMax()[1] - eps_){
                    outflux += neumann(element, fvGeometry, elemVolVars, scvf)[contiOilEqIdx]*scvf.area()/oildensity;
  

                    TRACE("    neumann(element, fvGeometry, elemVolVars, scvf)[contiOilEqIdx]=%le\n",neumann(element, fvGeometry, elemVolVars, scvf)[contiOilEqIdx]); 

                    // [neumann] = mol/s/m2 (gradient * scvf.unitOuterNormal() *  density * mobility)
                    // [neumann] / [oildensity] = (mol / s / m2) / (mol / m3) = m / s;    
                }
            }
        }

        auto initialOil = oilVolume_; // saturacion*w*h*porosidad*1

        TRACE("initialOil: %le, x=%lf y=%lf sat=%lf phi=%le\n",
            initialOil,  
            this->upperRight_[0], this->upperRight_[1],
            1.0 - this->wettingSaturation_, this->spData_().porosity_);

        std::ofstream outputFile;
        std::string fileName = name() + "_OilRecovery" + ".log";
        outputFile.open(fileName, std::ios::out | std::ios::app);
        total += (outflux*step_);

        Scalar injectionVolumeRate = this->InjectionVelocity(episodeIdx) * simulationArea_ * this->MatrixPorosity(episodeIdx);
        Scalar VPIi = injectionVolumeRate * step_;


        static Scalar VPIt = 0;
        VPIt += VPIi;

        Scalar VP = effectiveVolume_;

        Scalar VPIn = VPIt / VP;
        TRACE(" injectionVolumeRate=%le VPIi=%le VPIt=%lf, VPIn=%lf\n", 
            injectionVolumeRate, VPIi, VPIt, VPIn);
        
        outputFile << std::setprecision(8) 
            << stepIndex_ << ", " << time_ << ", " << step_ // 1,2,3
            << ", " << outflux  << ", " << total << ", "    // 4,5
            << 100*total/initialOil  << '%'               // 6
            << ", " <<  VPIt << ", " << VPIn          // 7,8
            << std::endl;
        TRACE("satW=%lf x=%lf y=%lf porosity= %lf, recovery= %lf%%\n", 
                this->wettingSaturation_,this->upperRight_[0],this->upperRight_[1],
                this->porosity_,
                100*total/initialOil);
        outputFile.close();

        // gnuplot file from 2.12 lswf:
        auto velocity = outflux/simulationArea_;
        recoveryOutput(stepIndex_,time_,step_,velocity, outflux*step_, total, episodeIdx);
        


    }
    

    void setTime(int i, Scalar t, Scalar s)
    {
        stepIndex_ = i-1;
        time_ = t;
        step_ = s;
    }

    /*!
     * \name Volume terms
     */


    /*!
     * \brief Evaluates the initial values for a control volume
     *
     * \param globalPos The global position
     */
    PrimaryVariables initialAtPos(const GlobalPosition& globalPos) const
    {
      PrimaryVariables priVars(0.0);
      priVars[pressureIdx] = this->InitialPressure();
      priVars[saturationIdx] = this->InitialSwr();
      // We begin with HS injection of initial conditions.
      for (int particle=0; particle < this->numParticles_; particle++) {
          int eqIdx = H2OIdx + particle + 1;
          priVars[eqIdx] = this->xParticleInitial(particle);
      }
      /*for (int i=1; H2OIdx + i < OilIdx; i++) {
          int particleIdx = H2OIdx + i;
          priVars[particleIdx] = this->xParticle_[0];
      }*/
      return priVars;
    }
    
    void recoveryOutput(int stepIndex, Scalar now, Scalar stepSize, Scalar averageVelocity, Scalar stepRecovery, Scalar totalRecovery, int episodeIdx) const {
        static Scalar IPV = 0;

        static time_t start = time(NULL); 
        // Pore volume is the effectiveVolume_ (volume*porosity);
        //Scalar area =  cylinderArea_;
        //Scalar poreVolume =  cylinderOpenVolume_;
        Scalar area =  simulationArea_;
        Scalar poreVolume =  effectiveVolume_;
        Scalar volumeFlow = this->InjectionVelocity(episodeIdx) * simulationArea_; 
        Scalar injectedVolume = volumeFlow * stepSize;
        Scalar stepIPV = injectedVolume/poreVolume; 

        IPV += stepIPV;
        static bool first = true;
        FILE *recovery;
        if (first) {
            recovery = fopen(recovery_.c_str(), "w");
            first = false;
            fprintf(stderr, "# step  currentTime stepSize averageVelocity  stepRecovery  totalRecovery percent_recovery wall_time(min) VPI(t)\n");
            fprintf(recovery, "# step  currentTime  stepSize averageVelocity  stepRecovery  totalRecovery percent_recovery wall_time(min) VPI(t)\n");
        } else {
            recovery = fopen(recovery_.c_str(), "a");
        }

        if (!std::isnan(averageVelocity)) {
            Scalar elapsed = (time(NULL) - start)/60.0;
            
            fprintf(stderr, " %d  %e   %e    %e  %e   %e   %lf %lf (%.1lf minutes)\n", 
                       stepIndex+1, now, stepSize, averageVelocity, stepRecovery, 
                       totalRecovery, 
                       totalRecovery/oilVolume_*100 + this->restartRecovery_,
                       IPV,
                       elapsed);  
            fprintf(recovery, "%d  %e   %e    %e  %e   %e   %lf %lf\n",
                       stepIndex+1, now, stepSize, averageVelocity, stepRecovery, 
                       totalRecovery, 
                       totalRecovery/oilVolume_*100 + this->restartRecovery_,
                       IPV);   
          oilRecovery = totalRecovery/oilVolume_*100 + this->restartRecovery_;
        }
        fclose(recovery);
    }
private:

    std::string recovery_ ;

    Scalar cylinderArea_;
    Scalar cylinderOpenVolume_;
    Scalar effectiveVolume_;
    Scalar oilVolume_;
    Scalar simulationArea_;

    
    int stepIndex_;
    Scalar step_;
    Scalar time_;
public:
    static constexpr Scalar eps_ = 1e-6;

};

} //end namespace Dumux

#endif
