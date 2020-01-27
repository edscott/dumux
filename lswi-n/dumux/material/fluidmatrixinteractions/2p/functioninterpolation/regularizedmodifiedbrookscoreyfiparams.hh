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
 * \brief   Parameters that are necessary for the \em regularization of
 *          the Brooks-Corey capillary pressure model.
 */
#ifndef DUMUX_REGULARIZED_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_PARAMS_HH
#define DUMUX_REGULARIZED_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_PARAMS_HH

#include "modifiedbrookscoreyfiparams.hh"

namespace Dumux
{
/*!
 * \brief   Parameters that are necessary for the \em regularization of
 *          the Brooks-Corey capillary pressure model.
 * \ingroup Fluidmatrixinteractions
 */
template <class ScalarT>
class RegularizedModifiedBrooksCoreyFIParams : public ModifiedBrooksCoreyFIParams<ScalarT>
{
    using ModifiedBrooksCoreyFIParams = Dumux::ModifiedBrooksCoreyFIParams<ScalarT>;

public:
    using Scalar = ScalarT;

    RegularizedModifiedBrooksCoreyFIParams()
        : ModifiedBrooksCoreyFIParams()
    {
        setThresholdSw(0.01);
    }

    RegularizedModifiedBrooksCoreyFIParams(Scalar pe, Scalar lambda)
        : ModifiedBrooksCoreyFIParams(pe, lambda)
    {
        setThresholdSw(0.01);
    }

    /*!
     * \brief Set the threshold saturation below which the capillary pressure
     *        is regularized.
     *
     * Most problems are very sensitive to this value (e.g. making it smaller
     * might result in negative pressures).
     */
    void setThresholdSw(Scalar thresholdSw)
    {
        thresholdSw_ = thresholdSw;
    }

    /*!
     * \brief Threshold saturation below which the capillary pressure
     *        is regularized.
     */
    Scalar thresholdSw() const
    {
        return thresholdSw_;
    }

private:
    Scalar thresholdSw_;
};
} // namespace Dumux

#endif
