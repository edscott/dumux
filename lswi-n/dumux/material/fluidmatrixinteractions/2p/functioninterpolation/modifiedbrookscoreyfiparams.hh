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
 * \brief Specification of the material parameters
 *       for the Brooks Corey constitutive relations.
 */
#ifndef DUMUX_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_PARAMS_HH
#define DUMUX_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_PARAMS_HH

#include <dumux/common/valgrind.hh>

namespace Dumux
{

/*!
 * \brief Specification of the material parameters
 *       for the Brooks Corey constitutive relations.
 *
 * \ingroup Fluidmatrixinteractions
 *
 *\see BrooksCorey
 */
template <class ScalarT>
class ModifiedBrooksCoreyFIParams
{
public:
    using Scalar = ScalarT;

    ModifiedBrooksCoreyFIParams()
    {
        Valgrind::SetUndefined(*this);
    }

    ModifiedBrooksCoreyFIParams(Scalar pe, Scalar lambda)
        : pe_HS_(pe), pe_LS_(pe), lambda_HS_(lambda), lambda_LS_(lambda)
    {
    }

    void dump(void){
        DBG("MatrixPe:       Hi = %le  Low = %le \n", pe_HS_,pe_LS_); 
        DBG("MatrixLambda:   Hi = %le  Low = %le \n", lambda_HS_,lambda_LS_); 
        DBG("MatrixKrwMax:   Hi = %le  Low = %le \n", k0rw_HS_,k0rw_LS_); 
        DBG("MatrixKrnMax:   Hi = %le  Low = %le \n", k0rn_HS_,k0rn_LS_); 
        DBG("Matrixnw:       Hi = %le  Low = %le \n", nw_HS_,nw_LS_); 
        DBG("Matrixnn:       Hi = %le  Low = %le \n", nn_HS_,nn_LS_); 
        DBG("SalinityLimits: Hi = %le  Low = %le \n", HS_,LS_); 

    }

    /*!
     * \brief Returns the entry pressure in \f$\mathrm{[Pa]}\f$
     */
    Scalar pe_HS() const
    { return pe_HS_; }

    Scalar pe_LS() const
    { return pe_LS_; }

    /*!
     * \brief Set the entry pressure in \f$\mathrm{[Pa]}\f$]
     */
    void setPe_HS(Scalar v)
    { pe_HS_ = v; }

    void setPe_LS(Scalar v)
    { pe_LS_ = v; }


    /*!
     * \brief Returns the lambda shape parameter \f$\mathrm{[-]}\f$
     */
    Scalar lambda_HS() const
    { return lambda_HS_; }

    Scalar lambda_LS() const
    { return lambda_LS_; }

    /*!
     * \brief Set the lambda shape parameter \f$\mathrm{[-]}\f$
     */
    void setLambda_HS(Scalar v)
    { lambda_HS_ = v; }

    void setLambda_LS(Scalar v)
    { lambda_LS_ = v; }

    // Extra parameters for the modified Brooks Corey (Lake, 1989; El-Amin, et al., 2015)

    //
    /*!
     * \brief Returns the K0rw parameter \f$\mathrm{[-]}\f$
     */
    Scalar k0rw_HS() const
    { return k0rw_HS_; }

    Scalar k0rw_LS() const
    { return k0rw_LS_; }

    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setK0rw_HS(Scalar v)
    { k0rw_HS_ = v; }

    void setK0rw_LS(Scalar v)
    { k0rw_LS_ = v; }

    //
    /*!
     * \brief Returns the  parameter \f$\mathrm{[-]}\f$
     */
    Scalar k0rn_HS() const
    { return k0rn_HS_; }

    Scalar k0rn_LS() const
    { return k0rn_LS_; }

    /*!
     * \brief Set the K0rn parameter \f$\mathrm{[-]}\f$
     */
    void setK0rn_HS(Scalar v)
    { k0rn_HS_ = v; }

    void setK0rn_LS(Scalar v)
    { k0rn_LS_ = v; }

    //
    /*!
     * \brief Returns the Nw parameter \f$\mathrm{[-]}\f$
     */
    Scalar nw_HS() const
    { return nw_HS_; }

    Scalar nw_LS() const
    { return nw_LS_; }

    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setNw_HS(Scalar v)
    { nw_HS_ = v; }

    void setNw_LS(Scalar v)
    { nw_LS_ = v; }

    //
    /*!
     * \brief Returns the Nn parameter \f$\mathrm{[-]}\f$
     */
    Scalar nn_HS() const
    { return nn_HS_; }

    Scalar nn_LS() const
    { return nn_LS_; }

    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setNn_HS(Scalar v)
    { nn_HS_ = v; }

    void setNn_LS(Scalar v)
    { nn_LS_ = v; }

    //
    /*!
    * \brief Returns the Salinity parameter \f$\mathrm{[-]}\f$
    */
    Scalar S() const
    { return S_; }

    Scalar HS() const
    { return HS_; }

    Scalar LS() const
    { return LS_; }

    /*!
     * \brief Set the Salinity parameter \f$\mathrm{[-]}\f$
     */
    void setS(Scalar v)
    { S_ = v; }

    void setHS(Scalar v)
    { 
        TRACE("high salinity set to %le\n", v);
        HS_ = v; 
    }

    void setLS(Scalar v)
    { 
        TRACE("low salinity set to %le\n", v);
        LS_ = v; 
    }

private:

    mutable Scalar S_;

    // Modified Brooks Corey for relative permeability
    // high salinity
    mutable Scalar pe_HS_;
    mutable Scalar lambda_HS_;
    mutable Scalar k0rw_HS_;
    mutable Scalar k0rn_HS_;
    mutable Scalar nw_HS_;
    mutable Scalar nn_HS_;
    mutable Scalar HS_;

    // low salinity
    mutable Scalar pe_LS_;
    mutable Scalar lambda_LS_;
    mutable Scalar k0rw_LS_;
    mutable Scalar k0rn_LS_;
    mutable Scalar nw_LS_;
    mutable Scalar nn_LS_;
    mutable Scalar LS_;

};
} // namespace Dumux

#endif
