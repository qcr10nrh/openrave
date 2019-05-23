// -*- coding: utf-8 -*-
// Copyright (C) 2019 Puttichai Lertkultanon
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU Lesser General Public License as published by the Free Software Foundation, either version 3
// of the License, or at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
#include "polynomialchecker.h"

namespace OpenRAVE {

namespace PiecewisePolynomialsInternal {

PolynomialChecker::PolynomialChecker(size_t ndof, int envid)
{
    this->ndof = ndof;
    this->envid = envid;

    _cacheCoordsVect.reserve(4); // quintic polynomial has at most 4 extrema
}

PolynomialCheckReturn PolynomialChecker::CheckPolynomial(const Polynomial& p, const dReal T, const dReal xmin, const dReal xmax, const dReal vm, const dReal am, const dReal jm)
{
    std::vector<Coordinate>& vcoords = _cacheCoordsVect;

    // Check position limits
    for( std::vector<Coordinate>::const_iterator it = p.vcextrema.begin(); it != p.vcextrema.end(); ++it ) {
        if( it->point >= -g_fPolynomialEpsilon && it->point <= T + g_fPolynomialEpsilon ) {
            // This extremum occurs in the range
            if( it->value > xmax + g_fPolynomialEpsilon || it->value < xmin - g_fPolynomialEpsilon ) {
                return PCR_PositionLimitsViolation;
            }
        }
    }

    // Check velocity limits
    if( p.degree > 0 && vm > g_fPolynomialEpsilon ) {
        p.FindAllLocalExtrema(1, vcoords);
        for( std::vector<Coordinate>::const_iterator it = vcoords.begin(); it != vcoords.end(); ++it ) {
            if( it->point >= -g_fPolynomialEpsilon && it->point <= T + g_fPolynomialEpsilon ) {
                // This extremum occurs in the range
                if( it->value > vm + g_fPolynomialEpsilon || it->value < -vm - g_fPolynomialEpsilon ) {
                    return PCR_VelocityLimitsViolation;
                }
            }
        }
    }

    // Check acceleration limits
    if( p.degree > 1 && am > g_fPolynomialEpsilon ) {
        p.FindAllLocalExtrema(2, vcoords);
        for( std::vector<Coordinate>::const_iterator it = vcoords.begin(); it != vcoords.end(); ++it ) {
            if( it->point >= -g_fPolynomialEpsilon && it->point <= T + g_fPolynomialEpsilon ) {
                // This extremum occurs in the range
                if( it->value > am + g_fPolynomialEpsilon || it->value < -am - g_fPolynomialEpsilon ) {
                    return PCR_AccelerationLimitsViolation;
                }
            }
        }
    }

    // Check jerk limits
    if( p.degree > 2 && jm > g_fPolynomialEpsilon ) {
        p.FindAllLocalExtrema(3, vcoords);
        for( std::vector<Coordinate>::const_iterator it = vcoords.begin(); it != vcoords.end(); ++it ) {
            if( it->point >= -g_fPolynomialEpsilon && it->point <= T + g_fPolynomialEpsilon ) {
                // This extremum occurs in the range
                if( it->value > jm + g_fPolynomialEpsilon || it->value < -jm - g_fPolynomialEpsilon ) {
                    return PCR_JerkLimitsViolation;
                }
            }
        }
    }
}

PolynomialCheckReturn PolynomialChecker::CheckChunk(const Chunk& c, const std::vector<dReal>& xminVect, const std::vector<dReal>& xmaxVect, const std::vector<dReal>& vmVect, const std::vector<dReal>& amVect, const std::vector<dReal>& jmVect)
{
    dReal vm = 0;
    dReal am = 0;
    dReal jm = 0;
    bool bHasVelocityLimits = vmVect.size() > 0;
    bool bHasAccelerationLimits = amVect.size() > 0;
    bool bHasJerkLimits = jmVect.size() > 0;
    for( size_t idof = 0; idof < ndof; ++idof ) {
        if( bHasVelocityLimits ) {
            vm = vmVect[idof];
        }
        if( bHasAccelerationLimits ) {
            am = amVect[idof];
        }
        if( bHasJerkLimits ) {
            jm = jmVect[idof];
        }
        PolynomialCheckReturn ret = CheckPolynomial(c.vpolynomials[idof], c.duration, xminVect[idof], xmaxVect[idof], vm, am, jm);
        if( ret != PCR_Normal ) {
            return ret;
        }
    }
}

PolynomialCheckReturn PolynomialChecker::CheckPiecewisePolynomialTrajectory(const PiecewisePolynomialTrajectory& traj, const std::vector<dReal>& xminVect, const std::vector<dReal>& xmaxVect, const std::vector<dReal>& vmVect, const std::vector<dReal>& amVect, const std::vector<dReal>& jmVect)
{
    dReal vm = 0;
    dReal am = 0;
    dReal jm = 0;
    bool bHasVelocityLimits = vmVect.size() > 0;
    bool bHasAccelerationLimits = amVect.size() > 0;
    bool bHasJerkLimits = jmVect.size() > 0;
    for( std::vector<Chunk>::const_iterator itchunk = traj.vchunks.begin(); itchunk != traj.vchunks.end(); ++itchunk ) {
        for( size_t idof = 0; idof < ndof; ++idof ) {
            if( bHasVelocityLimits ) {
                vm = vmVect[idof];
            }
            if( bHasAccelerationLimits ) {
                am = amVect[idof];
            }
            if( bHasJerkLimits ) {
                jm = jmVect[idof];
            }
            PolynomialCheckReturn ret = CheckPolynomial(itchunk->vpolynomials[idof], itchunk->duration, xminVect[idof], xmaxVect[idof], vm, am, jm);
            if( ret != PCR_Normal ) {
                return ret;
            }
        }
    }
}

} // end namespace PiecewisePolynomialsInternal

} // end namespace OpenRAVE
