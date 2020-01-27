#ifndef DUMUX_CL_ANION_HH
#define DUMUX_CL_ANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>
// Na 22.990, Mg 24.305, Ca 40.078, H 1.008, O 15.999, C 12.011, S 32.066, Cl 35.453

namespace Dumux
{
namespace Components{
template <class Scalar>
class ClAnion : public Base<Scalar, ClAnion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return false;}
    static int valence(void){return -1;}
    static Scalar a(void){return 3.5e-10;}
    static Scalar b(void){return 0.015;}
    static std::string name() { return "Cl-"; }
    static Scalar molarMass() { return 35.453e-3 ; }
    /*!
     * \brief The ionic radius in \f$\mathrm{[m]}\f$.
     * this is a bit tricky. Bicarbonate ion has 3 oxygen atoms
     * distributed along sp orbitals (tetrahedral orientation)
     * in covalent bond, 
     * C=77 pm, O=73 pm H=38. 77+73+38=188 pm
     */
    static Scalar ionicRadius() { return 181-12 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_CARBONATE_ANION_HH
#define DUMUX_CARBONATE_ANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>
// Na 22.990, Mg 24.305, Ca 40.078, H 1.008, O 15.999, C 12.011, S 32.066, Cl 35.453

namespace Dumux
{
namespace Components{
template <class Scalar>
class CO3Anion : public Base<Scalar, CO3Anion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return false;}
    static int valence(void){return -2;}
    static Scalar a(void){return 5.4e-10;}
    static Scalar b(void){return 0;}
    static std::string name() { return "CO3--"; }
    static Scalar molarMass() { return 60.015e-3 ; }
    /*!
     * \brief The ionic radius in \f$\mathrm{[m]}\f$.
     * this is a bit tricky. Bicarbonate ion has 3 oxygen atoms
     * distributed along sp orbitals (tetrahedral orientation)
     * in covalent bond, XXX this is probably wrong
     * C=77 pm, O=73 pm H=38. 77+73+38=188 pm
     */
    static Scalar ionicRadius() { return 188-12 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_BICARBONATE_ANION_HH
#define DUMUX_BICARBONATE_ANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>
// Na 22.990, Mg 24.305, Ca 40.078, H 1.008, O 15.999, C 12.011, S 32.066, Cl 35.453

namespace Dumux
{
namespace Components{
template <class Scalar>
class HCO3Anion : public Base<Scalar, HCO3Anion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return false;}
    static int valence(void){return -1;}
    static Scalar a(void){return 5.4e-10;}
    static Scalar b(void){return 0;}
    static std::string name() { return "HCO3-"; }
    static Scalar molarMass() { return 61.016e-3 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_NASO4_ANION_HH
#define DUMUX_NASO4_ANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components {
template <class Scalar>
class NaSO4Anion : public Base<Scalar, NaSO4Anion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return -1;}
    static Scalar a(void){return 5.4e-10;}
    static Scalar b(void){return 0;}
    static std::string name() { return "NaSO4-"; }
    static Scalar molarMass() { return 22.990e-3 + 96.062e-3; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_OHANION_HH
#define DUMUX_OHANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components {
template <class Scalar>
class OHAnion : public Base<Scalar, OHAnion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return false;}
    static int valence(void){return -1;}
    static Scalar a(void){return 10.65e-10;}
    static Scalar b(void){return 0.21;}
    static std::string name() { return "OH-"; }
    static Scalar molarMass() { return 17.007e-3 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_SULPHATE_ANION_HH
#define DUMUX_SULPHATE_ANION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components {
template <class Scalar>
class SO4Anion : public Base<Scalar, SO4Anion<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return false;}
    static int valence(void){return -2;}
    static Scalar a(void){return 5e-10;}
    static Scalar b(void){return -0.04;}
    static std::string name() { return "SO4--"; }
    static Scalar molarMass() { return 96.062e-3 ; }
    /*!
     * \brief The ionic radius in \f$\mathrm{[m]}\f$.
     * this is a bit tricky. Sulfate ion has 4 oxygen atoms
     * distributed along p orbitals in covalent bond, and
     * two extra electrons in the remaining p orbitals.
     * The longest axis would be O-S-O. Covalent radius for 
     * S=102 pm, O=73 pm. 102+2*73 = 248 pm 
     * this is probably off    */
    static Scalar ionicRadius() { return 248e-12 ; }
};
} // end namespace
} // end namespace

#endif

