#ifndef LOCAL_INDEXES
#define LOCAL_INDEXES


namespace Dumux::FluidSystems
{
    /****************************************
     * Fluid phase related static parameters
     ****************************************/
    static const int numPhases  = 2; // liquid phases
    static const int numMinorComponents  = 1; // components in water

    /****************************************
     * Component related static parameters
     ****************************************/
    static const int brineIdx   = 0;
    static const int oilIdx     = 1;
    static const int NaClIdx    = 2;

    /*******************************************
     * Component/phase related static parameters
     *******************************************/
    static const int numMajorComponents = numPhases;// brine, oil
    static const int numComponents      = numPhases + numMinorComponents;

    /*************************
     * Code related synonyms
     *************************/
    static const int lPhaseIdx     = brineIdx; // model compatibility
    static const int wPhaseIdx     = brineIdx; // index of the wetting phase
    static const int brinePhaseIdx = brineIdx; // index of the brine phase
    static const int wCompIdx      = brineIdx;
    static const int gPhaseIdx     = oilIdx; // model compatibility
    static const int nPhaseIdx     = oilIdx; // index of the non-wetting phase
    static const int oilPhaseIdx   = oilIdx; // index of the oil phase
    static const int nCompIdx      = oilIdx;

}


#endif
