/// Apache License 2.0

#include <OpenSpaceToolkitAstrodynamicsPy/Flight/Profile/Models/Tabulated.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Flight/Profile/Models/Transform.cpp>

inline void OpenSpaceToolkitAstrodynamicsPy_Flight_Profile_Models(pybind11::module& aModule)
{
    // Create "models" python submodule
    auto models = aModule.def_submodule("models");

    // add objects to "models" submodule
    OpenSpaceToolkitAstrodynamicsPy_Flight_Profile_Models_Transform(models);
    OpenSpaceToolkitAstrodynamicsPy_Flight_Profile_Models_Tabulated(models);
}
