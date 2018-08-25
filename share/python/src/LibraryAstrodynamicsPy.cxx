////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @project        Library/Astrodynamics
/// @file           LibraryAstrodynamicsPy.cxx
/// @author         Lucas Brémond <lucas@loftorbital.com>
/// @license        TBD

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/python.hpp>

#include <LibraryAstrodynamicsPy/Access.cpp>
#include <LibraryAstrodynamicsPy/Trajectory.cpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_PYTHON_MODULE (LibraryAstrodynamicsPy)
{

	boost::python::object package = boost::python::scope() ;
	
	package.attr("__path__") = "Library" ;

	LibraryAstrodynamicsPy_Trajectory() ;
	LibraryAstrodynamicsPy_Access() ;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////