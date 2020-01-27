
#ifndef DUMUX_CACATION_HH
#define DUMUX_CACATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class CaCation : public Base<Scalar, CaCation<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 2;}
    static Scalar a(void){return 5e-10;}
    static Scalar b(void){return 0.165;}
    static std::string name() { return "Ca++"; }
    static Scalar molarMass() { return 40.078e-3 ; }
    static Scalar ionicRadius() { return 100e-12 ; }
};

} // end namespace
} // end namespace

#endif


#ifndef DUMUX_CACLCATION_HH
#define DUMUX_CACLCATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class CaClCation : public Base<Scalar, CaClCation<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 1;}
    static Scalar a(void){return 4e-10;}
    static Scalar b(void){return 0;}
    static std::string name() { return "CaCl+"; }
    static Scalar molarMass() { return 40.078e-3 + 35.453e-3; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_HCATION_HH
#define DUMUX_HCATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class HCation : public Base<Scalar, HCation<Scalar> >
{
public:
    static bool isAcid(void){ return true;}
    static bool isSalt(void){ return false;}
    static int valence(void){return 1;}
    static Scalar a(void){return 4.78e-10;}
    static Scalar b(void){return 0.24;}
    static std::string name() { return "H+"; }
    static Scalar molarMass() { return 1.008e-3 ; }
    static Scalar ionicRadius() { return 10e-12 ; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_MG_CATION_HH
#define DUMUX_MG_CATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class MgCation : public Base<Scalar, MgCation<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 2;}
    static Scalar a(void){return 5.5e-10;}
    static Scalar b(void){return 0.2;}
    static std::string name() { return "Mg++"; }
    static Scalar molarMass() { return 24.302e-3 ; }
    static Scalar ionicRadius() { return 72e-12 ; }
    static Scalar liquidDiffCoeff(Scalar temperature, Scalar pressure) { return 2e-9; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_MGCL_CATION_HH
#define DUMUX_MGCL_CATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components{
template <class Scalar>
class MgClCation : public Base<Scalar, MgClCation<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 1;}
    static Scalar a(void){return 4e-10;}
    static Scalar b(void){return 0;}
    static std::string name() { return "MgCl+"; }
    static Scalar molarMass() { return 24.302e-3 + 35.453e-3; }
};

} // end namespace
} // end namespace

#endif

#ifndef DUMUX_NA_CATION_HH
#define DUMUX_NA_CATION_HH

#include <dumux/common/exceptions.hh>

#include <cmath>
#include <iostream>


namespace Dumux
{
namespace Components {
template <class Scalar>
class NaCation : public Base<Scalar, NaCation<Scalar> >
{
public:
    static bool isAcid(void){ return false;}
    static bool isSalt(void){ return true;}
    static int valence(void){return 1;}
    static Scalar a(void){return 4e-10;}
    static Scalar b(void){return 0.075;}
    static std::string name() { return "Na+"; }
    static Scalar ionicRadius() { return 102e-12 ; }
    static Scalar molarMass() { return 22.990e-3 ; }
    static Scalar liquidDiffCoeff(Scalar temperature, Scalar pressure) { return 2e-9; }
};

} // end namespace
} // end namespace

#endif

