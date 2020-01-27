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
 * \ingroup Fluidmatrixinteractions
 * \brief Implementation of the capillary pressure and
 * relative permeability <-> saturation relations according to Brooks and Corey.
 *
 */
#ifndef DUMUX_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_HH
#define DUMUX_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_HH

#include "modifiedbrookscoreyfiparams.hh"

#include <algorithm>
#include <cmath>

namespace Dumux
{
/*!
 * \ingroup Fluidmatrixinteractions
 *
 * \brief Implementation of the Brooks-Corey capillary pressure <->
 *        saturation relation. This class bundles the "raw" curves
 *        as static members and doesn't concern itself converting
 *        absolute to effective saturations and vice versa.
 *
 * For general info: EffToAbsLaw
 *
 *\see BrooksCoreyParams
 */
template <class ScalarT, class ParamsT = ModifiedBrooksCoreyFIParams<ScalarT> >
class ModifiedBrooksCoreyFI
{
public:
    using Params = ParamsT;
    using Scalar = typename Params::Scalar;

    /*!
     * \brief The capillary pressure-saturation curve according to Brooks & Corey.
     *
     * The Brooks-Corey empirical  capillary pressure <-> saturation
     * function is given by
     *
     *  \f$\mathrm{ p_C = p_e\overline{S}_w^{-1/\lambda}
     *  }\f$
     *
     * \param swe Effective saturation of the wetting phase \f$\mathrm{[\overline{S}_w]}\f$
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen,
                        and then the params container is constructed accordingly. Afterwards the values are set there, too.
     * \return Capillary pressure calculated by Brooks & Corey constitutive relation.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar pc(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        const Scalar pc_HS = params.pe_HS()*pow(swe, -1.0/params.lambda_HS());

        if (params.LS() == params.HS())
        {
            return pc_HS;
        }

        const Scalar pc_LS = params.pe_LS()*pow(swe, -1.0/params.lambda_LS());
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());

        return nS*pc_HS + (1-nS)*pc_LS;
        // return params.pe()*pow(swe, -1.0/params.lambda());
    }

    /*!
     * \brief The saturation-capillary pressure curve according to Brooks & Corey.
     *
     * This is the inverse of the capillary pressure-saturation curve:
     * \f$\mathrm{ \overline{S}_w = (\frac{p_C}{p_e})^{-\lambda}}\f$
     *
     * \param pc Capillary pressure \f$\mathrm{[p_C]}\f$  in \f$\mathrm{[Pa]}\f$.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen, and then the params container
     *                  is constructed accordingly. Afterwards the values are set there, too.
     * \return Effective wetting phase saturation calculated as inverse of BrooksCorey constitutive relation.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar sw(const Params& params, Scalar pc)
    {
        using std::pow;
        using std::max;

        pc = max(pc, 0.0); // the equation below is undefined for negative pcs

        const Scalar sw_HS = pow(pc/params.pe_HS(), -params.lambda_HS());
        if (params.LS() == params.HS())
        {
            return sw_HS;
        }
        const Scalar sw_LS = pow(pc/params.pe_LS(), -params.lambda_LS());

        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());

        return nS*sw_HS + (1-nS)*sw_LS;

        // return pow(pc/params.pe(), -params.lambda());
    }

    /*!
     * \brief The capillary pressure at Swe = 1.0 also called end point capillary pressure
     *
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen, and then the params container
     *                  is constructed accordingly. Afterwards the values are set there, too.
     */
    static Scalar endPointPc(const Params& params)
    {
      if (params.LS() == params.HS())
      {
          return params.pe_HS();
      }
      const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());
      return nS*params.pe_HS() + (1-nS)*params.pe_LS();
      //return params.pe();
    }

    /*!
     * \brief The partial derivative of the capillary
     *        pressure w.r.t. the effective saturation according to Brooks & Corey.
     *
     * This is equivalent to
     * \f$\mathrm{\frac{\partial p_C}{\partial \overline{S}_w} =
     * -\frac{p_e}{\lambda} \overline{S}_w^{-1/\lambda - 1}
     * }\f$
     *
     * \param swe Effective saturation of the wetting phase \f$\mathrm{[\overline{S}_w]}\f$
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen, and then the params container
     *                  is constructed accordingly. Afterwards the values are set there, too.
     * \return Partial derivative of \f$\mathrm{[p_c]}\f$ w.r.t. effective saturation according to Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar dpc_dswe(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        const Scalar dpc_dswe_HS = - params.pe_HS()/params.lambda_HS() * pow(swe, -1/params.lambda_HS() - 1);
        if (params.LS() == params.HS())
        {
            return dpc_dswe_HS;
        }

        const Scalar dpc_dswe_LS = - params.pe_LS()/params.lambda_LS() * pow(swe, -1/params.lambda_LS() - 1);
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());
        return nS*dpc_dswe_HS + (1-nS)*dpc_dswe_LS;

        //return - params.pe()/params.lambda() * pow(swe, -1/params.lambda() - 1);
    }

    /*!
     * \brief The partial derivative of the effective
     *        saturation w.r.t. the capillary pressure according to Brooks & Corey.
     *
     * \param pc Capillary pressure \f$\mathrm{[p_c]}\f$ in \f$\mathrm{[Pa]}\f$.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen, and then the params container
     *                  is constructed accordingly. Afterwards the values are set there, too.
     * \return Partial derivative of effective saturation w.r.t. \f$\mathrm{[p_c]}\f$ according to Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar dswe_dpc(const Params& params, Scalar pc)
    {
        using std::pow;
        using std::max;

        pc = max(pc, 0.0); // the equation below is undefined for negative pcs

        const Scalar dswe_dpc_HS = -params.lambda_HS()/params.pe_HS() * pow(pc/params.pe_HS(), - params.lambda_HS() - 1);

        if (params.LS() == params.HS())
        {
            return dswe_dpc_HS;
        }

        const Scalar dswe_dpc_LS = -params.lambda_LS()/params.pe_LS() * pow(pc/params.pe_LS(), - params.lambda_LS() - 1);
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());
        return nS*dswe_dpc_HS + (1-nS)*dswe_dpc_LS;

        //return -params.lambda()/params.pe() * pow(pc/params.pe(), - params.lambda() - 1);
    }

    /*!
     * \brief The relative permeability for the wetting phase of
     *        the medium implied by the Brooks-Corey
     *        parameterization.
     *
     * \param swe The mobile saturation of the wetting phase.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen,
     *                  and then the params container is constructed accordingly. Afterwards the values are set there, too.
     * \return Relative permeability of the wetting phase calculated as implied by Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar krw(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        //return pow(swe, 2.0/params.lambda() + 3);
        const Scalar krw_HS = params.k0rw_HS() * pow(swe, params.nw_HS());
        if (params.LS() == params.HS())
        {
            TRACE("params.LS() == params.HS()\n");
            return krw_HS;
        }
        const Scalar krw_LS = params.k0rw_LS() * pow(swe, params.nw_LS());
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());
        TRACE("krw (%le) high: %le->%le, low: %le->%le salinity: %le->%le\n", 
            nS,
            params.HS(),krw_HS,  params.LS(), krw_LS, 
            params.S(),
            nS*krw_HS + (1-nS)*krw_LS);
        return nS*krw_HS + (1-nS)*krw_LS;
        // return params.k0rw() * pow(swe, params.nw());
    }

    /*!
     * \brief The derivative of the relative permeability for the
     *        wetting phase with regard to the wetting saturation of the
     *        medium implied by the Brooks-Corey parameterization.
     *
     * \param swe The mobile saturation of the wetting phase.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen,
     *                  and then the params container is constructed accordingly. Afterwards the values are set there, too.
     * \return Derivative of the relative permeability of the wetting phase w.r.t. effective wetting phase
     *                  saturation calculated as implied by Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar dkrw_dswe(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        const Scalar dkrw_dswe_HS = params.nw_HS() * params.k0rw_HS() * pow(swe, params.nw_HS() - 1);
        if (params.LS() == params.HS())
        {
            return dkrw_dswe_HS;
        }
        const Scalar dkrw_dswe_LS = params.nw_LS() * params.k0rw_LS() * pow(swe, params.nw_LS() - 1);
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());

        return nS*dkrw_dswe_HS + (1-nS)*dkrw_dswe_LS;

        //return (2.0/params.lambda() + 3)*pow(swe, 2.0/params.lambda() + 2);
        //return params.nw() * params.k0rw() * pow(swe, params.nw() - 1);
    }

    /*!
     * \brief The relative permeability for the non-wetting phase of
     *        the medium as implied by the Brooks-Corey
     *        parameterization.
     *
     * \param swe The mobile saturation of the wetting phase.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen, and then the params container
     *                  is constructed accordingly. Afterwards the values are set there, too.
     * \return Relative permeability of the non-wetting phase calculated as implied by Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar krn(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        const Scalar krn_HS = params.k0rn_HS() * pow(1 - swe, params.nn_HS());

        if (params.LS() == params.HS())
        {
            return krn_HS;
        }
        const Scalar krn_LS = params.k0rn_LS() * pow(1 - swe, params.nn_LS());

        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());

        TRACE("krn (%le) high: %le->%le, low: %le->%le salinity: %le->%le\n", 
            nS,
            params.HS(),krn_HS,  params.LS(), krn_LS, 
            params.S(),
            nS*krn_HS + (1-nS)*krn_LS);
        return nS*krn_HS + (1-nS)*krn_LS;

        //const Scalar exponent = 2.0/params.lambda() + 1;
        //const Scalar tmp = 1.0 - swe;
        //return tmp*tmp*(1.0 - pow(swe, exponent));

        //return params.k0rn() * pow(1 - swe, params.nn());
    }

    /*!
     * \brief The derivative of the relative permeability for the
     *        non-wetting phase in regard to the wetting saturation of
     *        the medium as implied by the Brooks-Corey
     *        parameterization.
     *
     * \param swe The mobile saturation of the wetting phase.
     * \param params A container object that is populated with the appropriate coefficients for the respective law.
     *                  Therefore, in the (problem specific) spatialParameters  first, the material law is chosen,
     *                  and then the params container is constructed accordingly. Afterwards the values are set there, too.
     * \return Derivative of the relative permeability of the non-wetting phase w.r.t. effective wetting phase
     *                  saturation calculated as implied by Brooks & Corey.
     *
     * \note Instead of undefined behaviour if pc is not in the valid range, we return a valid number,
     *       by clamping the input.
     */
    static Scalar dkrn_dswe(const Params& params, Scalar swe)
    {
        using std::pow;
        using std::min;
        using std::max;

        swe = min(max(swe, 0.0), 1.0); // the equation below is only defined for 0.0 <= sw <= 1.0

        const Scalar dkrn_dswe_HS = (-1) * params.nn_HS() * params.k0rn_HS() * pow(1 - swe, params.nn_HS()-1);
        if (params.LS() == params.HS())
        {
            return dkrn_dswe_HS;
        }
        const Scalar dkrn_dswe_LS = (-1) * params.nn_LS() * params.k0rn_LS() * pow(1 - swe, params.nn_LS()-1);
        const Scalar nS = (params.S()-params.LS())/(params.HS()-params.LS());

        return nS*dkrn_dswe_HS + (1-nS)*dkrn_dswe_LS;

        //return 2.0*(swe - 1)*(1 + pow(swe, 2.0/params.lambda())*(1.0/params.lambda() + 1.0/2- swe*(1.0/params.lambda() + 1.0/2)));

        //return (-1) * params.nn() * params.k0rn() * pow(1 - swe, params.nn()-1);
    }

};

} // end namespace Dumux

#endif // BROOKS_COREY_HH
