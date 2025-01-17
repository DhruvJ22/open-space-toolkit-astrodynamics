/// Apache License 2.0

#include <OpenSpaceToolkitAstrodynamicsPy/Solvers/FiniteDifferenceSolver.cpp>
#include <OpenSpaceToolkitAstrodynamicsPy/Solvers/TemporalConditionSolver.cpp>

inline void OpenSpaceToolkitAstrodynamicsPy_Solvers(pybind11::module& aModule)
{
    // Create "solvers" python submodule
    auto solvers = aModule.def_submodule("solvers");

    // Add objects to "solvers" submodule
    OpenSpaceToolkitAstrodynamicsPy_Solvers_TemporalConditionSolver(solvers);
    OpenSpaceToolkitAstrodynamicsPy_Solvers_FiniteDifferenceSolver(solvers);
}
