/**************************************************************************
 *  Copyright 2018 Instituto Mexicano del Petroleo
 *
 *     All rights reserved for internal usage
 *     (If distributed to third parties,
 *      please note DuMux opensource restrictions)
 *
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

 ***************************************************************************/
#ifndef LSWF_IONS_HH
#define LSWF_IONS_HH

#ifdef USE_ACTIVITY_COEFICIENTS
# define GAMMA(X) (cm->gamma[X])
#else
# define GAMMA(X) 1.0
#endif
#define alog(X) pow(10, X)



#include <cmath>
#include <dumux/material/components/base.hh>
 
//#include "2pnc-release3.0-chemical/material/components/myOil1.hh"

// Transported and non transported ions
#include "2pnc-release3.0-chemical/material/components/ionBrine.hh"
// Transported components: Na+, Ca++, H+, Mg++, HCO3-, SO4--, Cl-
#include "2pnc-release3.0-chemical/material/components/na+.hh"
#include "2pnc-release3.0-chemical/material/components/ca++.hh"
#include "2pnc-release3.0-chemical/material/components/h+.hh"
#include "2pnc-release3.0-chemical/material/components/mg++.hh"
#include "2pnc-release3.0-chemical/material/components/hco3-.hh"
#include "2pnc-release3.0-chemical/material/components/so4--.hh"
#include "2pnc-release3.0-chemical/material/components/cl-.hh"
#include <dumux/material/components/simpleh2o.hh>
// Non transported components: CaCl+, CaSO4-(suspended), MgCl+, NaSO4-
#include "2pnc-release3.0-chemical/material/components/cacl+.hh"
#include "2pnc-release3.0-chemical/material/components/caso4.hh"
#include "2pnc-release3.0-chemical/material/components/mgcl+.hh"
#include "2pnc-release3.0-chemical/material/components/naso4-.hh"
#include "2pnc-release3.0-chemical/material/components/mgso4.hh"
#include "2pnc-release3.0-chemical/material/components/oh-.hh"
#include "2pnc-release3.0-chemical/material/components/co3--.hh"


# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
# define DBG(...)  fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);


static double brineDensity_;
static double k1A_;
static double k1B_;
static double k1C_;
static double ionicStrength_;
namespace Dumux
{
template <class Scalar>
class Ions
{
public:
    typedef Constants<Scalar> Constant;

    typedef Components::SimpleH2O<Scalar> Water;
    //typedef Components::MyOil<Scalar> MyOil;

    typedef Components::NaCation<Scalar> NaCation;
    typedef Components::CaCation<Scalar> CaCation;
    typedef Components::HCation<Scalar> HCation;
    typedef Components::MgCation<Scalar> MgCation;
    typedef Components::HCO3Anion<Scalar> HCO3Anion;
    typedef Components::SO4Anion<Scalar> SO4Anion;
    typedef Components::ClAnion<Scalar> ClAnion;
    typedef Components::OHAnion<Scalar> OHAnion;
    typedef Components::CaClCation<Scalar> CaClCation;
    typedef Components::MgClCation<Scalar> MgClCation;
    typedef Components::NaSO4Anion<Scalar> NaSO4Anion;
    typedef Components::CaSO4suspended<Scalar> CaSO4suspended;
    typedef Components::MgSO4<Scalar> MgSO4;
    typedef Components::CO3Anion<Scalar> CO3Anion;
    typedef Components::SimpleH2O<Scalar> H20;
    // Exported constants (not in Constants<Scalar>)
    static constexpr Scalar epsilonW_=55.3;  //
    static constexpr Scalar epsilonO_=8.83e-12;  //
    // Exported identifiers
    static const int numPhases = 2; // liquid phases: this is 2p model.
    static const int brinePhaseIdx = 0; // index of the brine phase
    static const int oilPhaseIdx = 1; // index of the oil phase
    static const int brineIdx = brinePhaseIdx; // index of the brine phase
    static const int oilIdx = oilPhaseIdx; // index of the oil phase
    // Reaction ions (implicit activities)
    //    [CO3--] = K_A * K_F / [Ca++]

    // Preprocessor cases (at least (and at most) one must be defined)
#ifdef SIMPLE_CACO3
    // Transported ions simple CaCO3 case.
    static const int transportIons = 4;
    static const int CaIdx   = 2;
    static const int HCO3Idx = 3;
    static const int HIdx = 4;
    static const int ClIdx = 5;
    // Concentrations by reactions:
    static const int CO3Idx = 6;
    // not used in SIMPLE_CACO3
    static const int SO4Idx  = 7;
    static const int NaIdx   = 8;
    static const int MgIdx   = 9;
#endif
#ifdef CACO3_CASO4
    // Transported ions in CaCO3/CaSO4 case.
    static const int transportIons = 6;
    static const int CaIdx   = 2;
    static const int HCO3Idx = 3;
    static const int HIdx = 4;
    static const int SO4Idx  = 5;
    static const int NaIdx   = 6;
    static const int ClIdx = 7;

    // Concentrations by reactions:
    static const int CO3Idx = 8;
    // not used in CACO3_CASO4
    static const int MgIdx   = 9;
#endif
#ifdef CACO3_CASO4_MGCO3
    // Transported ions in CaCO3/CaSO4/MgSO4 case.
    static const int transportIons = 7;
    static const int CaIdx   = 2;
    static const int HCO3Idx = 3;
    static const int HIdx = 4;
    static const int SO4Idx  = 5;
    static const int NaIdx   = 6;
    static const int MgIdx   = 7;
    static const int ClIdx   = 8;
    // Concentrations by reactions:
    static const int CO3Idx = 9;
#endif
    // Reaction ions (common for all cases)
    static const int H2OIdx   = 10;
    static const int OHIdx = 11;
    static const int CaClIdx = 12;
    static const int MgClIdx = 13;
    static const int NaSO4Idx = 14;
    static const int CaSO4Idx = 15;
    static const int MgSO4Idx = 16;
    // solid components
    static const int CaCO3sIdx = 17;
    static const int CaSO4sIdx = 18;
    static const int MgCO3sIdx = 19;
    // Total components
    static const int numIons = transportIons;
    static const int numComponents = numIons + numPhases;
    static const int numMinorComponents = numIons;
    static const int numMajorComponents = numPhases;

    // All components:
    // These are the internal identifiers for the chemical reactions.
    // Identifiers for transported ions will match the internal
    // identifiers.
    enum {
      // Phases  (match FluidSystem idx to chemical idx)
      _brine = brinePhaseIdx, // brine phase 0
      _oil = oilPhaseIdx, // oil phase 1
      // Transported components (match FluidSystem idx to chemical idx)
      _Na = NaIdx, //
      _Ca = CaIdx, //
      _H = HIdx,   //
      _Mg = MgIdx,//
      _HCO3 = HCO3Idx, //
      _SO4 = SO4Idx, //
      _Cl = ClIdx, //
      _H2O = H2OIdx, // water
      // Non transported chemical components
      _OH=OHIdx, // (-)
      _CaCl=CaClIdx, // (+)
      _MgCl=MgClIdx,// (+)
      _NaSO4=NaSO4Idx,// (-)
      _CaSO4=CaSO4Idx, // suspended
      _CO3=CO3Idx, // (--)
      _MgSO4=MgSO4Idx, //
      // Other components in solid, solid/aqueous or oil/aqueous interface
      _CaSO4s=CaSO4sIdx, // solid
      _CaCO3s=CaCO3sIdx, // solid
      _MgCO3s=MgCO3sIdx, // solid 19
      _MgCO3, // solid 20
      _RCOOH, // oil/aqueous 21
      _RCOO,// (-) oil/aqueous 22
      _RCOOCa, // (+) oil/aqueous 23
      _RCOOMg, // (+) oil/aqueous 24
      _vCaOH, // solid/aqueous 25 
      _vCaOH2, // + solid/aqueous 26
      _vCaSO4, // - solid/aqueous 27
      _vCaCO3, // - solid/aqueous 28
      _vCO3, // - solid/aqueous 29
      _vCO3H, // - solid/aqueous 30
      _vCO3Ca, // + solid/aqueous 31
      _vCO3Mg, // + solid/aqueous 32
      _vCaOH2RCOO, // + solid/aqueous 33
      _vCO3CaRCOO, // - solid/aqueous 34
      _vCO3MgRCOO, // - solid/aqueous 35
      _allComponents
    };
    // Export the defined total components.
    static const int allComponents = _allComponents;
    
    // Class template global functions
    //
    Ions(void){
        // XXX: These are default values. They are reset in lswf-problem.hh
       /* brineDensity_ = 1000.0;
        k1A_ = 1.73743e-05;
        k1B_ = 3.47486e-06;
        k1C_ = 1.73743e-08;*/
    }
    static Scalar brineDensity(void){return brineDensity_;}
    static void setBrineDensity(Scalar value){
        if (value == brineDensity_) return;
        DBG("Ions::Changing default brine density: %le-> %le\n", brineDensity_, value);
        brineDensity_ = value;
    }
    static void setK1A(Scalar value){
        if (value == k1A_) return;
        DBG("Ions::Changing default k1A: %le-> %le\n", k1A_, value);
        k1A_ = value;
    }
    static void setK1B(Scalar value){
        if (value == k1B_) return;
        DBG("Ions::Changing default k1B: %le-> %le\n", k1B_, value);
        k1B_ = value;
    }
    static void setK1C(Scalar value){
        if (value == k1C_) return;
        DBG("Ions::Changing default k1C: %le-> %le\n", k1C_, value);
        k1C_ = value;
    }
    static void setIonicStrength(Scalar value){
        if (value == ionicStrength_) return;
        DBG("Ions::Changing default ionic strength: %le-> %le\n", ionicStrength_, value);
        k1C_ = value;
    }

    //
    // name(): returns the name of the component.
    static std::string name(int compIdx) {
        switch (compIdx) {
            case NaIdx:return NaCation::name();
            case CaIdx:return CaCation::name();
            case HIdx:return HCation::name();
            case MgIdx:return MgCation::name();
            case HCO3Idx: return HCO3Anion::name();
            case SO4Idx: return SO4Anion::name();
            case ClIdx: return ClAnion::name();
            case OHIdx: return OHAnion::name();
            case CaClIdx: return CaClCation::name();
            case MgClIdx: return MgClCation::name();
            case NaSO4Idx: return NaSO4Anion::name();
            case CaSO4Idx: return CaSO4suspended::name();
            case CO3Idx: return CO3Anion::name();
            case H2OIdx: return H20::name();
            case oilIdx: return "oil";
            case brineIdx: return "brine";
        }
        DUNE_THROW(Dune::InvalidStateException, "name(): Invalid component index " << compIdx);
    }
    // molarMass(): returns the molecular mass of the component in mol/kg/1000
    // (division by 1000 since DuMux uses m3 as volumen unit).
    static Scalar molarMass(int compIdx) {
        switch (compIdx) {
            case brineIdx: return Water::molarMass();
            //case oilIdx:   return MyOil::molarMass();

            case NaIdx:return NaCation::molarMass();
            case CaIdx:return CaCation::molarMass();
            case HIdx:return HCation::molarMass();
            case MgIdx:return MgCation::molarMass();
            case HCO3Idx: return HCO3Anion::molarMass();
            case SO4Idx: return SO4Anion::molarMass();
            case ClIdx: return ClAnion::molarMass();
            case H2OIdx: return H20::molarMass();
            case OHIdx: return OHAnion::molarMass();
            case CaClIdx: return CaClCation::molarMass();
            case MgClIdx: return MgClCation::molarMass();
            case NaSO4Idx: return NaSO4Anion::molarMass();
            case CaSO4Idx: return CaSO4suspended::molarMass();
            case CO3Idx: return CO3Anion::molarMass();
        }
        DUNE_THROW(Dune::InvalidStateException, "ionMolarMass(): Invalid component index " << compIdx);
    }
    // isAcid(): returns boolean value "true" if ion is acidic (proton donor).
    static bool isAcid(int compIdx) {
        switch (compIdx) {
            case NaIdx:return NaCation::isAcid();
            case CaIdx:return CaCation::isAcid();
            case HIdx:return HCation::isAcid();
            case MgIdx:return MgCation::isAcid();
            case HCO3Idx: return HCO3Anion::isAcid();
            case SO4Idx: return SO4Anion::isAcid();
            case ClIdx: return ClAnion::isAcid();
            case H2OIdx: return false;
            case OHIdx: return OHAnion::isAcid();
            case CaClIdx: return CaClCation::isAcid();
            case MgClIdx: return MgClCation::isAcid();
            case NaSO4Idx: return NaSO4Anion::isAcid();
            case CaSO4Idx: return CaSO4suspended::isAcid();
            case CO3Idx: return CO3Anion::isAcid();
        }
        DUNE_THROW(Dune::InvalidStateException, "ionIsAcid(): Invalid component index " << compIdx);
    }
    // valence(); returns chemical valence of component.
    static Scalar valence(int compIdx) {
        switch (compIdx){
            case NaIdx:return NaCation::valence();
            case CaIdx:return CaCation::valence();
            case HIdx:return HCation::valence();
            case MgIdx:return MgCation::valence();
            case HCO3Idx: return HCO3Anion::valence();
            case SO4Idx: return SO4Anion::valence();
            case ClIdx: return ClAnion::valence();
            case H2OIdx: return 0;
            case OHIdx: return OHAnion::valence();
            case CaClIdx: return CaClCation::valence();
            case MgClIdx: return MgClCation::valence();
            case NaSO4Idx: return NaSO4Anion::valence();
            case CaSO4Idx: return CaSO4suspended::valence();
            case CO3Idx: return CO3Anion::valence();
            case MgSO4Idx: return MgSO4::valence();
        }
        DUNE_THROW(Dune::InvalidStateException, "ionValence(): Invalid component index " << compIdx);
    }
    //////////////////////////////////////////////////////////////////////////
    // ionicStrength(); global access function to solve chemical
    //                     reactions. Returns the associated ionic strength.
    //                     This function is called by model volumevariables.hh,
    //                     via immiscible-2pnc.hh fluidState.
    //
    // In ideal scenario, activities are all equal to one
    // In adjusted scenario, activities are calculated from ionic strength functions.

    // Same as above, but without solving ODE's when timeStepSize < 0

    // note: here ionic strength comes in with cm->iS
   static Scalar ionicStrength(chemicalModel_t *cm){
        Scalar *Ct = cm->molalities;
        //cm->iS = getIonicStrength(cm); 
        calculateGammas(cm); // Gammas with input iS. Will consider constant.
	solveEquilibrium(cm);
	// Final reaction rates and Q values (gammas do not change)
        solveR(cm);
	// Assign reaction rates for output to solutionDependentSourceTerm().
        // Here we need to convert back to mol/m3/s (DuMux units)
        // Internally we work with molal velocity, mol/kg/s.
        for (int componentIdx=0; componentIdx < numComponents; componentIdx++){
            // For Dumux we must convert our reaction rates from molal mol/kg/s to mol/m3/s
            cm->rateC[componentIdx] = cm->rateC[componentIdx] * brineDensity();
        }
        //exit(1);
        return cm->iS;
   }
#ifdef SIMPLIFIED
   static Scalar ionicStrength(Scalar timeStepSize, chemicalModel_t *cm){
        // Set up initial concentrations in Ct memory pointer
        Scalar *Ct = cm->molalities;
	// We get first ionic strength with Cl from input value.
	// Cl is set initially since it es also a minor component
	// (subject to transport).
	// requires Ca, H, HCO3, SO4, Na, Mg, Cl
        cm->iS = getIonicStrength(cm); 
        calculateGammas(cm); // Gammas with input iS. Will consider constant.

	// Now we get from equilibrium(iS): _OH, _NaSO4 which are required for 
	// solving electroneutrality.
	solveEquilibrium(cm);
 
	// Now we have all we need to solve ionic strength
	//    this will iterate on ionic strength and on equilibrium 
	//    concentrations and Cl concentration by electroneutrality.
        //
        // Does iS change with solve instead of get? (should not)
        // Scalar solvedIS =  solveIonicStrength(cm);
        // TRACE("get vs solved is: %le == %le (delta = %le)\n", cm->iS, solvedIS, fabs(cm->iS - solvedIS));
        // exit(1);
        // Result is exactly the same as with get instead of solve
	
	// Solve for chemical kinetics at final chemical values.
        TRACE("first pass Ca--> gamma=%lf\n", cm->gamma[_Ca]);
        
        /*
        // solveReactionRates() will alter [Cl] by electronegativity
        cm->iS = solveReactionRates(-1.0, cm); // negative timestep skips ODE's
        DBG("after second pass Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);
        */
	// Final reaction rates and Q values (gammas do not change, but Q values do...)
        solveR(cm);
        TRACE("after second pass Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);
	// Assign reaction rates for output to solutionDependentSourceTerm().
        for (int componentIdx=0; componentIdx < numComponents; componentIdx++){
            // For Dumux we must convert our reaction rates from molal mol/kg/s to mol/m3/s
            cm->rateC[componentIdx] = cm->rateC[componentIdx] * brineDensity();
        }
        //exit(1);
        return cm->iS;
    }
#else //  not SIMPLIFIED
   // Should work as the call without timeStepSize, when solve ODE is not set
    static Scalar ionicStrength(Scalar timeStepSize, chemicalModel_t *cm){
        // Set up initial concentrations in Ct memory pointer
        Scalar *Ct = cm->molalities;
        // XXX: transport may be feeding negative concentrations...
        //      (working the newton iterations)
        //      Since negative concentrations are physically impossible,
        //      the chemical model will work with small concentrations
        //      instead... 
        //       
        // Check for invalid input concentrations and correct:
        checkAllPositive(cm);
        //
        //
	// We get first ionic strength with Cl from input value.
	// Cl is set initially since it es also a minor component
	// (subject to transport).
	// requires Ca, H, HCO3, SO4, Na, Mg, Cl
        cm->iS = getIonicStrength(cm); 
        calculateGammas(cm); // Gammas with initial iS.
	// Now we get from equilibrium(iS): _OH, _NaSO4 which are required for 
	// solving electroneutrality.
	solveEquilibrium(cm);
	// With _OH, _NaSO4 we can now adjust Cl by electroneutrality:
	// this requires Ct[_Ca], Ct[_Mg], Ct[_SO4], Ct[_CO3], Ct[_H],
	//               Ct[_Na], Ct[_OH], Ct[_HCO3], Ct[_NaSO4].
#ifdef SOLVE_CL
        cm->molalities[_Cl] = solveCl(cm);  // by electroneutrality
#else
#warning "[Cl] is by transport only: no electroneutrality consideration."
#endif
	// Now we have all we need to solve ionic strength:
	//    this will iterate on ionic strength and on equilibrium 
	//    concentrations and Cl concentration by electroneutrality.
        cm->iS = solveIonicStrength(cm); 
        TRACE("is=%lf\n", cm->iS);
	// Make sure we have all chemical variables updated to final iS.
        calculateGammas(cm); // Calculate final gammas.
        solveEquilibrium(cm); // final equilibrium.
#ifdef SOLVE_CL
        cm->molalities[_Cl] = solveCl(cm);  // by electroneutrality
#endif
	
	// Solve for chemical kinetics at final chemical values.
        TRACE("first pass Ca--> gamma=%lf\n", cm->gamma[_Ca]);
        cm->iS = solveReactionRates(-1.0, cm); // negative timestep skips ODE's
        TRACE("after second pass Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);
	// Final reaction rates and Q values.
        solveR(cm);
        TRACE("after second pass Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);

	// Assign reaction rates for output to solutionDependentSourceTerm().
        for (int componentIdx=0; componentIdx < numComponents; componentIdx++){
            // For Dumux we must convert our reaction rates from mol/kg/s to mol/m3/s
            // This is done by multiplying by brineDensity() which comes in kg/m3
            cm->rateC[componentIdx] = cm->rateC[componentIdx] * brineDensity();
        }
        return cm->iS;
    }

 #endif

    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    //  Global (but internal and private) functions to solve the chemical reactions.
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
private:
    // getIonicStrength(); internal global access function to returns
    //                     the instantaneous associated ionic strength.
    static Scalar getIonicStrength(chemicalModel_t *cm){
        double *Ct = cm->molalities;
#ifdef SIMPLE_CACO3
#  ifdef ALL_IONS_FOR_IS
        int items[]={CaIdx, HIdx, HCO3Idx, ClIdx, -1};
#  else
        int items[]={CaIdx, HIdx, -1};
#  endif
#endif
#ifdef CACO3_CASO4
#  ifdef ALL_IONS_FOR_IS
        int items[]={CaIdx, HIdx, HCO3Idx, SO4Idx, NaIdx, ClIdx, -1};
#  else
        int items[]={CaIdx, HIdx, NaIdx, -1};
#  endif
#endif
#ifdef CACO3_CASO4_MGCO3
#  ifdef ALL_IONS_FOR_IS
        int items[]={CaIdx, HIdx, HCO3Idx, SO4Idx, NaIdx, MgIdx, ClIdx, -1};
#  else
        int items[]={CaIdx, HIdx, NaIdx, MgIdx, -1};
#  endif
#endif
        // Check only pertinent components for NAN:
        for (int i=0; items[i]>0; i++){
            int compIdx=items[i];
            if (isnan(Ct[compIdx])){
                std::cerr << "partial checkNAN: nan at element "<< compIdx << "\n";
                DUNE_THROW(Dune::InvalidStateException, "getIonicStrength(): NAN at component id: " << compIdx);
            } 
        }
        // check all components for NAN:
         checkNAN(Ct);
       
        // I = (4*(Ca + M       g + SO4) + H + Na + HCO3 + Cl)/2
        Scalar newiS = 0.0;
        for (int i=0; items[i]>0; i++){
            int compIdx=items[i];
            int v = abs(valence(compIdx));
            TRACE("valence(%s) = %d\n", name(compIdx).c_str(), v);
            if (v >= 2) v*=2;
            newiS += (v * Ct[compIdx]);
            if (newiS < 0) {
                DBG("newiS < 0\n");
            }
            TRACE("incremental %s newiS = %le\n", name(compIdx).c_str(), newiS);
        }
        newiS /= 2.0;
        TRACE( "*** point ionicStrength(): %le\n", newiS);
        return newiS;
    }
   // a(): returns the value of constant "a" for component.
    static bool useIdealGamma(int compIdx) {
        switch (compIdx){
            case NaIdx:
            case CaIdx:
            case HIdx:
            case MgIdx:
            case HCO3Idx:
            case SO4Idx:
            case ClIdx: 
            case OHIdx: 
            case CaClIdx:
            case MgClIdx:
            case NaSO4Idx:
            case CaSO4Idx:
            case CO3Idx: 
            case MgSO4Idx:
                return false;
        }
        return true;
    }
    // a(): returns the value of constant "a" for component.
    static Scalar a(int compIdx) {
        switch (compIdx){
            case NaIdx:return NaCation::a();
            case CaIdx:return CaCation::a();
            case HIdx:return HCation::a();
            case MgIdx:return MgCation::a();
            case HCO3Idx: return HCO3Anion::a();
            case SO4Idx: return SO4Anion::a();
            case ClIdx: return ClAnion::a();
            case OHIdx: return OHAnion::a();
            case CaClIdx: return CaClCation::a();
            case MgClIdx: return MgClCation::a();
            case NaSO4Idx: return NaSO4Anion::a();
            case CaSO4Idx: return CaSO4suspended::a();
            case CO3Idx: return CO3Anion::a();
            case MgSO4Idx: return MgSO4::a();
        }
        DUNE_THROW(Dune::InvalidStateException, "a(): Invalid component index " << compIdx);
    }
    // b(): returns the value of constant "b" for component.
    static Scalar b(int compIdx) {
        switch (compIdx){
            case NaIdx:return NaCation::b();
            case CaIdx:return CaCation::b();
            case HIdx:return HCation::b();
            case MgIdx:return MgCation::b();
            case HCO3Idx: return HCO3Anion::b();
            case SO4Idx: return SO4Anion::b();
            case ClIdx: return ClAnion::b();
            case OHIdx: return OHAnion::b();
            case CaClIdx: return CaClCation::b();
            case MgClIdx: return MgClCation::b();
            case NaSO4Idx: return NaSO4Anion::b();
            case CaSO4Idx: return CaSO4suspended::b();
            case MgSO4Idx: return MgSO4::b();
            case CO3Idx: return CO3Anion::b();
        }
        DUNE_THROW(Dune::InvalidStateException, "b(): Invalid component index " << compIdx);
    }
    // A(): returns the value of constant "A" for chemical system temperature.
    static Scalar A(Scalar temperature=130+273.15){

        Scalar A0 = 0.5058;
        Scalar T0 = 25+273.15;
        Scalar A1 = 0.6623;
        Scalar T1 = 130+273.15;
	return A1;
        Scalar m = (A0-A1)/(T0-T1);
        return m*(temperature - T0) +A0;

    }
    // B(): returns the value of constant "B" for chemical system temperature.
    static Scalar B(Scalar temperature=130+273.15){
        Scalar B0 = 0.3285e+10;
        Scalar T0 = 25+273.15;
        Scalar B1 = 0.3487e+10;
        Scalar T1 = 130+273.15;
	return B1;
        Scalar m = (B0-B1)/(T0-T1);
        return m*(temperature - T0) +B0;
    }
    // k1(): returns the value of k1, k2, k3 reaction rate constants, indexed
    //       by the reaction letter identifier (A-->1, B-->2, C-->3).
    static Scalar k1(char reaction) {
        // We use the k1 values in mol/lt/s since conversion to mol/kg/s is used internally
       switch (reaction){
           case 'A': return k1A_; //1.2e-05;// This is in mol/m3/s (molar velocity)
           case 'B': return k1B_; // 1.2e-08;
           case 'C': return k1C_; // 2.4e-06;
       }
       DUNE_THROW(Dune::InvalidStateException, "k1(): Invalid reaction index " << reaction);
    }
    // logK(): returns the log10 value of the reaction equilibrium constant, indexed
    //         by the reaction letter identifier.
    static Scalar logK(char reaction, Scalar temperature=110+273.15){
        static Scalar low['W'-'A'+1]={1.85, -4.36, 2.3, -2.3, -13.99, -10.33, -7.87,
                             -0.7, -0.15, -0.7, -2.37, -5.0, -1.2, -1.0, 11.8,
                             -2.10, 6.0, -5.1, -2.5, -2.5, -5.9, -5.9, -5.9};
        static Scalar high['W'-'A'+1]={0.58, -5.34, -0.01, 2.61, -12.24, -10.27, -8.67,
                              -0.54, -0.74, -0.77, -2.46, -5.0, -1.2, -1.3,
                              11.8, -3.25, 6.0, -5.1, -3.4, -3.4, -5.9, -5.9, -5.9};
        if (reaction > 'W' || reaction < 'A'){
            DUNE_THROW(Dune::InvalidStateException, "logK(): Invalid reaction index " << reaction);
        }
        Scalar logK0 = low[reaction-'A'];
        Scalar logK1 = high[reaction-'A'];
        Scalar T0 = 25+273.15;
        Scalar T1 = 110+273.15;
        Scalar m = (logK0-logK1)/(T0-T1);
        return m*(temperature - T0) + logK0;
    }
    // K(): returns the value of the reaction equilibrium constant, indexed
    //      by the reaction letter identifier.
    static Scalar K(char reaction, Scalar temperature=110+273.15){
        return alog(logK(reaction, temperature));
    }
    // gamma(): returns the activity coeficient for the component at a given
    //          ionic strength value.
    static Scalar gamma(int componentIdx, Scalar iS, Scalar T=383.15){
        if (iS < 0) return 1; // ideal case.
        // Return ideal activity if component is not in coded consideration
        if (useIdealGamma(componentIdx)) return 1.0;
        Scalar term1 = A(T) * pow(valence(componentIdx),2) * sqrt(iS);
        Scalar term2 = 1 + a(componentIdx) * B(T) * sqrt(iS);
        Scalar term3 = b(componentIdx)*iS;
        Scalar logGamma = term3 - term1/term2;
        // loggamma is log base 10
        // return value is always positive (it is a concentration modifier)
        TRACE("gamma(%s) = %le (loggamma=%lf iS=%lf)\n", name(componentIdx).c_str(),
                alog(logGamma), logGamma, iS);
        return alog(logGamma);
    }

    static void calculateGammas(chemicalModel_t *cm){
        for (int componentIdx=0; componentIdx<allComponents; componentIdx++){
            cm->gamma[componentIdx] = gamma(componentIdx, cm->iS, 373);
        }
    }

    // mess(): TRACE function for NAN identification.
    static void mess(Scalar *Ct, int i, const char *s){
        switch (i){
            case _Ca: DBG( "[_Ca] = %le %s\n", Ct[i], s); break;
            case _Mg: DBG( "[_Mg] = %le %s\n", Ct[i], s); break;
            case _SO4: DBG( "[_SO4] = %le %s\n", Ct[i], s); break;
            case _CO3: DBG( "[_CO3] = %le %s\n", Ct[i], s); break;
            case _H: DBG( "[_H] = %le %s\n", Ct[i], s); break;
            case _Na: DBG( "[_Na] = %le %s\n", Ct[i], s); break;
            case _OH: DBG( "[_OH] = %le %s\n", Ct[i], s); break;
            case _HCO3: DBG( "[_HCO3] = %le %s\n", Ct[i], s); break;
            case _Cl: DBG( "[_Cl] = %le %s\n", Ct[i], s); break;
            case _MgCl: DBG( "[_MgCl] = %le %s\n", Ct[i], s); break;
            case _CaCl: DBG( "[_CaCl] = %le %s\n", Ct[i], s); break;
            case _NaSO4: DBG( "[_NaSO4] = %le %s\n", Ct[i], s); break;
            default:
                DBG( "%le at item %d\n", Ct[i], i); break;
        }
        return;
    }
    // checkNAN(): debug function for NAN tracing.
    static void checkNAN(Scalar *Ct){
        for (int i=0; i<_allComponents; i++){
            TRACE("checkNAN ...element[%d] = %lf\n", i, Ct[i]);
            if (isnan(Ct[i])){
                mess(Ct, i, "NAN at element");
                DUNE_THROW(Dune::InvalidStateException, "checkNAN: nan at element "<< i << "\n");
            } 
        }
    }
    // solvePotentialOW(): returns the electrical potential at the
    //                     oil/brine interface.
    static Scalar solvePotentialOW(chemicalModel_t *cm, Scalar T=373){
        // For psiOW we prevously need to have obtained:
        //    Ct[_Ca] ... OK (initialConcentrations)
        //    Ct[_Mg] ... OK  (initialConcentrations), 
        //    Ct[_RCOO] (constant)
        Scalar iS = cm->iS;
        double *Ct = cm->molalities;
        Scalar Iv = iS * cm->density;
        
        Scalar C = (Constant::R*T) / (2 * Constant::F);
        Scalar C2 = (GAMMA(_Ca) * Ct[_Ca]/K('M')) + (GAMMA(_Mg) * Ct[_Mg]/K('N'));
        Scalar S1 = epsilonW_ * epsilonO_ * Iv * Constant::R * T;
        //Scalar S2 = 2 * pow(g,2) * pow(Constant::F,2); // simplified two expressions sqrt(pow(x,2))
        Scalar C3 = sqrt(S1/2) / Ct[_RCOO] / Constant::F;
        Scalar psiOW = C * (C2 - 1)/(C2 + C3);
        if (isnan(psiOW)){
            DBG("nan at psiOW\n");
        }
        return (psiOW);
    }
    // solvePotentialSW(): returns the electrical potential at the
    //                     solid/brine interface.
    static Scalar solvePotentialSW(chemicalModel_t *cm, Scalar T=373){
        // For psiSW we need:
        //    Ct[_vCaOH2] and Ct[_vCO3] (constants)
        //    Ct[_Ca] ... OK (initialConcentrations)
        //    Ct[_Mg] ... OK  (initialConcentrations)
        //    Ct[_SO4] ... OK
        //    Ct[_CO3] ... OK
        Scalar iS = cm->iS;
        double *Ct = cm->molalities;
        Scalar Iv = iS * cm->density;
        
        Scalar C = (Constant::R*T) / (2 * Constant::F);
        Scalar T1 = Ct[_vCaOH2] - Ct[_vCO3];
        Scalar T2a = ( (GAMMA(_Ca) * Ct[_Ca] / K('S')) +
                (GAMMA(_Mg) * Ct[_Mg] / K('T')) )  * Ct[_vCO3];
        Scalar T2b = ( (GAMMA(_SO4) * Ct[_SO4] / K('P')) +
                (GAMMA(_CO3) * Ct[_CO3] / K('Q')) ) * Ct[_vCaOH2];
        Scalar T2 = T2a - T2b;
        Scalar S1 = epsilonW_ * epsilonO_ * Iv * Constant::R * T;
        Scalar T3 = T2 + (sqrt(S1/2) / Constant::F);
        return C * (T1 + T2) / T3;
    }
    // solveCl(): solve the Cl- concentration from electroneutrality computation.
    static Scalar solveCl(chemicalModel_t *cm){
        Scalar iS = cm->iS;
        double *Ct = cm->molalities;
#ifdef SIMPLE_CACO3
        Scalar term1a =  2*(Ct[_Ca] - Ct[_CO3]);
        Scalar term1b =  Ct[_H] - Ct[_OH] - Ct[_HCO3];
        Scalar CaClterm = K('H')*Ct[_Ca]*GAMMA(_Ca)*GAMMA(_Cl)/GAMMA(_CaCl);
        Scalar MgClterm = 0;
#endif
#ifdef CACO3_CASO4
        Scalar term1a =  2*(Ct[_Ca] -  Ct[_SO4] - Ct[_CO3]);
        Scalar term1b =  Ct[_H] + Ct[_Na] - Ct[_OH] - Ct[_HCO3];
        Scalar CaClterm = K('H')*Ct[_Ca]*GAMMA(_Ca)*GAMMA(_Cl)/GAMMA(_CaCl);
        Scalar MgClterm = 0;
#endif
#ifdef CACO3_CASO4_MGCO3
        Scalar term1a =  2*(Ct[_Ca] + Ct[_Mg] -  Ct[_SO4] - Ct[_CO3]);
        Scalar term1b =  Ct[_H] + Ct[_Na] - Ct[_OH] - Ct[_HCO3] - Ct[_NaSO4];

        Scalar CaClterm = K('H')*Ct[_Ca]*GAMMA(_Ca)*GAMMA(_Cl)/GAMMA(_CaCl);
        Scalar MgClterm = K('I')*Ct[_Mg]*GAMMA(_Mg)*GAMMA(_Cl)/GAMMA(_MgCl);
#endif
        TRACE("Ions::solveCl: %lf/1-%lf-%lf = %lf\n", term1, CaClterm, MgClterm,
                term1 / (1 - CaClterm - MgClterm));
        if (isnan((term1a+term1b) / (1 - CaClterm - MgClterm))) {
            DBG( "Ions::solveCl: (%lf+%lf)/1-%lf-%lf = %lf\n", term1a, term1b,
                    CaClterm, MgClterm, (term1a+term1b) / (1 - CaClterm - MgClterm));
            exit(1);
        }
        if (CaClterm+MgClterm >= 1){
            DBG( "Ions::solveCl(): disabling CaClterm and MgClterm: %le, %le\n", CaClterm, MgClterm);
            //CaClterm = MgClterm = 0.0;
        }
        if ((term1a + term1b) / (1 - CaClterm - MgClterm) < 0){
            DBG( "Ions::solveCl(): This should not happen: Cl concentration cannot be less than zero.\n");
            return 0;
            //exit(1);
        }
        Scalar newCl = (term1a + term1b) / (1 - CaClterm - MgClterm);
        if (fabs((Ct[_Cl] - newCl)/Ct[_Cl]) > 0.01)  TRACE("Electroneutrality: Cl --> newCl: %le --> %le\n", Ct[_Cl], newCl);
        return newCl;
    }
    // solveEquilibrium(): fast reaction concentrations are solved by equilibrium.
    //                     Electrical potential taken into account for oil/brine,
    //                     solid/brine and oil/solid/brine interfaces.
    static void solveEquilibrium(chemicalModel_t *cm, Scalar T=373){
        double *Ct = cm->molalities;
        Scalar iS = cm->iS;
        // These concentrations are now activity modified.
        TRACE("solveEquilibrium ...\n");
        // Equilibrium relationships
        // Solve equilibrium (CO3 only by equilibrium):

        Ct[_OH] = K('E') / GAMMA(_OH) / GAMMA(_H) / Ct[_H];                          // 4.2
        Ct[_CO3] = K('A') * K('F') / GAMMA(_Ca) / Ct[_Ca] / GAMMA(_CO3); // 4.1
#if defined(CACO3_CASO4) || defined(CACO3_CASO4_MGCO3)
        Ct[_CaSO4] = GAMMA(_Ca)*GAMMA(_SO4)*Ct[_Ca]*Ct[_SO4]/GAMMA(_CaSO4)/K('D');   // 4.4
        Ct[_NaSO4] = GAMMA(_Na)*GAMMA(_SO4)*Ct[_Na]*Ct[_SO4]/GAMMA(_NaSO4)/K('J');   // 4.3
#endif
#if defined(CACO3_CASO4_MGCO3)
        Ct[_MgSO4] = GAMMA(_Mg)*GAMMA(_SO4)*Ct[_Mg]*Ct[_SO4]/GAMMA(_MgSO4)/K('K');
#endif
#ifdef BRINE_OIL_CHEMISTRY
        // brine/oil interface:
        Scalar psiOW = solvePotentialOW(cm); // Now calculate oil/brine psi values
        cm->psi[0] = psiOW;
        // Activity coeficients not available for _R compounds (we assume ideal)
        // Ct[_RCOO] is assumed constant and was set from input file at setInitialConcentrations()
        //   (otherise we would have 3 equations and 4 unknowns)
        Ct[_RCOOH] = exp(-psiOW*Constant::F/Constant::R/T)*GAMMA(_H)*Ct[_H]*Ct[_RCOO]/K('L'); 
        if (isnan(Ct[_RCOOH])){
                DBG("nan at RCOOH\n");
        }
        // 5.1
        Ct[_RCOOCa] = exp(-2*psiOW*Constant::F/Constant::R/T) * GAMMA(_Ca)*Ct[_Ca]*Ct[_RCOO]/K('M');  // 5.2
        Ct[_RCOOMg] = exp(-2*psiOW*Constant::F/Constant::R/T) * GAMMA(_Mg)*Ct[_Mg]*Ct[_RCOO]/K('N');  // 5.3
        TRACE("Ct[_RCOOH] = %le, Ct[_RCOOCa] = %le, Ct[_RCOOMg] = %le\n",
                Ct[_RCOOH], Ct[_RCOOCa], Ct[_RCOOMg]);
#endif
#ifdef BRINE_SOLID_CHEMISTRY
        // brine/solid interface:
        // Activity coeficients not available for ">" compounds (we assume ideal)
        Scalar psiSW = solvePotentialSW(cm); // Now calculate solid/brine psi values
        cm->psi[1] = psiSW;
        // brine/solid
        Ct[_vCaOH] = exp(psiSW*Constant::F/Constant::R/T) * Ct[_vCaOH2] / K('O') / GAMMA(_H) /Ct[_H]; // 5.3
        Ct[_vCaSO4] = exp(-2*psiSW*Constant::F/Constant::R/T) * GAMMA(_SO4)*Ct[_SO4]*Ct[_vCaOH2] / K('P'); // 5.4
        Ct[_vCaCO3] = exp(-2*psiSW*Constant::F/Constant::R/T) * GAMMA(_CO3)*Ct[_CO3]*Ct[_vCaOH2] / K('Q'); // 5.5
        Ct[_vCO3H] = exp(-psiSW*Constant::F/Constant::R/T) * GAMMA(_H)*Ct[_H]*Ct[_vCO3] / K('R');     // 5.6
        Ct[_vCO3Ca] = exp(-2*psiSW*Constant::F/Constant::R/T) * GAMMA(_Ca)*Ct[_Ca]*Ct[_vCO3] / K('S'); // 5.7
        Ct[_vCO3Mg] = exp(-2*psiSW*Constant::F/Constant::R/T) * GAMMA(_Mg)*Ct[_Mg]*Ct[_vCO3] / K('T'); // 5.8
#endif
#ifdef BRINE_OIL_SOLID_CHEMISTRY
#if !defined(BRINE_OIL_CHEMISTRY) || !defined(BRINE_SOLID_CHEMISTRY)
#error " Both BRINE_OIL_CHEMISTRY and BRINE_SOLID_CHEMISTRY must be defined for BRINE_OIL_SOLID_CHEMISTRY"
#endif
        // brine/solid/oil
        Ct[_vCaOH2RCOO] =
            exp(( psiSW - psiOW)*Constant::F/Constant::R/T)*Ct[_RCOO]*Ct[_vCaOH2] / K('U');
        Ct[_vCO3CaRCOO] =
            exp((-psiSW - psiOW)*Constant::F/Constant::R/T)*Ct[_RCOO]*Ct[_vCO3]*GAMMA(_Ca)*Ct[_Ca] / K('V');
        Ct[_vCO3MgRCOO] =
            exp((-psiSW - psiOW)*Constant::F/Constant::R/T)*Ct[_RCOO]*Ct[_vCO3]*GAMMA(_Mg)*Ct[_Mg] / K('W');
#endif
        return;
    }
    
    static void checkReactionRateConsistency(chemicalModel_t *cm){
        double *Ct = cm->molalities;
        Scalar iS = cm->iS;
        // Check for direction of precipitation reactions (by equilibrium)
        // CaCO3s
        bool debugStop = false;
        if (Ct[_Ca]*GAMMA(_Ca)*Ct[_HCO3]*GAMMA(_HCO3) / K('A') / Ct[_H] / GAMMA(_H)> 1){
            TRACE("CaCO3 will precipitate: Ct[_Ca]*Ct[_HCO3]/ Ct[_H] = %le > K_A = %le\n",
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_HCO3]*GAMMA(_HCO3)/ Ct[_H]/GAMMA(_H), K('A'));
            if (cm->rateC[CaCO3sIdx] < 0){
                DBG("inconsistent cm->rateC[CaCO3sIdx]: should be positive\n");
                debugStop = true;
            }
        } else {
            TRACE("CaCO3 will dissolve: Ct[_Ca]*Ct[_HCO3]/ Ct[_H] = %le < K_A = %le\n",
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_HCO3]*GAMMA(_HCO3)/ Ct[_H]/GAMMA(_H), K('A'));
            if (cm->rateC[CaCO3sIdx] > 0){
                DBG("inconsistent cm->rateC[CaCO3sIdx]: should be negative\n");
                debugStop = true;
           }
        }
        // CaSO4s
        if (Ct[_Ca]*GAMMA(_Ca)*Ct[_SO4]*GAMMA(_SO4)  / K('B')> 1){
            if (cm->rateC[CaSO4sIdx] < 0){
                TRACE("inconsistent cm->rateC[CaSO4sIdx]: should be positive\n");
                TRACE("CaSO4 will precipitate: Ct[_Ca]*Ct[_SO4] = %le < K_B = %le, cm->rateC[CaSO4sIdx]=%le\n"  ,
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_SO4]*GAMMA(_SO4), K('B'), cm->rateC[CaSO4sIdx]);
                DBG("inconsistent cm->rateC[CaSO4sIdx]: should be positive\n");
                debugStop = true;
            }
        } else {
            TRACE("CaSO4 will dissolve: Ct[_Ca]*Ct[_HCO3]/ Ct[_H] = %le < K_B = %le\n",
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_SO4]*GAMMA(_SO4), K('B'));
            if (cm->rateC[CaSO4sIdx] > 0){
                TRACE("CaSO4 will precipitate: Ct[_Ca]*Ct[_SO4] = %le < K_B = %le, cm->rateC[CaSO4sIdx]=%le\n",
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_SO4]*GAMMA(_SO4), K('B'), cm->rateC[CaSO4sIdx]);
                DBG("inconsistent cm->rateC[CaSO4sIdx]: should be negative\n");
                debugStop = true;
            }
        }
        // MgCO3s
        if (Ct[_Mg]*GAMMA(_Mg)*Ct[_HCO3]*GAMMA(_HCO3) / K('C') / Ct[_H] / GAMMA(_H)> 1){
            TRACE("MgCO3s will precipitate: Ct[_Mg]*Ct[_HCO3]/ Ct[_H] = %le > K_C = %le\n",
                    Ct[_Mg]*GAMMA(_Mg)*Ct[_HCO3]*GAMMA(_HCO3)/ Ct[_H]/GAMMA(_H), K('C'));
            if (cm->rateC[MgCO3sIdx] < 0){
                DBG("inconsistent cm->rateC[MgCO3sIdx]: should be positive\n");
                debugStop = true;
            }
        } else {
            TRACE("CaCO3 will dissolve: Ct[_Ca]*Ct[_HCO3]/ Ct[_H] = %le < K_A = %le\n",
                    Ct[_Ca]*GAMMA(_Ca)*Ct[_HCO3]*GAMMA(_HCO3)/ Ct[_H]/GAMMA(_H), K('A'));
            if (cm->rateC[MgCO3sIdx] > 0){
                DBG("inconsistent cm->rateC[MgCO3sIdx]: should be negative\n");
                debugStop = true;
            }
        }
#ifdef DEBUG
        if (debugStop) DUNE_THROW(Dune::InvalidStateException, "checkReactionRateConsistency");
#endif
    }

    // solveDependentRates(): Solve reaction rates which depend on slow precipitation
    static void solveDependentRates(chemicalModel_t *cm){
        double *R = cm->rateC;
        double *Ct = cm->molalities;
        // Reaccion rates for precipitates consider that precipitate is
        // leaving the liquid phase. Thus, precipitation would imply a negative
        // rate. Thus the ions that are consumed to form the precipitate would
        // also have a negative rate. This explains the reason for the sign in
        // concentrations of Ca++, Mg++ and SO4--
        //
        // Dependent rates:
        // H+ produced with precipitate CaCO3 and produced with precipitate MgCO3.
        // HCO3- + Ca++ ---> H+ + CaCO3s
        // HCO3- + Mg++ ---> H+ + MgCO3s
        R[HIdx] = R[CaCO3sIdx] + R[MgCO3sIdx];         
        // HCO3- consumed produced with precipitate CaCO3 and consumed with precipitate MgCO3.
        // HCO3- + Ca++ ---> H+ + CaCO3s
        // HCO3- + Mg++ ---> H+ + MgCO3s
        R[HCO3Idx] = -R[CaCO3sIdx] - R[MgCO3sIdx]; 
        // Ca++ consumed with precipitate CaCO3 and consumed with precipitate CaSO4
        // HCO3- + Ca++ ---> H+ + CaCO3s
        // Ca++ + SO4-- ---> CaSO4s
        R[CaIdx] = -R[CaCO3sIdx] - R[CaSO4sIdx]; // (stoichiometric rate)
        //R[CaIdx] = R[CaCO3sIdx] + R[CaSO4sIdx]; // eq. 55 (confusion rate)
        // SO4-- consumed with precipitate CaSO4
        // Ca++ + SO4-- ---> CaSO4s
        R[SO4Idx] = -R[CaSO4sIdx];   // (stoichiometric rate)
        //R[SO4Idx] = R[CaSO4sIdx];  // eq. 55 (confusion rate)
        // Mg++ consumed with precipitate MgCO3
        // HCO3- + Mg++ ---> H+ + MgCO3s
        R[MgIdx] = -R[MgCO3sIdx];  // (stoichiometric rate)
        //R[MgIdx] = R[MgCO3sIdx]; // eq. 55 (confusion rate)

 
        TRACE("[H+]=%le, [Ca++]=%le, [HCO3-]=%le, [SO4++] = %le\n",
            Ct[_H], Ct[_Ca], Ct[_HCO3], Ct[_SO4]);
        // No reaction rate term for these components:
        R[NaIdx] = 0;
        R[brinePhaseIdx] = 0;
        R[oilPhaseIdx] = 0;
        R[ClIdx] = 0; // Cl- is solved by electroneutrality

    }
    static void solveR(chemicalModel_t *cm){
        Scalar iS = cm->iS;
        double *Ct = cm->molalities;
        // k1 is in mol/lt/s, but we are working with molalities...
        //    convert to mol/kg/s:
        //    FIXME(maybe): use actual brine density from partial molar volumes
        //    see private:ionicStrength()!!!!
        //
        Scalar QA = ((Ct[_Ca]*GAMMA(_Ca)*Ct[_HCO3]*GAMMA(_HCO3)) / K('A') / Ct[_H]) / GAMMA(_H);
        Scalar QB = 1.0;
        Scalar QC = 1.0;
        Scalar R_A = 0;
        Scalar R_B = 0;
        Scalar R_C = 0; 

        R_A = -k1('A') *(QA - 1.0); // Esta es la famosa R_CaCO3 de la ecuacion 50
        // Pero... la derivada del cambio de densidad del CaCO3 es con signo contrario.
        //         considerando la fase sólida! Pero esa no nos concierne por ahora.
        
#if defined(CACO3_CASO4) || defined(CACO3_CASO4_MGCO3) // Include CaSO4
        QB = (Ct[_Ca]*GAMMA(_Ca)*Ct[_SO4]*GAMMA(_SO4)) / K('B');
        R_B = -k1('B') *(QB - 1.0);
#endif
#ifdef CACO3_CASO4_MGCO3 // Include  MgCO3
        QC = (Ct[_Mg]*GAMMA(_Mg)*Ct[_HCO3]*GAMMA(_HCO3)) / K('C') / Ct[_H] / GAMMA(_H);
        R_C = -k1('C') *(QC - 1.0);
#endif

	// These will be in original mol/m3/s for VTK.
        // Units are specified in input file as comment.
        cm->R[0] = R_A;
	cm->R[1] = R_B;
	cm->R[2] = R_C;
        //  Internally we will work only with molality so rates must be
        //  converted to mol/kg/s
        //  Conversion to molal velocity (mol/kg/s) 
        //  If liter density = 1, we basically have the same thing
        //  We write out the formula so we can change brineDensity()
        //  to input density later on.
        //  brineDensity() is set to episode input density
        //  in lswf-problem.hh
        //
        //  brineDensity() comes in DuMuX units, i.e., kg/m3
        // Conversion from mol/m3/s --> mol/kg/s
        R_A /= brineDensity();
        R_B /= brineDensity();
        R_C /= brineDensity();

        // Independent rates: (positive values indicate production)
        // Production in solid phase, nothinTRACEg is in aqueous phase
        cm->rateC[CaCO3sIdx] = -R_A;
        cm->rateC[CaSO4sIdx] = -R_B;
        cm->rateC[MgCO3sIdx] = -R_C;

#ifndef SIMPLIFIED
        // Reaction rates cannot contradict equilibrium equation:
        checkReactionRateConsistency(cm);
#endif
        // Dependent rates:
        solveDependentRates(cm);
        cm->Q[0] = QA;
        cm->Q[1] = QB;
        cm->Q[2] = QC;
        TRACE("Ca=%le SO4=%le 1-x, x= %le --> -cm->rateC[CaIdx] is: %le\n",Ct[_Ca], Ct[_SO4], (Ct[_Ca]*Ct[_SO4]) / K('B'),  R[CaIdx]);
    }

    static void adjustVanishedReactant(Scalar timeStepSize, chemicalModel_t *cm){
        double *Ct = cm->molalities;
        // If any concentration is below zero, we must adjust R.
        bool zeroed;
        do {
            zeroed = false;
            // Reaction rates do not use activity coeficients (equilibrium parameter)
            if ( Ct[_HCO3] > 0 &&  (cm->rateC[HCO3Idx] * timeStepSize) + Ct[_HCO3] < 0){ 
                DBG("*** HCO3 is zeroed....(precipitating CaCO3 or MgCO3)\n");
                // Slow down the dissolution reaction:
                cm->rateC[CaCO3sIdx] /= 10.0;
                cm->rateC[MgCO3sIdx] /= 10.0;
                // Recalculate dependent reaction rates:
                solveDependentRates(cm);
                // continue in the loop:
                zeroed = true;
                continue;
            }
            if ( Ct[_Ca] > 0 &&  (cm->rateC[CaIdx] * timeStepSize) + Ct[_Ca]< 0){ 
                DBG("*** Ca++ is zeroed...(precipitating CaCO3 or CaSO4)\n");
                cm->rateC[CaCO3sIdx] /= 10.0;
                cm->rateC[CaSO4sIdx] /= 10.0;
                solveDependentRates(cm);
                zeroed = true;
                continue;
            }
            if ( Ct[_Mg] > 0 && (cm->rateC[MgIdx] * timeStepSize) + Ct[_Mg] < 0){ 
                DBG("*** Mg is zeroed...(precipitating MgCO3) cm->rateC[MgIdx]%le, cm->rateC[MgIdx] * timeStepSize = %le, Ct[_Mg] = %le\n",
                        cm->rateC[MgIdx],  cm->rateC[MgIdx] * timeStepSize, Ct[_Mg]);
                cm->rateC[MgCO3sIdx] /= 10.0;
                solveDependentRates(cm);
                zeroed = true;
                exit(1);
                continue;
            }
            if ( Ct[_H] > 0 &&  (cm->rateC[HIdx] * timeStepSize) + Ct[_H] < 0){ 
                DBG("*** H is zeroed...Automatically reducing rates R_CaCO3: %le --> %le and R_MgCO3: %le --> %le\n",
                      cm->rateC[CaCO3sIdx], cm->rateC[CaCO3sIdx]/10.0,  cm->rateC[MgCO3sIdx],  cm->rateC[MgCO3sIdx]/10.0);
                cm->rateC[CaCO3sIdx] /= 10.0;
                cm->rateC[MgCO3sIdx] /= 10.0;
                solveDependentRates(cm);
                zeroed = true;
                continue;
                //exit(1);
            }
            if ( Ct[_CaCO3s] > 0 && (cm->rateC[CaCO3sIdx] * timeStepSize) + Ct[_CaCO3s] < 0){ 
                DBG("*** _CaCO3 is zeroed...R_CaCO3=%le, R_CaCO3 * timeStepSize = %le, Ct[_CaCO3] = %le\n",
                        cm->rateC[CaCO3sIdx],  cm->rateC[CaCO3sIdx] * timeStepSize, Ct[_CaCO3s]);
                cm->rateC[CaCO3sIdx] /= 10;
                zeroed = true;
                exit(1);
                continue;
            }
            if ( Ct[_CaSO4s] > 0 && (cm->rateC[CaSO4sIdx] * timeStepSize) + Ct[_CaSO4s] < 0){ 
                DBG("*** _CaSO4s is zeroed...cm->rateC[CaSO4sIdx]=%le, cm->rateC[CaSO4sIdx] * timeStepSize = %le, Ct[_CaSO4s] = %le\n",
                        cm->rateC[CaSO4sIdx],  cm->rateC[CaSO4sIdx] * timeStepSize, Ct[_CaSO4s]);
                cm->rateC[CaSO4sIdx] /= 10;
                solveDependentRates(cm);
                zeroed = true;
                exit(1);
                continue;
            }
            // This is always false in SIMPLE_CACO3 compilation (directives are ok anyways)
            if ( Ct[_SO4] > 0 && (cm->rateC[SO4Idx] * timeStepSize) + Ct[_SO4] < 0){ 
                DBG("*** SO4 is zeroed...(precipitating CaSO4) cm->rateC[SO4Idx]=%le, cm->rateC[SO4Idx] * timeStepSize = %le, Ct[_SO4] = %le\n",
                        cm->rateC[SO4Idx],  cm->rateC[SO4Idx] * timeStepSize, Ct[_SO4]);
                cm->rateC[CaSO4sIdx] /= 10;
                solveDependentRates(cm);
                zeroed = true;
                //exit(1);
                continue;
            }
        } while (zeroed);
    }

    // Check all transport component concentrations positive.
    static bool checkAllPositive(chemicalModel_t *cm){
        bool result = true;
        double *Ct = cm->molalities;
        static int warned=0;
        for (int compIdx=numMajorComponents; compIdx < numComponents; compIdx++) {
            if (Ct[compIdx] < 0){
                    DBG("*******************************checkEquilibrium*******************************\n");
                    DBG("*******   transport provides component %s concentration negative!   ********\n", name(compIdx).c_str());
                    DBG("*******   Chemical model will use a small concentration instead.    ********\n", name(compIdx).c_str());
                    DBG("*************************************************************************\n");
                    warned = true;
                    result = false;
#ifndef DEBUG
                Ct[compIdx] = 0.0001;
#endif
            }
        }
        return result;
    }

    // Second equilibrium may or may not use solved ODE's
    static bool checkEquilibrium(chemicalModel_t *cm){
        bool result = checkAllPositive(cm);
        double *Ct = cm->molalities;
        double iS = cm->iS;


#ifdef DEBUG
        Scalar eq[numComponents];
        eq[_Ca] = K('A')*Ct[_H] * GAMMA(_H) / Ct[_HCO3] / GAMMA(_HCO3) / GAMMA(_Ca);
        eq[_H]  = K('F')*Ct[_HCO3] * GAMMA(_HCO3) / Ct[_CO3] / GAMMA(_CO3) / GAMMA(_H);
        eq[_HCO3] = K('A')*Ct[_H] * GAMMA(_H) / Ct[_Ca] / GAMMA(_Ca)/ GAMMA(_HCO3);
        eq[_SO4] = K('B') / Ct[_Ca] / GAMMA(_Ca) / GAMMA(_SO4);
        eq[_Mg] =  K('C')*Ct[_H] * GAMMA(_H) / Ct[_HCO3] / GAMMA(_HCO3) / GAMMA(_Mg);

        // negative concentrations are not valid.
                 
        TRACE("log/K_A: %le/%le, log/K_F; %le/%le; log/K_B: %le/%le; log/K_C: %le/%le\n",
                    logK('A'),K('A'), logK('F'), K('F'), logK('B'), K('B'), logK('C'), K('C'));
   
        if (!result){
            DBG("checkEquilibrium: ionic strength=%le\n", cm->iS);
            DBG("Kinetic/Equilibrium/gamma  values: \n");

            for (int compIdx=numMajorComponents; compIdx < numComponents; compIdx++) {
                DBG("%s: %le/%le/%le\n",
                        name(compIdx).c_str(), Ct[compIdx], eq[compIdx], cm->gamma[compIdx]);
                DBG("reaction rate: %le\n", cm->rateC[compIdx]);
            }
            
            DBG("DEBUG is enabled. Will not continue.\n");
            exit(1);
        }
#endif
        return result;
    }

    // solveODEs(): solves ordinary differential equations which define concentration change in
    //              time interval (depends on reaction rate constants).
    static void solveODEs(Scalar timeStepSize, chemicalModel_t *cm){
        double *Ct = cm->molalities;
        double iS = cm->iS;
        TRACE("solveODEs timestep %lf\n", timeStepSize);
        // Solve ODEs for concentrations at the end of the time interval.
        // Solids:
        // Precipitated solids no longer active in in aqueous phase.
        Ct[_CaCO3s] = (-cm->rateC[CaCO3sIdx]* timeStepSize) + Ct[_CaCO3s]; //  3.1
        Ct[_MgCO3s] = (-cm->rateC[MgCO3sIdx]* timeStepSize) + Ct[_MgCO3s]; //  3.1.2
        Ct[_CaSO4s] = (-cm->rateC[CaSO4sIdx] * timeStepSize) + Ct[_CaSO4s]; //  3.2

        Ct[_SO4] = (cm->rateC[SO4Idx] * timeStepSize) + Ct[_SO4];
        Ct[_Mg] = (cm->rateC[MgIdx] * timeStepSize) + Ct[_Mg];
        // Solutes
        DBG("solveODEs concentration/activity: Ca=%le/%le H=%le/%le, HCO3=%le/%le, SO4=%le/%le\n",
                Ct[_Ca], GAMMA(_Ca), Ct[_H], GAMMA(_H), Ct[_HCO3], GAMMA(_HCO3), Ct[_SO4], GAMMA(_SO4));
        adjustVanishedReactant(timeStepSize, cm);
        // R values are now adjusted to zero concentrations
        // Integration proceeds from concentration, no GAMMA here
        // since no equilibrium constants are involved.
        Ct[_H] = (cm->rateC[HIdx] * timeStepSize) + Ct[_H];          // 3.3
        Ct[_HCO3] = (cm->rateC[HCO3Idx] * timeStepSize) + Ct[_HCO3]; // 3.4
        Ct[_Ca] = (cm->rateC[CaIdx] * timeStepSize) + Ct[_Ca];       // 3.5
        Ct[_SO4] = (cm->rateC[SO4Idx] * timeStepSize) + Ct[_SO4];
        Ct[_Mg] = (cm->rateC[MgIdx] * timeStepSize) + Ct[_Mg];
        // XXX: Not significant yet (while considering constant solid concentrations)
        Ct[_CaCO3s] = cm->rateC[CaCO3sIdx]* timeStepSize + Ct[_CaCO3s]; //  3.1
        Ct[_MgCO3s] = cm->rateC[MgCO3sIdx]* timeStepSize + Ct[_MgCO3s]; //  3.1.2
        Ct[_CaSO4s] = cm->rateC[CaSO4sIdx] * timeStepSize + Ct[_CaSO4s]; //  3.2
         DBG("and then... solveODEs concentration/activity: Ca=%le/%le H=%le/%le, HCO3=%le/%le, SO4=%le/%le\n",
                Ct[_Ca], GAMMA(_Ca), Ct[_H], GAMMA(_H), Ct[_HCO3], GAMMA(_HCO3), Ct[_SO4], GAMMA(_SO4));
           

        return;
    }
    // solveIonicStrength(): internal iterative function to solve ionic strength.
    static Scalar solveIonicStrength(chemicalModel_t *cm){
        // Now solve all concentrations and recalculate iS
        // for timestep concentration.
        Scalar newIS;
        Scalar criteria;
        int count=0;
        do {
            solveEquilibrium(cm); // equilibriums (fast reactions, depends on iS)
#ifdef SOLVE_CL
            cm->molalities[_Cl] = solveCl(cm); // by electroneutrality
#endif
            newIS=getIonicStrength(cm);
            criteria = fabs((newIS - cm->iS)/cm->iS)*100;
            cm->iS = newIS;
            count++;
            TRACE("solveIonicStrength(): newStrength at iteration %d (%s): %lf criteria = %lf\n",
                    count, iS<0?"ideal":"activities", iS, criteria);

        } while (fabs(criteria > 0.1)); // changes less than 0.1%
        TRACE("** intermediate iS=%le\n", newIS);
        return newIS;
    }

    static Scalar solveReactionRates(Scalar timeStepSize, chemicalModel_t *cm){
        // Get reaction rates:
        // To solve R we need Ct values.
        // We input initial concentrations for Ct
        Scalar newIS;
        Scalar criteria;
        int count=0;
        double molalities[allComponents];
        memcpy(molalities, cm->molalities, allComponents*sizeof(double));
        do {
            count++;
            memcpy(cm->molalities, molalities, allComponents*sizeof(double));
            solveR(cm);
#ifndef SOLVE_ODES
# warning "not solving Ct from ODEs"
#else
            solveODEs(timeStepSize, cm);
#endif
            if (!checkEquilibrium(cm)){
            /*    DBG("!checkEquilibrium on pass %d\n", count);
                newIS = solveIonicStrength(cm);
                criteria = fabs((newIS - cm->iS)/cm->iS)*100;
                DBG("old iS=%lf, new iS=%lf\n", cm->iS, newIS);
                cm->iS = newIS;
                // recalculate gammas with new iS
                calculateGammas(cm);
                DBG(" solveReactionRates: Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                    cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);


                exit(1);*/
            }
#if 10
#ifdef USE_ACTIVITY_COEFICIENTS

            newIS = solveIonicStrength(cm);
            criteria = fabs((newIS - cm->iS)/cm->iS)*100;
            cm->iS = newIS;
            // recalculate gammas with new iS
            calculateGammas(cm);
            TRACE(" solveReactionRates: Ca--> gamma=%lf, [Ca]=%lf QA=%lf QB=%lf QC=%lf\n",
                cm->gamma[_Ca], cm->molalities[_Ca], cm->Q[0], cm->Q[1], cm->Q[2]);
            TRACE("solveReactionRates()2: newIStrength at iteration %d (%s): %lf criteria = %lf\n",
                    count, cm->iS<0?"ideal":"activities", cm->iS, criteria);
#else
            newIS = cm->iS; // does not change in ideal case (activities are 1.0)
            break;
#endif
#endif
        } while (fabs(criteria > 0.1)); // changes less than 0.1%
        return newIS;
    }


public:
    static std::string componentName(int compIdx)
    {
        switch (compIdx)
        {
            case brineIdx: return Water::name();
            //case oilIdx: return MyOil::name();
            default: return Ions::name(compIdx);
        }
        DUNE_THROW(Dune::InvalidStateException, "componentName(): Invalid component index " << compIdx);
    }

};

}// end namespace Dumux
#endif
