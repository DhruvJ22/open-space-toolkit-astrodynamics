/// Apache License 2.0

#include <OpenSpaceToolkitAstrodynamicsPy/Trajectory/Orbit/Models/Kepler.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Trajectory/Orbit/Models/Kepler/BrouwerLyddaneMean.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Trajectory/Orbit/Models/Propagated.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Trajectory/Orbit/Models/SGP4.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Trajectory/Orbit/Models/Tabulated.cpp>

inline void OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models(pybind11::module& aModule)
{
    // Create "models" python submodule
    auto models = aModule.def_submodule("models");

    // add objects to "models" submodule
    OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models_Kepler(models);
    OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models_SGP4(models);
    OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models_Tabulated(models);
    OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models_Propagated(models);
    OpenSpaceToolkitAstrodynamicsPy_Trajectory_Orbit_Models_BrouwerLyddaneMean(models);
}
