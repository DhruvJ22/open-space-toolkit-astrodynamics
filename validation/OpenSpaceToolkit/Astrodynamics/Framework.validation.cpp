/// Apache License 2.0

#include <gtest/gtest.h>

#include <OpenSpaceToolkit/Core/Containers/Array.hpp>
#include <OpenSpaceToolkit/Core/Containers/Dictionary.hpp>
#include <OpenSpaceToolkit/Core/Containers/Table.hpp>
#include <OpenSpaceToolkit/Core/Containers/Tuple.hpp>
#include <OpenSpaceToolkit/Core/FileSystem/File.hpp>
#include <OpenSpaceToolkit/Core/FileSystem/Path.hpp>
#include <OpenSpaceToolkit/Core/Types/Integer.hpp>
#include <OpenSpaceToolkit/Core/Types/Real.hpp>
#include <OpenSpaceToolkit/Core/Types/Shared.hpp>
#include <OpenSpaceToolkit/Core/Types/Size.hpp>
#include <OpenSpaceToolkit/Core/Types/String.hpp>

#include <OpenSpaceToolkit/Mathematics/Geometry/3D/Objects/Composite.hpp>
#include <OpenSpaceToolkit/Mathematics/Geometry/3D/Objects/Cuboid.hpp>
#include <OpenSpaceToolkit/Mathematics/Geometry/3D/Objects/Point.hpp>
#include <OpenSpaceToolkit/Mathematics/Objects/Vector.hpp>

#include <OpenSpaceToolkit/Physics/Data/Scalar.hpp>
#include <OpenSpaceToolkit/Physics/Environment.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Atmospheric/Earth.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Atmospheric/Earth/CSSISpaceWeather.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Atmospheric/Earth/Exponential.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Atmospheric/Earth/Manager.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Ephemerides/Analytical.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Gravitational/Earth.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Magnetic/Earth.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Object.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Objects/CelestialBodies/Earth.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Objects/CelestialBodies/Moon.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Objects/CelestialBodies/Sun.hpp>
#include <OpenSpaceToolkit/Physics/Time/DateTime.hpp>
#include <OpenSpaceToolkit/Physics/Time/Duration.hpp>
#include <OpenSpaceToolkit/Physics/Time/Instant.hpp>
#include <OpenSpaceToolkit/Physics/Time/Interval.hpp>
#include <OpenSpaceToolkit/Physics/Time/Scale.hpp>
#include <OpenSpaceToolkit/Physics/Time/Time.hpp>
#include <OpenSpaceToolkit/Physics/Units/Derived.hpp>
#include <OpenSpaceToolkit/Physics/Units/Length.hpp>
#include <OpenSpaceToolkit/Physics/Units/Mass.hpp>

#include <OpenSpaceToolkit/Astrodynamics/CrossValidator.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/AtmosphericDrag.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/CentralBodyGravity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/PositionDerivative.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/ThirdBodyGravity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/Thruster.hpp>
#include <OpenSpaceToolkit/Astrodynamics/EventCondition/InstantCondition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/GuidanceLaw/ConstantThrust.hpp>
#include <OpenSpaceToolkit/Astrodynamics/GuidanceLaw/QLaw.hpp>
#include <OpenSpaceToolkit/Astrodynamics/MissionSequence.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Parser.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Solvers/FiniteDifferenceSolver.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/LocalOrbitalFrameDirection.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Orbit.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Orbit/Model.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Orbit/Models/Kepler/COE.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Propagator.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Segment.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Sequence.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesBroker.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubset.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianPosition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianVelocity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/NumericalSolver.hpp>

using ostk::core::ctnr::Array;
using ostk::core::ctnr::Table;
using ostk::core::ctnr::Tuple;
using CntrObject = ostk::core::ctnr::Object;
using ostk::core::ctnr::Dictionary;
using ostk::core::filesystem::File;
using ostk::core::filesystem::Path;
using ostk::core::types::Integer;
using ostk::core::types::Real;
using ostk::core::types::Shared;
using ostk::core::types::String;
using ostk::core::types::Size;

using ostk::math::geometry::d3::objects::Composite;
using ostk::math::geometry::d3::objects::Cuboid;
using ostk::math::geometry::d3::objects::Point;
using ostk::math::object::Matrix3d;
using ostk::math::object::Vector3d;
using ostk::math::object::VectorXd;

using ostk::physics::data::Scalar;
using ostk::physics::Environment;
using ostk::physics::coord::Frame;
using ostk::physics::coord::Position;
using ostk::physics::coord::Velocity;
using ostk::physics::environment::Object;
using ostk::physics::environment::object::Celestial;
using ostk::physics::environment::object::celestial::Earth;
using ostk::physics::environment::object::celestial::Moon;
using ostk::physics::environment::object::celestial::Sun;
using ostk::physics::environment::ephemerides::Analytical;
using ostk::physics::time::DateTime;
using ostk::physics::time::Duration;
using ostk::physics::time::Instant;
using ostk::physics::time::Interval;
using ostk::physics::time::Time;
using ostk::physics::time::Scale;
using ostk::physics::units::Angle;
using ostk::physics::units::Derived;
using ostk::physics::units::Length;
using ostk::physics::units::Mass;
using ostk::physics::environment::object::celestial::Earth;
using EarthGravitationalModel = ostk::physics::environment::gravitational::Earth;
using EarthMagneticModel = ostk::physics::environment::magnetic::Earth;
using EarthAtmosphericModel = ostk::physics::environment::atmospheric::Earth;
using EarthSpaceWeatherManager = ostk::physics::environment::atmospheric::earth::Manager;
using ostk::physics::environment::atmospheric::earth::CSSISpaceWeather;

using ostk::astro::eventcondition::InstantCondition;
using ostk::astro::trajectory::State;
using ostk::astro::trajectory::Sequence;
using ostk::astro::trajectory::Segment;
using ostk::astro::trajectory::Propagator;
using ostk::astro::trajectory::Orbit;
using ostk::astro::trajectory::LocalOrbitalFrameFactory;
using ostk::astro::trajectory::LocalOrbitalFrameDirection;
using ostk::astro::trajectory::state::NumericalSolver;
using ostk::astro::trajectory::state::CoordinatesSubset;
using ostk::astro::trajectory::state::CoordinatesBroker;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianPosition;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianVelocity;
using ostk::astro::trajectory::state::NumericalSolver;
using ostk::astro::trajectory::orbit::models::kepler::COE;
using ostk::astro::Dynamics;
using ostk::astro::flight::system::PropulsionSystem;
using ostk::astro::flight::system::SatelliteSystem;
using ostk::astro::dynamics::PositionDerivative;
using ostk::astro::dynamics::CentralBodyGravity;
using ostk::astro::dynamics::ThirdBodyGravity;
using ostk::astro::dynamics::AtmosphericDrag;
using ostk::astro::dynamics::Thruster;
using ostk::astro::guidancelaw::ConstantThrust;
using ostk::astro::guidancelaw::QLaw;
using ostk::astro::solvers::FiniteDifferenceSolver;

using ostk::astro::validation::Quantity;
using ostk::astro::validation::Tool;
using ostk::astro::validation::ToolComparison;
using ostk::astro::validation::QuantityComparison;
using ostk::astro::validation::MissionSequence;
using ostk::astro::validation::CrossValidator;
using ostk::astro::validation::Parser;

class OpenSpaceToolkit_Astrodynamics_Validation
    : public ::testing::TestWithParam<std::tuple<String, Array<ToolComparison>>>
{
   protected:
    const String pathToData = {"/app/validation/OpenSpaceToolkit/Astrodynamics/data"};
};

TEST_P(OpenSpaceToolkit_Astrodynamics_Validation, ValidationTestRunner)
{
    // Access the test parameters
    const auto parameters = GetParam();

    const String scenarioName = std::get<0>(parameters);
    const Array<ToolComparison> toolComparisons = std::get<1>(parameters);

    // Instatiate mission sequence app to set up the scenario
    MissionSequence missionSequence = {Parser::ParseYaml(String::Format("{0}/scenarios", pathToData), scenarioName)};

    // Solve the scenario
    missionSequence.run();

    // Compare with each reference tool
    for (const ToolComparison& toolComparison : toolComparisons)
    {
        const Tool tool = toolComparison.tool;
        const Array<QuantityComparison> quantityComparisons = toolComparison.quantityComparisons;

        // Compare output quantities with reference tool
        const Array<VectorXd> allDeltasWithTool =
            missionSequence.compareResults(Parser::ParseCSV(pathToData, scenarioName, tool), toolComparison);

        for (Size coordinateSubsetIndex = 0; coordinateSubsetIndex < quantityComparisons.getSize();
             coordinateSubsetIndex++)
        {
            const Quantity quantity = quantityComparisons[coordinateSubsetIndex].quantity;
            const Real tolerance = quantityComparisons[coordinateSubsetIndex].tolerance;

            const Size maxIndex = CrossValidator::FindMaxDeltaIndex(allDeltasWithTool, coordinateSubsetIndex);
            const Real maxDelta = allDeltasWithTool[maxIndex][coordinateSubsetIndex];

            // Assert tolerance
            EXPECT_LT(maxDelta, tolerance) << String::Format(
                "Scenario: {}\nFor Tool: {}\nFor Quantity: {}\nTolerance: {}\nState Index: {} out of {}\n",
                scenarioName,
                CrossValidator::ToolToString(tool),
                CrossValidator::QuantityToString(quantity),
                tolerance,
                maxIndex,
                allDeltasWithTool.getSize() - 1
            );
        }
    }
}

static const std::vector<std::tuple<String, Array<ToolComparison>>> testCases_ForceModel = {
    {
        "001-force-model-spherical-a",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "001-force-model-spherical-b",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.3e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.3e-3},
                },
            },
        },
    },
    {
        "001-force-model-spherical-c",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.3e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.3e-3},
                },
            },
        },
    },
    {
        "002-force-model-non-spherical-60x60",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "002-force-model-non-spherical-360x360",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "003-force-model-exponential-320",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 2.3e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.7e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "003-force-model-exponential-500",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "003-force-model-exponential-600",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "003-force-model-nrlmsis-470-small-area",
        {
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.6e+1},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.8e-2},
                },
            },
        },
    },
    {
        "003-force-model-nrlmsis-470-large-area-short-duration",
        {
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 6.0e+0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 6.0e-3},
                },
            },
        },
    },
    {
        "004-force-model-sun-moon",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "004-force-model-sun",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "004-force-model-moon",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "005-force-model-all-perturbs",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 2.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.2e-3},
                },
            },
        },
    },
    {
        "006-force-model-constant-thrust",  // Constant thruster maneuver in In-Track direction
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 0.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 0.9e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.0e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.1e-3},
                },
            },
        },
    },
    {
        "006-force-model-constant-thrust-exponential",  // Constant thruster maneuver in In-Track direction
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.5e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.7e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.0e-3},
                },
            },
        },
    },
};
INSTANTIATE_TEST_SUITE_P(
    ForceModelValidation, OpenSpaceToolkit_Astrodynamics_Validation, ::testing::ValuesIn(testCases_ForceModel)
);

static const std::vector<std::tuple<String, Array<ToolComparison>>> testCases_Thruster = {
    {
        "010-thruster-direction-crosstrack",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.1e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.2e-3},
                },
            },
        },
    },
    {
        "010-thruster-direction-in-cross-radial",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.6e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.8e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.1e-3},
                },
            },
        },
    },
    {
        "010-thruster-direction-intrack",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.5e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.7e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.0e-3},
                },
            },
        },
    },
    {
        "010-thruster-direction-radial",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.1e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.2e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-drag-decrease",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-drag-increase",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 120.0e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 130.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 120.0e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 130.0e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-mass-decrease",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 12.0e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 12.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 4.0e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 4.1e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-mass-increase",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.1e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.2e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 1.3e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-thrust-decrease-isp-increase",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.1e-3},
                },
            },
        },
    },
    {
        "011-thruster-params-thrust-increase-isp-decrease",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 10.6e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 4.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 2.2e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.4e-3},
                },
            },
        },
    },
};
INSTANTIATE_TEST_SUITE_P(
    ThrusterValidation, OpenSpaceToolkit_Astrodynamics_Validation, ::testing::ValuesIn(testCases_Thruster)
);

static const std::vector<std::tuple<String, Array<ToolComparison>>> testCases_Sequence = {
    {
        "020-sequence-multiple-2h-maneuvers",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.2e-3},
                },
            },
        },
    },
    {
        "020-sequence-multiple-30m-maneuvers",
        {
            {
                Tool::GMAT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.8e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.0e-3},
                },
            },
            {
                Tool::OREKIT,
                {
                    {Quantity::CARTESIAN_POSITION_GCRF, 1.9e-0},
                    {Quantity::CARTESIAN_VELOCITY_GCRF, 2.2e-3},
                },
            },
        },
    },
};
INSTANTIATE_TEST_SUITE_P(
    SequenceValidation, OpenSpaceToolkit_Astrodynamics_Validation, ::testing::ValuesIn(testCases_Sequence)
);
