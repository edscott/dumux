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
#ifndef DUMUX_MODIFIED_BROOKS_COREY_PARAMS_HH
#define DUMUX_MODIFIED_BROOKS_COREY_PARAMS_HH

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
class ModifiedBrooksCoreyParams
{
public:
    using Scalar = ScalarT;

    ModifiedBrooksCoreyParams()
    {
        Valgrind::SetUndefined(*this);
    }

    ModifiedBrooksCoreyParams(Scalar pe, Scalar lambda)
        : pe_(pe), lambda_(lambda)
    {
    }

    /*!
     * \brief Returns the entry pressure in \f$\mathrm{[Pa]}\f$
     */

    Scalar pe() const
    { return pe_; }

    /*!
     * \brief Set the entry pressure in \f$\mathrm{[Pa]}\f$]
     */
    void setPe(Scalar v)
    { pe_ = v; }

    /*!
     * \brief Returns the lambda shape parameter \f$\mathrm{[-]}\f$
     */

    Scalar lambda() const
    { return lambda_; }

    /*!
     * \brief Set the lambda shape parameter \f$\mathrm{[-]}\f$
     */
    void setLambda(Scalar v)
    { lambda_ = v; }

    // Extra parameters for the modified Brooks Corey (Lake, 1989; El-Amin, et al., 2015)

    //
    /*!
     * \brief Returns the K0rw parameter \f$\mathrm{[-]}\f$
     */

    Scalar k0rw() const
    { return k0rw_; }

    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setK0rw(Scalar v)
    { k0rw_ = v; }

    //
    /*!
     * \brief Returns the  parameter \f$\mathrm{[-]}\f$
     */

    Scalar k0rn() const
    { return k0rn_; }

    /*!
     * \brief Set the K0rn parameter \f$\mathrm{[-]}\f$
     */
    void setK0rn(Scalar v)
    { k0rn_ = v; }

    //
    /*!
     * \brief Returns the Nw parameter \f$\mathrm{[-]}\f$
     */

    Scalar nw() const
    { return nw_; }

    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setNw(Scalar v)
    { nw_ = v; }

    //
    /*!
     * \brief Returns the Nn parameter \f$\mathrm{[-]}\f$
     */

    Scalar nn() const
    { return nn_; }


    /*!
     * \brief Set the  parameter \f$\mathrm{[-]}\f$
     */
    void setNn(Scalar v)
    { nn_ = v; }


private:

    Scalar pe_;
    Scalar lambda_;
    Scalar k0rw_;
    Scalar k0rn_;
    Scalar nw_;
    Scalar nn_;

};
} // namespace Dumux

#endif
