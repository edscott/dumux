#ifndef DUMUX_CASO4_HH
#define DUMUX_CASO4_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class CaSO4suspended : public Base<Scalar, CaSO4suspended<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 0;}
    static Scalar a(void){return 3;}
    static Scalar b(void){return 0;}
    static std::string name() { return "CaSO4"; }
    static Scalar molarMass() { return 40.078e-3 + 96.062e-3; }
    static Scalar ionicRadius() { return 100e-12 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_MGSO4_HH
#define DUMUX_MGSO4_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class MgSO4 : public Base<Scalar, MgSO4<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 0;}
    static Scalar a(void){return 3;}
    static Scalar b(void){return 0;}
    static std::string name() { return "MgSO4"; }
    static Scalar molarMass() { return 24.3e-3 + 96.062e-3; }
    static Scalar ionicRadius() { return 100e-12 ; }
};

} // end namespace
} // end namespace

#endif

