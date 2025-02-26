// Copyright (c) 2009-2022 The Regents of the University of Michigan.
// Part of HOOMD-blue, released under the BSD 3-Clause License.

#include "TwoStepRATTLEBDGPU.cuh"
#include "TwoStepRATTLELangevinGPU.cuh"
#include "TwoStepRATTLENVEGPU.cuh"

#include "ManifoldZCylinder.h"

namespace hoomd
    {
namespace md
    {
namespace kernel
    {
template hipError_t
gpu_rattle_brownian_step_one<ManifoldZCylinder>(Scalar4* d_pos,
                                                int3* d_image,
                                                Scalar4* d_vel,
                                                const BoxDim& box,
                                                const Scalar* d_diameter,
                                                const unsigned int* d_tag,
                                                const unsigned int* d_group_members,
                                                const unsigned int group_size,
                                                const Scalar4* d_net_force,
                                                const Scalar3* d_gamma_r,
                                                Scalar4* d_orientation,
                                                Scalar4* d_torque,
                                                const Scalar3* d_inertia,
                                                Scalar4* d_angmom,
                                                const rattle_bd_step_one_args& rattle_bd_args,
                                                ManifoldZCylinder manifold,
                                                const bool aniso,
                                                const Scalar deltaT,
                                                const unsigned int D,
                                                const bool d_noiseless_t,
                                                const bool d_noiseless_r,
                                                const GPUPartition& gpu_partition);

template hipError_t
gpu_include_rattle_force_bd<ManifoldZCylinder>(const Scalar4* d_pos,
                                               Scalar4* d_net_force,
                                               Scalar* d_net_virial,
                                               const Scalar* d_diameter,
                                               const unsigned int* d_tag,
                                               const unsigned int* d_group_members,
                                               const unsigned int group_size,
                                               const rattle_bd_step_one_args& rattle_bd_args,
                                               ManifoldZCylinder manifold,
                                               size_t net_virial_pitch,
                                               const Scalar deltaT,
                                               const bool d_noiseless_t,
                                               const GPUPartition& gpu_partition);

template hipError_t gpu_rattle_langevin_step_two<ManifoldZCylinder>(
    const Scalar4* d_pos,
    Scalar4* d_vel,
    Scalar3* d_accel,
    const Scalar* d_diameter,
    const unsigned int* d_tag,
    unsigned int* d_group_members,
    unsigned int group_size,
    Scalar4* d_net_force,
    const rattle_langevin_step_two_args& rattle_langevin_args,
    ManifoldZCylinder manifold,
    Scalar deltaT,
    unsigned int D);

template hipError_t gpu_rattle_nve_step_two<ManifoldZCylinder>(Scalar4* d_pos,
                                                               Scalar4* d_vel,
                                                               Scalar3* d_accel,
                                                               unsigned int* d_group_members,
                                                               const GPUPartition& gpu_partition,
                                                               Scalar4* d_net_force,
                                                               ManifoldZCylinder manifold,
                                                               Scalar eta,
                                                               Scalar deltaT,
                                                               bool limit,
                                                               Scalar limit_val,
                                                               bool zero_force,
                                                               unsigned int block_size);

template hipError_t
gpu_include_rattle_force_nve<ManifoldZCylinder>(const Scalar4* d_pos,
                                                const Scalar4* d_vel,
                                                Scalar3* d_accel,
                                                Scalar4* d_net_force,
                                                Scalar* d_net_virial,
                                                unsigned int* d_group_members,
                                                const GPUPartition& gpu_partition,
                                                size_t net_virial_pitch,
                                                ManifoldZCylinder manifold,
                                                Scalar eta,
                                                Scalar deltaT,
                                                bool zero_force,
                                                unsigned int block_size);

    } // end namespace kernel
    } // end namespace md
    } // end namespace hoomd
