// Copyright (c) 2009-2022 The Regents of the University of Michigan.
// Part of HOOMD-blue, released under the BSD 3-Clause License.

#ifdef __HIPCC__
#error This header cannot be compiled by nvcc
#endif

#include <functional>
#include <memory>

#include "WallData.h"
// Export all wall data types into Python. This is needed to allow for syncing Python and C++
// list/array data structures containing walls for WallPotential objects.
void hoomd::md::export_wall_data(pybind11::module& m)
    {
    pybind11::class_<hoomd::md::SphereWall>(m, "SphereWall")
        .def(pybind11::init(
                 [](hoomd::Scalar radius, pybind11::tuple origin, bool inside, bool open)
                 {
                     return hoomd::md::SphereWall(
                         radius,
                         hoomd::make_scalar3(origin[0].cast<hoomd::Scalar>(),
                                             origin[1].cast<hoomd::Scalar>(),
                                             origin[2].cast<hoomd::Scalar>()),
                         inside,
                         open);
                 }),
             pybind11::arg("radius"),
             pybind11::arg("origin"),
             pybind11::arg("inside"),
             pybind11::arg("open"))
        .def_property_readonly("radius", [](const hoomd::md::SphereWall& wall) { return wall.r; })
        .def_property_readonly(
            "origin",
            [](const hoomd::md::SphereWall& wall)
            { return pybind11::make_tuple(wall.origin.x, wall.origin.y, wall.origin.z); })
        .def_property_readonly("inside",
                               [](const hoomd::md::SphereWall& wall) { return wall.inside; })
        .def_property_readonly("open", [](const hoomd::md::SphereWall& wall) { return wall.open; });

    pybind11::class_<hoomd::md::CylinderWall>(m, "CylinderWall")
        .def(pybind11::init(
                 [](hoomd::Scalar radius,
                    pybind11::tuple origin,
                    pybind11::tuple z_orientation,
                    bool inside,
                    bool open)
                 {
                     return hoomd::md::CylinderWall(
                         radius,
                         hoomd::make_scalar3(origin[0].cast<hoomd::Scalar>(),
                                             origin[1].cast<hoomd::Scalar>(),
                                             origin[2].cast<hoomd::Scalar>()),
                         hoomd::make_scalar3(z_orientation[0].cast<hoomd::Scalar>(),
                                             z_orientation[1].cast<hoomd::Scalar>(),
                                             z_orientation[2].cast<hoomd::Scalar>()),
                         inside,
                         open);
                 }),
             pybind11::arg("radius"),
             pybind11::arg("origin"),
             pybind11::arg("axis"),
             pybind11::arg("inside"),
             pybind11::arg("open"))
        .def_property_readonly("radius", [](const hoomd::md::CylinderWall& wall) { return wall.r; })
        .def_property_readonly(
            "origin",
            [](const hoomd::md::CylinderWall& wall)
            { return pybind11::make_tuple(wall.origin.x, wall.origin.y, wall.origin.z); })
        .def_property_readonly(
            "axis",
            [](const hoomd::md::CylinderWall& wall)
            { return pybind11::make_tuple(wall.axis.x, wall.axis.y, wall.axis.z); })
        .def_property_readonly("inside",
                               [](const hoomd::md::CylinderWall& wall) { return wall.inside; })
        .def_property_readonly("open",
                               [](const hoomd::md::CylinderWall& wall) { return wall.open; });

    pybind11::class_<hoomd::md::PlaneWall>(m, "PlaneWall")
        .def(pybind11::init(
                 [](pybind11::tuple origin, pybind11::tuple normal, bool open)
                 {
                     return hoomd::md::PlaneWall(
                         hoomd::make_scalar3(origin[0].cast<hoomd::Scalar>(),
                                             origin[1].cast<hoomd::Scalar>(),
                                             origin[2].cast<hoomd::Scalar>()),
                         hoomd::make_scalar3(normal[0].cast<hoomd::Scalar>(),
                                             normal[1].cast<hoomd::Scalar>(),
                                             normal[2].cast<hoomd::Scalar>()),
                         open);
                 }),
             pybind11::arg("origin"),
             pybind11::arg("normal"),
             pybind11::arg("open"))
        .def_property_readonly(
            "origin",
            [](const hoomd::md::PlaneWall& wall)
            { return pybind11::make_tuple(wall.origin.x, wall.origin.y, wall.origin.z); })
        .def_property_readonly(
            "normal",
            [](const hoomd::md::PlaneWall& wall)
            { return pybind11::make_tuple(wall.normal.x, wall.normal.y, wall.normal.z); })
        .def_property_readonly("open", [](const hoomd::md::PlaneWall& wall) { return wall.open; });
    }
