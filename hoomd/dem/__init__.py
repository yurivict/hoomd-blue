# Copyright (c) 2009-2022 The Regents of the University of Michigan.
# Part of HOOMD-blue, released under the BSD 3-Clause License.

R"""Simulate rounded, faceted shapes in molecular dynamics.

The DEM component provides forces which apply short-range, purely
repulsive interactions between contact points of two shapes. The
resulting interaction is consistent with expanding the given polygon
or polyhedron by a disk or sphere of a particular rounding radius.

The pair forces located in :py:mod:`hoomd.dem.pair` behave like other
hoomd pair forces, computing forces and torques for each particle
based on its interactions with its neighbors. Also included are
geometric helper utilities in :py:mod:`hoomd.dem.utils`.

Initialization
--------------

When initializing systems, be sure to set the inertia tensor of DEM
particles. Axes with an inertia tensor of 0 (the default) will not
have their rotational degrees of freedom integrated. Because only the
three principal components of inertia are given to hoomd, particle
vertices should also be specified in the principal reference frame so
that the inertia tensor is diagonal.

Example::

    snap = hoomd.data.make_snapshot(512, box=hoomd.data.boxdim(L=10))
    snap.particles.moment_inertia[:] = (10, 10, 10)
    system = hoomd.init.read_snapshot(snap)

Integration
-----------

To allow particles to rotate, use integrators which can update
rotational degrees of freedom:

  * `hoomd.md.methods.NVE`
  * `hoomd.md.methods.NVT`
  * `hoomd.md.methods.NPT`
  * `hoomd.md.methods.Langevin`
  * `hoomd.md.methods.Brownian`

Note that the Nosé-Hoover thermostats used in
`hoomd.md.methods.NVT` and `hoomd.md.methods.NPT`
work by rescaling momenta and angular momenta. This can lead to
instabilities in the start of the simulation if particles are
initialized with 0 angular momentum and no neighbor interactions. Two
easy fixes for this problem are to initialize each particle with some
angular momentum or to first run for a few steps with
`hoomd.md.methods.Langevin` or
`hoomd.md.methods.Brownian`.

Data Storage
------------

To store trajectories of DEM systems, use a format that knows about
anisotropic particles, such as:

  * :py:class:`hoomd.dump.getar`
  * :py:class:`hoomd.dump.GSD`

.. rubric:: Stability

:py:mod:`hoomd.dem` is **stable**. When upgrading from version 2.x to 2.y (y > x),
existing job scripts that follow *documented* interfaces for functions and classes


"""

# this file exists to mark this directory as a python module

# need to import all submodules defined in this directory

from hoomd.dem import pair
from hoomd.dem import params
from hoomd.dem import utils
