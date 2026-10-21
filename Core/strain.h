/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Author(s): Brian Bay (OSU)
           Edoardo Pasca (UKRI-STFC)
		   
*/

#ifndef DVC_H
#define DVC_H

// full scope kinematic analysis support
#ifndef STRAIN_MEASURES_HPP
#define STRAIN_MEASURES_HPP

#include <string>

/******************************************************************************/
/******************************************************************************/

	
/******************************************************************************/
/******************************************************************************/

//#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <stdint.h>
#include <omp.h>
#include <thread>
#include <algorithm>

// adjust Makefile if changes made here
#include "InputRead.h"
#include "DataCloud.h"
#include "Search.h"
#include "Utility.h"

// full scope kinematic analysis support
#include <Eigen/Dense>


/******************************************************************************/
class CCPI_EXPORT StrainCalc
{
public:
	StrainCalc();
/*
	int find_flag(std::string flag, int &argc, char *argv[]);
	int find_flag(std::string flag, int &argc, char *argv[], int &val);
	int find_flag(std::string flag, int &argc, char *argv[], double &val);
	int find_flag(size_t pos, size_t len, std::string flag, int &argc, char *argv[]);
*/
	int nmp() const {return num_mod_para;}

	int ndp_min() const {return num_data_points_min;}
	int ndp_def() const {return num_data_points_def;}

	double objt_min() const {return objmin_thresh_min;}
	double objt_max() const {return objmin_thresh_max;}
	double objt_def() const {return objmin_thresh_def;}

	void write_output (std::string fname, std::string hdr, DataCloud &data, std::vector<std::vector<double>> &output);


private:
	
	int num_mod_para;				// number of model parameters (terms in the polynomial)

	int num_data_points_min;		// min number for the strain window size
	int num_data_points_def;		// default number for the strain window size
	// max comes from DataCloud .sort file setting

	double objmin_thresh_min;		// min for objmin threshold
	double objmin_thresh_max;		// max for objmin threshold
	double objmin_thresh_def;		// default for objmin threshold


};
/******************************************************************************/
// strain_measures.hpp
//
// Kinematic quantities derived from the 3D displacement gradient tensor D (Eigen).
//
//   Convention: D(i,j) = du_i / dX_j   (gradient w.r.t. reference coordinates)
//   If your D is stored as du_j / dX_i, pass D.transpose().
//
//   F = I + D                     deformation gradient,  J = det F
//   C = F^T F = U^2               right Cauchy-Green (reference frame)
//   B = F F^T = V^2               left Cauchy-Green  (deformed frame)
//   F = R U = V R                 polar decomposition, R proper rotation
//
//   Spectral form, with principal stretches lambda_i and reference principal
//   directions N_i (eigen-decomposition of C), deformed directions n_i = R N_i:
//     U = sum lambda_i N_i (x) N_i        V = sum lambda_i n_i (x) n_i
//
// Usage: build one strain::Kinematics per point. That does a single
// symmetric eigen-solve; every quantity below is then derived from it.
//
//   strain::Kinematics k(D);
//   if (!k.valid()) { /* det F <= 0: flag the point */ }
//   Eigen::Matrix3d E  = k.greenLagrange();
//   Eigen::Matrix3d EB = k.biot();
//   Eigen::Vector3d w  = k.rotationVector();               // theta * axis
//   Eigen::Vector3d n  = k.orientation(N0, strain::OrientationMode::MaterialFiber);
//
// Validity: if det F <= 0 (inverted or collapsed subvolume, e.g. a bad
// correlation) the polar decomposition does not exist. valid() is then
// false, and every quantity that needs U, V or R returns NaN. Quantities that
// need only F (small strain/rotation, Green-Lagrange, Cauchy-Green tensors,
// J, fiber stretch) are still returned so the data can be inspected.
//
// Header-only, C++11, requires Eigen 3.3 or later.

struct PointResultBasic {
    bool valid;
    double J, volumetric, equivalent;
    Eigen::Vector3d principalStrain;     // Biot principal values, lambda_i - 1
    Eigen::Matrix3d biot, henc, grla;
    Eigen::Vector3d rotationVector;      // theta * axis, radians
};

struct PointResultFull {
    bool valid;
    double J, volumetric, equivalent;
    Eigen::Vector3d principalStrain;     // Biot principal values, lambda_i - 1
    Eigen::Matrix3d biot, henc, grla;
    Eigen::Vector3d rotationVector;      // theta * axis, radians
    Eigen::Vector3d fiberRigid;          // (a) R N0
    Eigen::Vector3d fiberDeformed;       // (b) F N0 / |F N0|
    double fiberStretch, fiberTurn;      // |F N0|, angle N0 -> (b)
    Eigen::Vector3d planeNormal;         // Nanson: F^-T M0, normalized
    double areaRatio;
};

namespace strain {

// How a reference direction N0 is carried into the deformed configuration.
enum class OrientationMode {
    // n = R N0. Rigid-body rotation of the material region, stretch removed.
    // Every direction turns by the same rotation R. Use for "how much did this
    // region rotate"; exact for the principal stretch directions.
    RigidRotation,
    // n = F N0 / |F N0|. Where the material line initially along N0 actually
    // points after deformation, including reorientation caused by stretch and
    // shear. Use for physical features: fibers, laminae, inclusions, etc.
    MaterialFiber
};

class Kinematics {
public:
    using Matrix3d = Eigen::Matrix3d;
    using Vector3d = Eigen::Vector3d;

    explicit Kinematics(const Matrix3d& D)
        : D_(D), F_(Matrix3d::Identity() + D), J_(F_.determinant())
    {
        C_ = F_.transpose() * F_;
        valid_ = J_ > 0.0;   // false for NaN input as well

        if (!valid_) {
            const double nan = std::numeric_limits<double>::quiet_NaN();
            lambda_.setConstant(nan);
            N_.setConstant(nan);
            R_.setConstant(nan);
            n_.setConstant(nan);
            angle_ = nan;
            axis_.setConstant(nan);
            return;
        }

        // Iterative solver: accurate eigenvectors even when eigenvalues are
        // nearly equal, which is the normal case for C near I (small strain).
        Eigen::SelfAdjointEigenSolver<Matrix3d> es(C_);

        // Eigen sorts ascending; store descending so lambda(0) is the maximum.
        for (int i = 0; i < 3; ++i) {
            lambda_(i) = std::sqrt(es.eigenvalues()(2 - i));
            N_.col(i) = es.eigenvectors().col(2 - i);
        }
        // Make N a proper rotation (right-handed triad). Eigenvector signs are
        // arbitrary, so this only flips one column's sign.
        if (N_.determinant() < 0.0) N_.col(2) = -N_.col(2);

        // R = F U^-1
        R_ = F_ * N_ * lambda_.cwiseInverse().asDiagonal() * N_.transpose();
        n_ = R_ * N_;

        const Eigen::AngleAxisd aa(R_);
        angle_ = aa.angle();
        axis_ = aa.axis();
    }

    // ---------------------------------------------------------------- basics
    bool valid() const { return valid_; }
    const Matrix3d& displacementGradient() const { return D_; }
    const Matrix3d& deformationGradient() const { return F_; }
    double jacobian() const { return J_; }   // volume ratio dv / dV

    // ------------------------------------------- infinitesimal (first order)
    // Not rotation invariant: use only when rotations and strains are small.
    Matrix3d smallStrain() const { return 0.5 * (D_ + D_.transpose()); }
    Matrix3d smallRotation() const { return 0.5 * (D_ - D_.transpose()); }
    // Axial vector of smallRotation(); approximates rotationVector().
    Vector3d smallRotationVector() const
    {
        const Matrix3d W = smallRotation();
        return Vector3d(W(2, 1), W(0, 2), W(1, 0));
    }

    // ------------------------------------- finite strain, reference frame
    const Matrix3d& rightCauchyGreen() const { return C_; }
    Matrix3d greenLagrange() const { return 0.5 * (C_ - Matrix3d::Identity()); }
    Matrix3d rightStretch() const { return spectralRef(lambda_); }                    // U
    Matrix3d biot() const { return spectralRef(lambda_.array() - 1.0); }              // U - I
    Matrix3d hencky() const { return spectralRef(lambda_.array().log()); }            // ln U

    // --------------------------------------- finite strain, deformed frame
    // Each equals R (reference-frame counterpart) R^T.
    Matrix3d leftCauchyGreen() const { return F_ * F_.transpose(); }                  // B
    Matrix3d leftStretch() const { return spectralDef(lambda_); }                     // V
    Matrix3d spatialBiot() const { return spectralDef(lambda_.array() - 1.0); }       // V - I
    Matrix3d spatialHencky() const { return spectralDef(lambda_.array().log()); }     // ln V
    Matrix3d eulerAlmansi() const                                                     // 1/2 (I - B^-1)
    {
        return spectralDef(0.5 * (1.0 - lambda_.array().square().inverse()));
    }

    // ------------------------------------------------ principal quantities
    // Principal stretches, descending: lambda(0) >= lambda(1) >= lambda(2).
    // Principal engineering strains are lambda - 1; true strains ln(lambda).
    const Vector3d& principalStretches() const { return lambda_; }
    // Columns N_i, matching principalStretches(); right-handed triad.
    // Directions are not unique where stretches are equal (any basis of the
    // repeated eigenspace is valid) and each has an arbitrary sign.
    const Matrix3d& referencePrincipalDirections() const { return N_; }
    // Columns n_i = R N_i: principal directions in the deformed configuration.
    const Matrix3d& deformedPrincipalDirections() const { return n_; }

    // Hencky volumetric strain, tr(ln U) = ln J.
    double volumetricStrain() const { return std::log(J_); }
    // von Mises equivalent strain from the Hencky deviator: sqrt(2/3 h':h').
    // Equals |ln lambda| for isochoric uniaxial stretch; zero for pure dilation.
    double equivalentStrain() const
    {
        const Eigen::Array3d h = lambda_.array().log();
        const Eigen::Array3d dev = h - h.mean();
        return std::sqrt(2.0 / 3.0 * dev.square().sum());
    }

    // --------------------------------------------------------------- rotation
    const Matrix3d& rotation() const { return R_; }                 // R
    double rotationAngle() const { return angle_; }                 // radians, [0, pi]
    // Unit axis, right-hand rule. Undefined (and arbitrary) when the angle is
    // ~0; check rotationAngle() before using it.
    const Vector3d& rotationAxis() const { return axis_; }
    Vector3d rotationVector() const { return angle_ * axis_; }      // theta * axis

    // ---------------------------------------- orientation of directions
    // Option (a): n = R N0 (rigid-body rotation of the region).
    Vector3d rigidRotate(const Vector3d& N0) const { return R_ * N0.normalized(); }

    // Option (b): n = F N0 / |F N0| (deformed direction of a material line).
    Vector3d deformFiber(const Vector3d& N0) const { return (F_ * N0.normalized()).normalized(); }

    Vector3d orientation(const Vector3d& N0, OrientationMode mode) const
    {
        return mode == OrientationMode::RigidRotation ? rigidRotate(N0) : deformFiber(N0);
    }

    // Angle (radians, [0, pi]) between N0 and its deformed orientation.
    // For RigidRotation this equals rotationAngle() only when N0 is
    // perpendicular to the rotation axis.
    double orientationChange(const Vector3d& N0, OrientationMode mode) const
    {
        return angleBetween(N0, orientation(N0, mode));
    }

    // Stretch of the material line along N0: |F N0| (lambda; strain = lambda - 1).
    double fiberStretch(const Vector3d& N0) const { return (F_ * N0.normalized()).norm(); }

    // Change in angle between two material lines (radians): initial angle
    // minus deformed angle. For initially orthogonal lines this is the exact
    // engineering shear strain gamma (positive when the angle closes).
    double angleChange(const Vector3d& N1, const Vector3d& N2) const
    {
        return angleBetween(N1, N2) - angleBetween(F_ * N1, F_ * N2);
    }

    // --------------------------------------------------- material planes
    // Plane normals do not follow F. For a plane with reference unit normal
    // M0, the deformed normal follows Nanson's relation: m ~ F^-T M0.
    // Option (a) for a plane is rigidRotate(M0); option (b) is this function.
    Vector3d deformNormal(const Vector3d& M0) const
    {
        return (F_.inverse().transpose() * M0.normalized()).normalized();
    }
    // Area ratio da / dA of a plane element with reference normal M0.
    double areaRatio(const Vector3d& M0) const
    {
        return J_ * (F_.inverse().transpose() * M0.normalized()).norm();
    }

private:
    // sum g_i N_i (x) N_i
    Matrix3d spectralRef(const Vector3d& g) const { return N_ * g.asDiagonal() * N_.transpose(); }
    // sum g_i n_i (x) n_i
    Matrix3d spectralDef(const Vector3d& g) const { return n_ * g.asDiagonal() * n_.transpose(); }

    static double angleBetween(const Vector3d& a, const Vector3d& b)
    {
        return std::atan2(a.cross(b).norm(), a.dot(b));   // robust near 0 and pi
    }

    Matrix3d D_, F_, C_;
    double J_;
    bool valid_;
    Vector3d lambda_;     // principal stretches, descending
    Matrix3d N_;          // reference principal directions (columns)
    Matrix3d R_;          // rotation
    Matrix3d n_;          // deformed principal directions (columns)
    double angle_;
    Vector3d axis_;
};

// ------------------------------------------------------- convenience wrappers
// One-off calls. When computing several quantities at the same point,
// construct a Kinematics once instead (one eigen-solve rather than several).
inline Eigen::Matrix3d greenLagrange(const Eigen::Matrix3d& D)
{
    return 0.5 * (D + D.transpose() + D.transpose() * D);
}
inline Eigen::Matrix3d biot(const Eigen::Matrix3d& D) { return Kinematics(D).biot(); }
inline Eigen::Matrix3d hencky(const Eigen::Matrix3d& D) { return Kinematics(D).hencky(); }
inline Eigen::Matrix3d polarRotation(const Eigen::Matrix3d& D) { return Kinematics(D).rotation(); }

} // namespace strain

/******************************************************************************/

#endif // STRAIN_MEASURES_HPP
#endif // DVC_H


