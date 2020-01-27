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
 * \brief A default implementation of the parameters for the adapter
 *        class to convert material laws from effective to absolute
 *        saturations.
 */
#ifndef DUMUX_EFF_TO_ABS_LAW_PARAMS_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_HH
#define DUMUX_EFF_TO_ABS_LAW_PARAMS_MODIFIED_BROOKS_COREY_FUNCTION_INTERPOLATION_HH

#include <dune/common/float_cmp.hh>

namespace Dumux
{
/*!
 * \ingroup Fluidmatrixinteractions
 * \brief A default implementation of the parameters for the adapter
 *        class to convert material laws from effective to absolute
 *        saturations.
 */
template <class EffLawParamsT>
class EffToAbsLawParams : public EffLawParamsT
{
    using EffLawParams = EffLawParamsT;
public:
    using Scalar = typename EffLawParams::Scalar;

    EffToAbsLawParams()
        : EffLawParams()
    { swr_HS_ = snr_HS_ = swr_LS_ = snr_LS_ = 0; }

    /*!
     * \brief Equality comparison with another set of params
     */
    template<class OtherParams>
    bool operator== (const OtherParams& otherParams) const
    {
        return Dune::FloatCmp::eq(swr_HS_, otherParams.swrHS(), /*eps*/1e-6*swr_HS_)
               && Dune::FloatCmp::eq(snr_HS_, otherParams.snrHS(), /*eps*/1e-6*snr_HS_)
               && Dune::FloatCmp::eq(swr_LS_, otherParams.swrLS(), /*eps*/1e-6*swr_LS_)
               && Dune::FloatCmp::eq(snr_LS_, otherParams.snrLS(), /*eps*/1e-6*snr_LS_)
               && EffLawParamsT::operator==(otherParams);
    }

    /*!
     * \brief Return the residual wetting saturation.
     */
    Scalar swr_HS() const
    { return swr_HS_; }

    Scalar swr_LS() const
    { return swr_LS_; }

    /*!
     * \brief Set the residual wetting saturation.
     */
    void setSwr_HS(Scalar v)
    { swr_HS_ = v; }

    void setSwr_LS(Scalar v)
    { swr_LS_ = v; }

    /*!
     * \brief Return the residual non-wetting saturation.
     */
    Scalar snr_HS() const
    { return snr_HS_; }

    Scalar snr_LS() const
    { return snr_LS_; }

    /*!
     * \brief Set the residual non-wetting saturation.
     */
    void setSnr_HS(Scalar v)
    { snr_HS_ = v; }

    void setSnr_LS(Scalar v)
    { snr_LS_ = v; }

private:
    Scalar swr_HS_;
    Scalar snr_HS_;

    Scalar swr_LS_;
    Scalar snr_LS_;
};

}

#endif
