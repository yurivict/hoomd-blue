// Copyright (c) 2009-2022 The Regents of the University of Michigan.
// Part of HOOMD-blue, released under the BSD 3-Clause License.

#include "Profiler.h"
#include "SharedSignal.h"
#include "SystemDefinition.h"

#include <memory>
#include <string>
#include <vector>

#ifndef __COMPUTE_H__
#define __COMPUTE_H__

/*! \file Compute.h
    \brief Declares a base class for all computes
*/

#ifdef __HIPCC__
#error This header cannot be compiled by nvcc
#endif

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

/*! \ingroup hoomd_lib
    @{
*/

/*! \defgroup computes Computes
    \brief All classes that implement the Compute concept.
    \details See \ref page_dev_info for more information
*/

/*! @}
 */

namespace hoomd
    {
//! Performs computations on ParticleData structures
/*! The Compute is an abstract concept that performs some kind of computation on the
    particles in a ParticleData structure. This computation is to be done by reading
    the particle data only, no writing. Computes will be used to generate neighbor lists,
    calculate forces, and calculate temperatures, just to name a few.

    For performance and simplicity, each compute is associated with a ParticleData
    on construction. ParticleData pointers are managed with reference counted std::shared_ptr.
    Since each ParticleData cannot change size, this allows the Compute to preallocate
    any data structures that it may need.

    Computes may be referenced more than once and may reference other computes. To prevent
    unneeded data from being calculated, the time step will be passed into the compute
    method so that it can skip calculations if they have already been done this timestep.
    For convenience, the base class will provide a shouldCompute() method that implements
    this behaviour. Derived classes can override if more complicated behavior is needed.

    See \ref page_dev_info for more information
    \ingroup computes
*/
class PYBIND11_EXPORT Compute
    {
    public:
    //! Constructs the compute and associates it with the ParticleData
    Compute(std::shared_ptr<SystemDefinition> sysdef);
    virtual ~Compute() { }

    //! Abstract method that performs the computation
    /*! \param timestep Current time step
        Derived classes will implement this method to calculate their results
    */
    virtual void compute(uint64_t timestep) { }

    //! Abstract method that performs a benchmark
    virtual double benchmark(unsigned int num_iters);

    //! Reset stat counters
    /*! If derived classes provide statistics for the last run, they should resetStats() to
        clear any counters. System will reset the stats before any run() so that stats printed
        at the end of the run only apply to that run() alone.
    */
    virtual void resetStats() { }

    //! Sets the profiler for the compute to use
    virtual void setProfiler(std::shared_ptr<Profiler> prof);

    //! Set autotuner parameters
    /*! \param enable Enable/disable autotuning
        \param period period (approximate) in time steps when returning occurs

        Derived classes should override this to set the parameters of their autotuners.
    */
    virtual void setAutotunerParams(bool enable, unsigned int period) { }

    //! Force recalculation of compute
    /*! If this function is called, recalculation of the compute will be forced (even if had
     *  been calculated earlier in this timestep)
     * \param timestep current timestep
     */
    void forceCompute(uint64_t timestep);

    /// Python will notify C++ objects when they are detached from Simulation
    virtual void notifyDetach() {};

    void addSlot(std::shared_ptr<hoomd::detail::SignalSlot> slot)
        {
        m_slots.push_back(slot);
        }

    void removeDisconnectedSlots()
        {
        for (unsigned int i = 0; i < m_slots.size();)
            {
            if (!m_slots[i]->connected())
                {
                m_exec_conf->msg->notice(8) << "Found dead signal @" << std::hex << m_slots[i].get()
                                            << std::dec << std::endl;
                m_slots.erase(m_slots.begin() + i);
                }
            else
                {
                i++;
                }
            }
        }

    protected:
    const std::shared_ptr<SystemDefinition>
        m_sysdef; //!< The system definition this compute is associated with
    const std::shared_ptr<ParticleData>
        m_pdata;                      //!< The particle data this compute is associated with
    std::shared_ptr<Profiler> m_prof; //!< The profiler this compute is to use
    std::shared_ptr<const ExecutionConfiguration>
        m_exec_conf; //!< Stored shared ptr to the execution configuration
    std::vector<std::shared_ptr<hoomd::detail::SignalSlot>>
        m_slots;              //!< Stored shared ptr to the system signals
    bool m_force_compute;     //!< true if calculation is enforced
    uint64_t m_last_computed; //!< Stores the last timestep compute was called
    bool m_first_compute;     //!< true if compute has not yet been called

    //! Simple method for testing if the computation should be run or not
    virtual bool shouldCompute(uint64_t timestep);

    //! Peek to see if computation should be run without updating internal state
    virtual bool peekCompute(uint64_t timestep) const;

    private:
    //! The python export needs to be a friend to export shouldCompute()
    friend void export_Compute();
    };

namespace detail
    {
//! Exports the Compute class to python
#ifndef __HIPCC__
void export_Compute(pybind11::module& m);
#endif
    } // end namespace detail

    } // end namespace hoomd
#endif
