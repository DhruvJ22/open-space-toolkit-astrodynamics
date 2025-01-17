/// Apache License 2.0

#include <OpenSpaceToolkit/Physics/Coordinate/Frame.hpp>
#include <OpenSpaceToolkit/Physics/Coordinate/Position.hpp>
#include <OpenSpaceToolkit/Physics/Coordinate/Velocity.hpp>
#include <OpenSpaceToolkit/Physics/Environment/Objects/CelestialBodies/Earth.hpp>
#include <OpenSpaceToolkit/Physics/Time/Duration.hpp>
#include <OpenSpaceToolkit/Physics/Time/Instant.hpp>
#include <OpenSpaceToolkit/Physics/Time/Scale.hpp>

#include <OpenSpaceToolkit/Astrodynamics/Dynamics/AtmosphericDrag.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/CentralBodyGravity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/PositionDerivative.hpp>
#include <OpenSpaceToolkit/Astrodynamics/EventCondition/COECondition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/EventCondition/InstantCondition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/GuidanceLaw/ConstantThrust.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Segment.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianPosition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianVelocity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/NumericalSolver.hpp>

#include <Global.test.hpp>

using ostk::core::ctnr::Map;
using ostk::core::ctnr::Array;
using ostk::core::types::String;
using ostk::core::types::Shared;
using ostk::core::types::Real;
using ostk::core::types::Size;

using ostk::math::object::VectorXd;
using ostk::math::object::MatrixXd;

using ostk::physics::environment::object::Celestial;
using ostk::physics::time::Instant;
using ostk::physics::time::Duration;
using ostk::physics::time::DateTime;
using ostk::physics::time::Scale;
using ostk::physics::environment::object::Celestial;
using ostk::physics::environment::object::celestial::Earth;
using ostk::physics::coord::Frame;
using ostk::physics::coord::Position;
using ostk::physics::coord::Velocity;
using EarthGravitationalModel = ostk::physics::environment::gravitational::Earth;

using ostk::astro::trajectory::state::NumericalSolver;
using ostk::astro::Dynamics;
using ostk::astro::flight::system::SatelliteSystem;
using ostk::astro::dynamics::Thruster;
using ostk::astro::guidancelaw::ConstantThrust;
using ostk::astro::trajectory::Segment;
using ostk::astro::trajectory::LocalOrbitalFrameDirection;
using ostk::astro::trajectory::LocalOrbitalFrameFactory;
using ostk::astro::Dynamics;
using ostk::astro::dynamics::AtmosphericDrag;
using ostk::astro::dynamics::CentralBodyGravity;
using ostk::astro::dynamics::PositionDerivative;
using ostk::astro::eventcondition::InstantCondition;
using ostk::astro::eventcondition::COECondition;
using ostk::astro::eventcondition::RealCondition;
using ostk::astro::trajectory::State;
using ostk::astro::trajectory::state::CoordinatesBroker;
using ostk::astro::trajectory::state::CoordinatesSubset;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianPosition;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianVelocity;
using EarthGravitationalModel = ostk::physics::environment::gravitational::Earth;
using EarthMagneticModel = ostk::physics::environment::magnetic::Earth;
using EarthAtmosphericModel = ostk::physics::environment::atmospheric::Earth;

class OpenSpaceToolkit_Astrodynamics_Trajectory_Segment : public ::testing::Test
{
    void SetUp() override
    {
        VectorXd initialCoordinates(7);
        initialCoordinates << 7000000.0, 0.0, 0.0, 0.0, 7546.05329, 0.0, 200.0;
        initialStateWithMass_ = {Instant::J2000(), initialCoordinates, Frame::GCRF(), thrustCoordinatesBrokerSPtr_};

        VectorXd finalCoordinates(7);
        finalCoordinates << 7000000.0, 0.0, 0.0, 0.0, 7546.05329, 0.0, 180.0;
        finalStateWithMass_ = {Instant::J2000(), finalCoordinates, Frame::GCRF(), thrustCoordinatesBrokerSPtr_};
    }

   protected:
    const State defaultState_ = {
        Instant::DateTime(DateTime(2021, 3, 20, 12, 0, 0), Scale::UTC),
        Position::Meters({7000000.0, 0.0, 0.0}, Frame::GCRF()),
        Velocity::MetersPerSecond({0.0, 7546.05329, 0.0}, Frame::GCRF()),
    };

    const String defaultName_ = "A Segment";

    const Shared<Celestial> earthSpherical_ = std::make_shared<Celestial>(Earth::Spherical());
    const Array<Shared<Dynamics>> defaultDynamics_ = {
        std::make_shared<PositionDerivative>(),
        std::make_shared<CentralBodyGravity>(earthSpherical_),
    };
    const NumericalSolver defaultNumericalSolver_ = {
        NumericalSolver::LogType::NoLog,
        NumericalSolver::StepperType::RungeKuttaDopri5,
        1e-2,
        1.0e-15,
        1.0e-15,
    };
    const Shared<InstantCondition> defaultInstantCondition_ = std::make_shared<InstantCondition>(
        InstantCondition::Criterion::AnyCrossing, defaultState_.accessInstant() + Duration::Minutes(15.0)
    );

    const Shared<const ConstantThrust> constantThrustSPtr_ = std::make_shared<ConstantThrust>(
        LocalOrbitalFrameDirection({1.0, 0.0, 0.0}, LocalOrbitalFrameFactory::VNC(Frame::GCRF()))
    );

    const Shared<Thruster> defaultConstantThrust_ =
        std::make_shared<Thruster>(SatelliteSystem::Default(), constantThrustSPtr_);

    const Segment defaultCoastSegment_ =
        Segment::Coast(defaultName_, defaultInstantCondition_, defaultDynamics_, defaultNumericalSolver_);

    const Shared<CoordinatesBroker> thrustCoordinatesBrokerSPtr_ =
        std::make_shared<CoordinatesBroker>(CoordinatesBroker({
            CartesianPosition::Default(),
            CartesianVelocity::Default(),
            CoordinatesSubset::Mass(),
        }));

    State initialStateWithMass_ = State::Undefined();
    State finalStateWithMass_ = State::Undefined();
};

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_Constructor)
{
    {
        EXPECT_NO_THROW(Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast));
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_Accessors)
{
    {
        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {defaultState_, defaultState_}, true, Segment::Type::Coast
        );

        EXPECT_NO_THROW(segmentSolution.accessStartInstant());
        EXPECT_NO_THROW(segmentSolution.accessEndInstant());
    }

    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast);

        EXPECT_THROW(segmentSolution.accessStartInstant(), ostk::core::error::RuntimeError);
        EXPECT_THROW(segmentSolution.accessEndInstant(), ostk::core::error::RuntimeError);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_Getters)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast);

        EXPECT_THROW(segmentSolution.getInitialMass(), ostk::core::error::RuntimeError);
        EXPECT_THROW(segmentSolution.getFinalMass(), ostk::core::error::RuntimeError);
        EXPECT_THROW(segmentSolution.getPropagationDuration(), ostk::core::error::RuntimeError);
    }

    {
        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        EXPECT_DOUBLE_EQ(200.0, segmentSolution.getInitialMass().inKilograms());
        EXPECT_DOUBLE_EQ(180.0, segmentSolution.getFinalMass().inKilograms());
        EXPECT_DOUBLE_EQ(0.0, segmentSolution.getPropagationDuration().inSeconds());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_ComputeDeltaMass)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast);

        EXPECT_DOUBLE_EQ(0.0, segmentSolution.computeDeltaMass().inKilograms());
    }

    {
        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        EXPECT_DOUBLE_EQ(20.0, segmentSolution.computeDeltaMass().inKilograms());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_ComputeDeltaV)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast);

        EXPECT_DOUBLE_EQ(0.0, segmentSolution.computeDeltaV(1500.0));
    }

    {
        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        // 1500 * 9.80665 * ln(200 / 180) -> Rocket equation
        EXPECT_DOUBLE_EQ(1549.850551313734, segmentSolution.computeDeltaV(1500.0));
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_CalculateStatesAt)
{
    // Test that the function throws when the segment has no states
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {}, true, Segment::Type::Coast);

        EXPECT_THROW(
            segmentSolution.calculateStatesAt({Instant::J2000()}, defaultNumericalSolver_),
            ostk::core::error::RuntimeError
        );
    }

    // Test that the returns an empty state array if an empty instant array is provided
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Array<State> propagatedStates = segmentSolution.calculateStatesAt({}, defaultNumericalSolver_);

        EXPECT_EQ(0, propagatedStates.getSize());
    }

    // Test that the function throws when the instant array is out of order
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        EXPECT_THROW(
            segmentSolution.calculateStatesAt({Instant::Now(), Instant::J2000()}, defaultNumericalSolver_),
            ostk::core::error::runtime::Wrong
        );
    }

    // Test that the function throws when an instant outside the segment is desired
    {
        const Array<Instant> instantArrayOutsideSegment = {
            defaultState_.getInstant(), defaultState_.getInstant() + Duration::Minutes(1.0)
        };
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        EXPECT_THROW(
            segmentSolution.calculateStatesAt(instantArrayOutsideSegment, defaultNumericalSolver_),
            ostk::core::error::RuntimeError
        );
    }

    // Test successfull result for propagation to states within segment including bounds
    {
        const Array<Instant> instantArray = {
            defaultState_.getInstant(),
            defaultState_.getInstant() + Duration::Minutes(1.0),
            defaultState_.getInstant() + Duration::Minutes(2.0)
        };

        const State state1 = {
            defaultState_.getInstant() + Duration::Minutes(2.0),
            defaultState_.getPosition(),
            defaultState_.getVelocity(),
        };

        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_, state1}, true, Segment::Type::Coast);

        const Array<State> propagatedStates = segmentSolution.calculateStatesAt(instantArray, defaultNumericalSolver_);

        for (Size i = 0; i < instantArray.getSize(); i++)
        {
            EXPECT_EQ(instantArray[i], propagatedStates[i].getInstant());
        }
        EXPECT_EQ(defaultState_, propagatedStates[0]);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_GetDynamicsContribution)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();
        for (const Shared<Dynamics>& dynamics : defaultDynamics_)
        {
            EXPECT_NO_THROW(segmentSolution.getDynamicsContribution(dynamics, stateFrame));
        }
    }

    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();
        for (const Shared<Dynamics>& dynamics : defaultDynamics_)
        {
            const MatrixXd contributionDefault = segmentSolution.getDynamicsContribution(dynamics, stateFrame);
            const Array<Shared<const CoordinatesSubset>> dynamicsWriteCoordinatesSubsets =
                dynamics->getWriteCoordinatesSubsets();
            const MatrixXd contributionExplicit =
                segmentSolution.getDynamicsContribution(dynamics, stateFrame, dynamicsWriteCoordinatesSubsets);
            EXPECT_EQ(contributionDefault, contributionExplicit);
        }
    }

    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();

        for (const Shared<Dynamics>& dynamics : defaultDynamics_)
        {
            const Array<Shared<const CoordinatesSubset>> dynamicsWriteCoordinatesSubsets =
                dynamics->getWriteCoordinatesSubsets();
            const Shared<const CoordinatesSubset> dynamicsWriteCoordinatesSubset = dynamicsWriteCoordinatesSubsets[0];
            const MatrixXd contribution =
                segmentSolution.getDynamicsContribution(dynamics, stateFrame, {dynamicsWriteCoordinatesSubset});
            EXPECT_EQ(contribution.cols(), dynamicsWriteCoordinatesSubset->getSize());
            EXPECT_EQ(contribution.rows(), segmentSolution.states.getSize());
        }
    }

    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();

        // Construct a coordinatesSubset not part of the dynamics for which the contribution is requested
        const Shared<Dynamics> dynamics = defaultDynamics_[0];
        const Shared<const CoordinatesSubset> coordinatesSubset = CoordinatesSubset::DragCoefficient();

        EXPECT_FALSE(dynamics->getWriteCoordinatesSubsets().contains(coordinatesSubset));

        const String expectedString =
            "Provided coordinates subset is not part of the dynamics write coordinates subsets.";

        // Test the throw and the message that is thrown
        EXPECT_THROW(
            {
                try
                {
                    MatrixXd contribution =
                        segmentSolution.getDynamicsContribution(dynamics, stateFrame, {coordinatesSubset});
                }
                catch (const ostk::core::error::runtime::Undefined& e)
                {
                    EXPECT_EQ(expectedString, e.getMessage());
                    throw;
                }
            },
            ostk::core::error::RuntimeError
        );
    }

    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        // Construct a dynamics not part of the segment
        const Earth earth = Earth::FromModels(
            std::make_shared<EarthGravitationalModel>(EarthGravitationalModel::Type::Spherical),
            std::make_shared<EarthMagneticModel>(EarthMagneticModel::Type::Undefined),
            std::make_shared<EarthAtmosphericModel>(EarthAtmosphericModel::Type::Exponential)
        );
        const Shared<Celestial> earthSPtr = std::make_shared<Celestial>(earth);
        const Shared<Dynamics> atmosphericDragDynamics = std::make_shared<AtmosphericDrag>(earthSPtr);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();

        const String expectedString = "Provided dynamics is not part of the segment dynamics.";

        // Test the throw and the message that is thrown
        EXPECT_THROW(
            {
                try
                {
                    MatrixXd contribution =
                        segmentSolution.getDynamicsContribution(atmosphericDragDynamics, stateFrame);
                }
                catch (const ostk::core::error::runtime::Undefined& e)
                {
                    EXPECT_EQ(expectedString, e.getMessage());
                    throw;
                }
            },
            ostk::core::error::RuntimeError
        );
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_GetDynamicsAccelerationContribution)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();

        // Check error for PositionDerivative
        const String expectedString =
            "Provided coordinates subset is not part of the dynamics write coordinates subsets.";

        // Test the throw and the message that is thrown
        EXPECT_THROW(
            {
                try
                {
                    MatrixXd accelerationContribution =
                        segmentSolution.getDynamicsAccelerationContribution(defaultDynamics_[0], stateFrame);
                }
                catch (const ostk::core::error::runtime::Undefined& e)
                {
                    EXPECT_EQ(expectedString, e.getMessage());
                    throw;
                }
            },
            ostk::core::error::RuntimeError
        );

        // Check output for Gravity
        MatrixXd accelerationContribution =
            segmentSolution.getDynamicsAccelerationContribution(defaultDynamics_[1], stateFrame);
        EXPECT_EQ(accelerationContribution.cols(), 3);
        EXPECT_EQ(accelerationContribution.rows(), segmentSolution.states.getSize());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_GetAllDynamicsContributions)
{
    {
        const Segment::Solution segmentSolution =
            Segment::Solution(defaultName_, defaultDynamics_, {defaultState_}, true, Segment::Type::Coast);

        const Shared<const Frame> stateFrame = defaultState_.accessFrame();
        Map<Shared<Dynamics>, MatrixXd> contributions = segmentSolution.getAllDynamicsContributions(stateFrame);

        EXPECT_EQ(defaultDynamics_.getSize(), contributions.size());
        for (const Shared<Dynamics>& dynamics : defaultDynamics_)
        {
            EXPECT_EQ(1, contributions.count(dynamics));  // Check all dynamics are present
            EXPECT_EQ(
                segmentSolution.states.getSize(), contributions.at(dynamics).rows()
            );  // Check the number of rows corresponds to the number of states
            EXPECT_GT(
                contributions.at(dynamics).cols(), dynamics->getWriteCoordinatesSubsets().getSize()
            );  // Check the number of columns corresponds to the number of coordinates subsets to which the dynamics
                // writes
        }
    }

    {
        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        const Shared<const Frame> stateFrame = initialStateWithMass_.accessFrame();
        Map<Shared<Dynamics>, MatrixXd> contributions = segmentSolution.getAllDynamicsContributions(stateFrame);

        EXPECT_EQ(defaultDynamics_.getSize(), contributions.size());
        for (const Shared<Dynamics>& dynamics : defaultDynamics_)
        {
            EXPECT_EQ(1, contributions.count(dynamics));  // Check all dynamics are present
            EXPECT_EQ(
                segmentSolution.states.getSize(), contributions.at(dynamics).rows()
            );  // Check the number of rows corresponds to the number of states
            EXPECT_GT(
                contributions.at(dynamics).cols(), dynamics->getWriteCoordinatesSubsets().getSize()
            );  // Check the number of columns corresponds to the number of coordinates subsets to which the dynamics
                // writes
        }
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_Print)
{
    {
        testing::internal::CaptureStdout();

        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        EXPECT_NO_THROW(segmentSolution.print(std::cout, true));
        EXPECT_NO_THROW(segmentSolution.print(std::cout, false));

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, SegmentSolution_StreamOperator)
{
    {
        testing::internal::CaptureStdout();

        const Segment::Solution segmentSolution = Segment::Solution(
            defaultName_, defaultDynamics_, {initialStateWithMass_, finalStateWithMass_}, true, Segment::Type::Maneuver
        );

        EXPECT_NO_THROW(std::cout << segmentSolution << std::endl);

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, Coast)
{
    {
        EXPECT_NO_THROW(
            Segment::Coast(defaultName_, defaultInstantCondition_, defaultDynamics_, defaultNumericalSolver_)
        );
    }

    {
        EXPECT_THROW(
            Segment::Coast(defaultName_, nullptr, defaultDynamics_, NumericalSolver::Undefined()),
            ostk::core::error::runtime::Undefined
        );
    }

    {
        EXPECT_THROW(
            Segment::Coast(defaultName_, defaultInstantCondition_, {}, NumericalSolver::Undefined()),
            ostk::core::error::runtime::Undefined
        );
    }

    {
        EXPECT_THROW(
            Segment::Coast(defaultName_, defaultInstantCondition_, defaultDynamics_, NumericalSolver::Undefined()),
            ostk::core::error::runtime::Undefined
        );
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, Maneuver)
{
    {
        EXPECT_NO_THROW(Segment::Maneuver(
            defaultName_, defaultInstantCondition_, defaultConstantThrust_, defaultDynamics_, defaultNumericalSolver_
        ));
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, GetName)
{
    EXPECT_EQ(defaultName_, defaultCoastSegment_.getName());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, GetEventCondition)
{
    EXPECT_TRUE(defaultCoastSegment_.getEventCondition() == defaultInstantCondition_);
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, GetDynamics)
{
    EXPECT_EQ(defaultDynamics_, defaultCoastSegment_.getDynamics());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, GetNumericalSolver)
{
    EXPECT_EQ(defaultNumericalSolver_, defaultCoastSegment_.getNumericalSolver());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, GetType)
{
    EXPECT_EQ(Segment::Type::Coast, defaultCoastSegment_.getType());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, AccessEventCondition)
{
    EXPECT_TRUE(defaultCoastSegment_.accessEventCondition() == defaultInstantCondition_);
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, AccessDynamics)
{
    EXPECT_EQ(defaultDynamics_, defaultCoastSegment_.accessDynamics());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, AccessNumericalSolver)
{
    EXPECT_EQ(defaultNumericalSolver_, defaultCoastSegment_.accessNumericalSolver());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, StreamOperator)
{
    {
        testing::internal::CaptureStdout();

        EXPECT_NO_THROW(std::cout << defaultCoastSegment_ << std::endl);

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, Solve)
{
    {
        const Segment::Solution solution = defaultCoastSegment_.solve(defaultState_);

        EXPECT_LT(
            (solution.states.accessLast().getInstant() - defaultInstantCondition_->getInstant()).inSeconds(), 1e-7
        );
        EXPECT_TRUE(solution.states.getSize() > 0);
    }

    {
        const Shared<RealCondition> eventCondition = std::make_shared<RealCondition>(COECondition::Eccentricity(
            RealCondition::Criterion::AnyCrossing,
            Frame::GCRF(),
            Real(0.5),
            EarthGravitationalModel::EGM2008.gravitationalParameter_
        ));

        const Segment segment =
            Segment::Coast("SMA condition", eventCondition, defaultDynamics_, defaultNumericalSolver_);

        const Segment::Solution solution = segment.solve(defaultState_, Duration::Minutes(1.0));

        EXPECT_TRUE(solution.states.getSize() > 0);
        EXPECT_FALSE(solution.conditionIsSatisfied);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Segment, Print)
{
    testing::internal::CaptureStdout();

    EXPECT_NO_THROW(defaultCoastSegment_.print(std::cout, true));
    EXPECT_NO_THROW(defaultCoastSegment_.print(std::cout, false));
    EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
}
