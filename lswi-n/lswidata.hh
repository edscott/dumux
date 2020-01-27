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



#ifndef LSWI_DATA_HH
#define LSWI_DATA_HH
#include "dumux/common/episodedata.hh"

namespace Dumux {

static const char *lswiScalars[]={
            "Temperature",
            "InitialPressure",
            "DtInitial",
            "MaxTimeStepSize",
            "TEnd",
            "TimeLimit",
            "MatrixPermeability",
            "MatrixPorosity",
            "MatrixLambda",
            "MatrixKrwMax",
            "MatrixSwr",
            "Matrixnw",
            "MatrixPe",
            "Matrixnn",
            "MatrixKrnMax",
            "MatrixSnr",
            "InjectionVelocity",
            "BrineDensity",
            "BrineViscosity",
            "RelativeVelocity",
            //"xNa",
            NULL
        };

template <class TypeTag>
class LswiData: public EpisodeData<TypeTag> {
    using Scalar = GetPropType<TypeTag, Properties::Scalar>;

    const char **scalars_;
    Scalar *target_;
    Scalar **xParticles_;

protected:
    Scalar restartRecovery_;
    int useBCM_;
    int numParticles_;
    particle_t *particles_;
public:
    LswiData():
        scalars_(lswiScalars),
        target_(nullptr)
    {
        this->init(scalars_);
        restartRecovery_ = getParam<Scalar>("Restart.Recovery", 0.0);
        useBCM_ = getParam<int>("SpatialParams.useBCM", 0);

        target_ = (Scalar *)calloc(this->episodes_, sizeof(Scalar));
        if (!target_) this->callocError("LswiData target_");
        int k = 0;
        for (int i=0; i<this->stages_; i++){
            auto stageEpisodes = this->getStageEpisodes(i);
            for (int j=0; j<stageEpisodes; j++, k++){
                auto e = this->episodeGroup(i,j);
                std::string variable = e + ".target";
                if (hasParam(variable)) {
                    target_[k] = getParam<Scalar>(variable);
                } else target_[k] = 0;
            }
        } 

        //
        // Get heap memory for Cation/anion concentrations.
        numParticles_ = getParam<int>("Problem.Particles");
        if (numParticles_ != BRINE_N_COMPONENTS - 1){
            DUNE_THROW(Dune::InvalidStateException, "Problem.Particles does not match (BRINE_N_COMPONENTS-1) : " << numParticles_ << " != " << BRINE_N_COMPONENTS-1);
        }

        particles_ = (particle_t *)calloc(numParticles_, sizeof(particle_t));
        if (!particles_) this->callocError("(particle_t *)");

        // Get particle Idx and Molecular weight from Problem.Particle.n group
        std::string problem = "Problem.Particle.";
        for (int particle=0; particle<numParticles_; particle++){
            std::string Idx = problem + std::to_string(particle+1)+".Idx";
            std::string Mw = problem + std::to_string(particle+1)+".MolecularWeight";
            DBG("get parameter \"%s\"\n", Idx.c_str());
            particles_[particle].idx = getParam<std::string>(Idx);
            DBG("got parameter \"%s\"->%s\n", Idx.c_str(), particles_[particle].idx.c_str());
            DBG("get parameter \"%s\"\n", Mw.c_str());  
            particles_[particle].molecularWeight = getParam<Scalar>(Mw);
            DBG("got parameter \"%s\"->%lf\n", Mw.c_str(), particles_[particle].molecularWeight);
        }


        // We handle C heap memory for xParticles to avoid any compiler 
        // automatic C++ cleanup (just to be on the safe side of town).
        xParticles_ = (Scalar **)calloc(this->stages_+1, sizeof(Scalar *));
        if (!xParticles_) this->callocError("constructor, xParticles_");

        for (int stage=0; stage<this->stages_+1; stage++){
            xParticles_[stage] = (Scalar *)calloc(numParticles_, sizeof(Scalar));
            if (!xParticles_[stage]) {
                this->callocError("constructor,  xParticles_[stage]");
            }
        }
        if (hasParam("Problem.UseMoleFractions")){
            DBG("Using mole fractions for input (stage 0 is IC)\n");
            for (int stage=0; stage<this->stages_+1; stage++){
              for (int particle=0; particle<numParticles_; particle++){
                std::string x;
                if (stage==0){
                    x ="Problem.x" + particles_[particle].idx;
                } else {
                    x = "Stage." + std::to_string(stage) +".x" + particles_[particle].idx;
                }
                xParticles_[stage][particle] = getParam<Scalar>(x);
                DBG("particle-%d (%s) component: %s, mw=%lf, X=%le\n",
                        particle, x.c_str(), 
                        particles_[particle].idx.c_str(), 
                        particles_[particle].molecularWeight,
                        xParticles_[stage][particle]);
              }
            }
            return;
        } else {
            DBG("Using ppm for input (stage 0 is IC)\n");
        }

        // Data is input in ppm. Use component information to convert to mole fraction.
        std::vector<Scalar *> ppmParticles;
        ppmParticles.reserve(this->stages_+1);

        for (int stage=0; stage<this->stages_+1; stage++){
            ppmParticles[stage] = (Scalar *)calloc(numParticles_, sizeof(Scalar));
            if (!ppmParticles[stage]) {
                this->callocError("constructor,  ppmParticles_[stage]");
            }
        }

        // Get initial conditions for particles_ (stage 0)
        // Get stage conditions for particles_ (stage 1 through this->stages_)
        for (int stage=0; stage<this->stages_+1; stage++){
          for (int particle=0; particle<numParticles_; particle++){
            std::string ppm;
            if (stage==0){
                ppm ="Problem.ppm" + particles_[particle].idx;
            } else {
                ppm = "Stage." + std::to_string(stage) +".ppm" + particles_[particle].idx;
            }
            ppmParticles[stage][particle] = getParam<Scalar>(ppm);
            
          }
        }

        // Conversion to mole fraction
        Scalar molesParticle[numParticles_];
        Scalar density;
        
        for (int stage=0; stage<this->stages_+1; stage++){
            Scalar totalppk = 0.0;
            Scalar totalMoles = 0.0;
            if (stage==0){
                // initial density
                density = this->getValue("BrineDensity");
            } else {
                // input current density (here stage value is 
                // C indexed, not fortran indexed)
                // Probably should fix this index mixup.
                density = this->getStageValue(stage-1, "BrineDensity");
            }
            for (int particle=0; particle<numParticles_; particle++){
                Scalar ppk = ppmParticles[stage][particle]/1000.0;
                totalppk += ppk;
                molesParticle[particle] = 
                    ppk/particles_[particle].molecularWeight;
                totalMoles += molesParticle[particle];
            }
            Scalar molesWater = (density - totalppk)/18.015;
            for (int particle=0; particle<numParticles_; particle++){
                xParticles_[stage][particle] = 
                    molesParticle[particle] / (molesWater + totalMoles);
            }
        }
        for (int stage=0; stage<this->stages_+1; stage++){
          for (int particle=0; particle<numParticles_; particle++){
            std::string ppm;
            if (stage==0){
                ppm ="Problem.ppm" + particles_[particle].idx;
            } else {
                ppm = "Stage." + std::to_string(stage) +".ppm" + particles_[particle].idx;
            }
            DBG("particle-%d (%s) component: %s, mw=%lf, ppm=%le, X=%le\n",
                        particle,ppm.c_str(), 
                        particles_[particle].idx.c_str(), 
                        particles_[particle].molecularWeight,
                        ppmParticles[stage][particle],
                        xParticles_[stage][particle]);
          }
        }


        for (int stage=0; stage<this->stages_+1; stage++){
            free(ppmParticles[stage]);
        }
    }
    ~LswiData(void){
        if (target_) free(target_);
    }


    void dump(std::ostream& stream = std::cout){
        stream << "Problem: "<< this->name_ << std::endl;
        dumpInitial();
        dumpStages();
        dumpEpisodes();
        
    }

protected:
    int TEnd(void){ 
        return this->TEnd_;
    }

    Scalar target(int episodeIdx){ 
        return target_[episodeIdx];
    }

    Scalar InitialPressure(void) const {
        return this->getValue("InitialPressure");
    }
    Scalar InitialSwr(void) const {
        return this->getValue("MatrixSwr");
    }
    
    // In this problem we input episode time boundaries in hours.
    Scalar lowerTimeStepBoundary(int episode) const {
        Scalar hours = this->get(episode, "lowerTimeStepBoundary");
        return 3600 * hours;
    }
    // In this problem we input episode time boundaries in hours.
    Scalar upperTimeStepBoundary(int episode) const {
        Scalar hours = this->get(episode, "upperTimeStepBoundary");
        return 3600 * hours;
    }
 
    Scalar DtInitial(int episode) const {
        return this->get(episode, "DtInitial");
    }

    Scalar maxTimeStepSize(void) const {
        return this->getValue("MaxTimeStepSize");
    }

    Scalar maxTimeStepSize(int episode) const {
        return this->get(episode, "MaxTimeStepSize");
    }
    
    Scalar InjectionVelocity(int episode) const {
        Scalar relative = this->get(episode, "RelativeVelocity");
        return this->get(episode, "InjectionVelocity") * relative;
    }
    Scalar MatrixPorosity(int episode) const {
        return this->get(episode, "MatrixPorosity");
    }
    Scalar brineDensity(int episode) const {
        return this->get(episode, "BrineDensity");
    }
    Scalar brineViscosity(int episode) const {
        return this->get(episode, "BrineViscosity");
    }
    
    /*Scalar xNa(int episode) const {
        return this->get(episode, "xNa");
    }*/
    /*Scalar InitialxParticle(void) const {
        return this->getValue("xNa");
    }*/

    // For mole fractions, stage data is fortran indexed,
    // i.e., [1 .. this->stages_].
    Scalar xParticleTotal(int episode) const {
        Scalar total = 0;
        int stageIdx = this->stageNumber(episode)+1;
        for (auto particle=0; particle<numParticles_; particle++){
            total += xParticles_[stageIdx][particle];
        }
        return total;
    }
    Scalar xParticleTotalFromStageNumber(int i) const {
        Scalar total = 0;
        for (auto particle=0; particle<numParticles_; particle++){
            total += xParticles_[i][particle];
        }
        return total;
    }

    Scalar xParticle(int particleIdx, int episode) const {
        int stageIdx = this->stageNumber(episode)+1;
        return xParticles_[stageIdx][particleIdx];
    }

    // For mole fractions, stage 0 is initial conditions.
    Scalar xParticleInitial(int particleIdx) const {
        return xParticles_[0][particleIdx];
    }
    Scalar xParticleInitialTotal(void) const {
        Scalar total = 0;
        for (auto particle=0; particle<numParticles_; particle++){
            total += xParticles_[0][particle];
        }
        return total;
    }

private:

    void dumpInitial(std::ostream& stream = std::cout){
        for (auto p=scalars_; p && *p; p++){
            auto value = this->getValue(*p);
            stream << "initial " << this->name_ << ": " << *p << " --> " << value << std::endl;
        }    
    }
    
    void dumpStages(std::ostream& stream = std::cout){ 
        
        for (int i=0; i<this->stages_; i++){
            stream << "*** Stage(" << i << ", " << this->stageGroup(i) <<
                "): " << this->stageName(i) << std::endl;

            stream << "lswidata: target -->" << target_[i] << std::endl;
            for (auto p=scalars_; p && *p; p++){
                auto value = this->getStageValue(i, *p);
                auto defaultValue = this->getValue(*p);
                if (value!=defaultValue)
                    stream << "stagedata: " << *p << " --> " << value << std::endl;
            }            
        }
    }

    void dumpEpisodes(std::ostream& stream = std::cout){
        for (int i=0; i<this->episodes_; i++){
            auto group = this->episodeGroup(i);
            auto name = this->episodeName(i);
            stream << "*** Episode(" << i << ", " << group <<
                "): " << this->episodeName(i) << std::endl;
            stream << "lswidata: target --> " << target_[i] << std::endl;
            stream << "lswidata: lowerTimeStepBoundary_  --> " << 
                this->lowerTimeStepBoundary(i) << std::endl;
            stream << "lswidata: upperTimeStepBoundary_  --> " <<
                this->upperTimeStepBoundary(i) << std::endl;
            for (auto p=scalars_; p && *p; p++){
                auto value = this->getEpisodeValue(i, *p);
                auto defaultValue = this->getValue(*p);
                if (value!=defaultValue){
                    stream << "episodedata: " << *p << " --> " << value << std::endl;
                }
            }
        }
    }
   
    
};
}
#endif

