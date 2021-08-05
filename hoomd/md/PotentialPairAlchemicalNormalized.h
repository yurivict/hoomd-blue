// Copyright (c) 2009-2021 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

// Maintainer: jproc

#ifndef POTENTIAL_PAIR_ALCHEMICAL_NORMALIZED_H
#define POTENTIAL_PAIR_ALCHEMICAL_NORMALIZED_H

#include "PotentialPairAlchemical.h"

#include "pybind11/functional.h"
#include "pybind11/stl.h"

#ifdef ENABLE_HIP
#include <hip/hip_runtime.h>
#endif

#ifdef ENABLE_MPI
#include "hoomd/Communicator.h"
#endif

/*! \file PotentialPairAlchemicalNormalized.h
    \brief Defines the template class for alchemical pair potentials
*/

#ifdef __HIPCC__
#error This header cannot be compiled by nvcc
#endif

template<class evaluator> struct AlchemyPackageNormalized : AlchemyPackage<evaluator>
    {
    std::vector<Scalar> normalization_values = {};

    AlchemyPackageNormalized(std::nullptr_t) {};
    AlchemyPackageNormalized() {};
    };

template<class evaluator> struct Normalized : public evaluator
    {
    using evaluator::evaluator;
    Scalar m_normValue {1.};

    void setNormalizationValue(Scalar value)
        {
        m_normValue = value;
        }

    bool evalForceAndEnergy(Scalar& force_divr, Scalar& pair_eng, bool energy_shift)
        {
        bool evaluated = evaluator::evalForceAndEnergy(force_divr, pair_eng, energy_shift);
        force_divr *= m_normValue;
        pair_eng *= m_normValue;
        return evaluated;
        }

    static std::string getName()
        {
        return std::string("normalized_") + evaluator::getName();
        }
    };

//! Template class for computing normalized alchemical pair potentials
/*! <b>Overview:</b>

    <b>Implementation details</b>



    \sa export_PotentialPair()
*/
template<class evaluator,
         typename extra_pkg = AlchemyPackageNormalized<evaluator>,
         typename alpha_particle_type = AlchemicalNormalizedPairParticle>
class PotentialPairAlchemicalNormalized
    : public PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>,
      public std::enable_shared_from_this<
          PotentialPairAlchemicalNormalized<evaluator, extra_pkg, alpha_particle_type>>
    {
    protected:
    public:
    //! Construct the pair potential
    PotentialPairAlchemicalNormalized(std::shared_ptr<SystemDefinition> sysdef,
                                      std::shared_ptr<NeighborList> nlist)
        : PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>(sysdef, nlist){};
    //! Destructor
    ~PotentialPairAlchemicalNormalized() {};

    void setNormalizer(std::function<std::vector<Scalar>(pybind11::list)>& callback)
        {
        m_normalizer = callback;
        }

    protected:
    // templated base classes need us to pull members we want into scope or overuse this->
    using PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>::m_exec_conf;
    using PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>::m_alchemy_index;
    using PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>::
        m_alchemical_particles;
    using PotentialPair<evaluator, extra_pkg>::m_params;
    using PotentialPair<evaluator, extra_pkg>::m_typpair_idx;

    std::function<std::vector<Scalar>(pybind11::list)> m_normalizer = nullptr;

    // Extra steps to insert
    inline extra_pkg pkgInitialize(const uint64_t& timestep) override;
    inline void pkgPerNeighbor(const unsigned int& i,
                               const unsigned int& j,
                               const unsigned int& typei,
                               const unsigned int& typej,
                               const bool& in_rcut,
                               evaluator& eval,
                               extra_pkg&) override;
    inline void pkgFinalize(extra_pkg&) override;
    };

template<class evaluator, typename extra_pkg, typename alpha_particle_type>
inline extra_pkg
PotentialPairAlchemicalNormalized<evaluator, extra_pkg, alpha_particle_type>::pkgInitialize(
    const uint64_t& timestep)
    {
    // Create pkg for passing additional variables between specialized code
    extra_pkg pkg
        = PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>::pkgInitialize(
            timestep);

    // Precompute normalization
    // pkg.normalization_values.assign(m_alchemy_index.getNumElements(), 1.0);
    std::vector<pybind11::dict> norm_function_input(m_alchemy_index.getNumElements(),pybind11::dict());
    for (unsigned int i = 0; i < m_alchemy_index.getW(); i++)
        for (unsigned int j = 0; j <= i; j++)
            {
            unsigned int idx = m_alchemy_index(i, j);
            norm_function_input[idx]
                = evaluator::alchemParams(m_params[m_typpair_idx(i, j)], pkg.alphas[idx]).asDict();
            norm_function_input[idx]["mask"] = pkg.compute_mask[idx].to_string();
            norm_function_input[idx]["pair"] = std::pair<unsigned int, unsigned int>(i, j);
            }
    std::vector<Scalar> norm_function_output = m_normalizer(pybind11::cast(norm_function_input));
    pkg.normalization_values.swap(norm_function_output);

    return pkg;
    }

template<class evaluator, typename extra_pkg, typename alpha_particle_type>
inline void
PotentialPairAlchemicalNormalized<evaluator, extra_pkg, alpha_particle_type>::pkgPerNeighbor(
    const unsigned int& i,
    const unsigned int& j,
    const unsigned int& typei,
    const unsigned int& typej,
    const bool& in_rcut,
    evaluator& eval,
    extra_pkg& pkg)
    {
    PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>::pkgPerNeighbor(i,
                                                                                       j,
                                                                                       typei,
                                                                                       typej,
                                                                                       in_rcut,
                                                                                       eval,
                                                                                       pkg);
    eval.setNormalizationValue(pkg.normalization_values[m_alchemy_index(typei, typej)]);
    }

template<class evaluator, typename extra_pkg, typename alpha_particle_type>
inline void
PotentialPairAlchemicalNormalized<evaluator, extra_pkg, alpha_particle_type>::pkgFinalize(
    extra_pkg& pkg)
    {
    // for all used variables, store the net force
    for (unsigned int i = 0; i < m_alchemy_index.getNumElements(); i++)
        for (unsigned int j = 0; j < evaluator::num_alchemical_parameters; j++)
            if (pkg.compute_mask[i][j])
                {
                std::shared_ptr<alpha_particle_type>& alpha_p
                    = m_alchemical_particles[j * m_alchemy_index.getNumElements() + i];
                alpha_p->setNetForce(alpha_p->alchemical_derivative_normalization_value);
                }
    }

//! Export this pair potential to python
/*! \param name Name of the class in the exported python module
    \tparam T Class type to export. \b Must be an instantiated PotentialPairAlchemical class
   template.
*/
template<class evaluator_base,
         typename extra_pkg = AlchemyPackageNormalized<evaluator_base>,
         typename alpha_particle_type = AlchemicalNormalizedPairParticle>
void export_PotentialPairAlchemicalNormalized(pybind11::module& m, const std::string& name)
    {
    typedef Normalized<evaluator_base> evaluator;
    typedef PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type> base;
    typedef PotentialPairAlchemicalNormalized<evaluator, extra_pkg, alpha_particle_type> T;
    export_PotentialPairAlchemical<evaluator, extra_pkg, alpha_particle_type>(
        m,
        name + std::string("NormBase").c_str());
    pybind11::class_<T, base, std::shared_ptr<T>> PotentialPairAlchemicalNormalized(m,
                                                                                    name.c_str());
    PotentialPairAlchemicalNormalized
        .def(pybind11::init<std::shared_ptr<SystemDefinition>,
                            std::shared_ptr<NeighborList>>())
        .def("setNormalizer", &T::setNormalizer);
    }

#endif // POTENTIAL_PAIR_ALCHEMICAL_NORMALIZED_H
