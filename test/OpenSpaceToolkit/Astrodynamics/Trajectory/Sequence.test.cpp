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
#include <OpenSpaceToolkit/Astrodynamics/Dynamics/Thruster.hpp>
#include <OpenSpaceToolkit/Astrodynamics/EventCondition/COECondition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/EventCondition/InstantCondition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/GuidanceLaw/ConstantThrust.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/LocalOrbitalFrameFactory.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Segment.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/Sequence.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianPosition.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesSubsets/CartesianVelocity.hpp>
#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/NumericalSolver.hpp>

#include <Global.test.hpp>

using ostk::core::ctnr::Array;
using ostk::core::types::Shared;
using ostk::core::types::Size;
using ostk::core::types::Index;
using ostk::core::types::Real;

using ostk::math::geometry::d3::objects::Composite;
using ostk::math::geometry::d3::objects::Cuboid;
using ostk::math::geometry::d3::objects::Point;
using ostk::math::object::Matrix3d;
using ostk::math::object::Vector3d;
using ostk::math::object::VectorXd;

using ostk::physics::environment::object::Celestial;
using ostk::physics::time::Instant;
using ostk::physics::time::Duration;
using ostk::physics::time::DateTime;
using ostk::physics::time::Scale;
using ostk::physics::environment::object::celestial::Earth;
using ostk::physics::coord::Frame;
using ostk::physics::coord::Position;
using ostk::physics::coord::Velocity;
using ostk::physics::units::Angle;
using ostk::physics::units::Length;
using ostk::physics::units::Mass;
using ostk::physics::data::Scalar;
using EarthGravitationalModel = ostk::physics::environment::gravitational::Earth;
using EarthMagneticModel = ostk::physics::environment::magnetic::Earth;
using EarthAtmosphericModel = ostk::physics::environment::atmospheric::Earth;

using ostk::astro::trajectory::state::NumericalSolver;
using ostk::astro::flight::system::SatelliteSystem;
using ostk::astro::flight::system::PropulsionSystem;
using ostk::astro::trajectory::Segment;
using ostk::astro::trajectory::Sequence;
using ostk::astro::trajectory::LocalOrbitalFrameFactory;
using ostk::astro::trajectory::State;
using ostk::astro::trajectory::state::CoordinatesSubset;
using ostk::astro::trajectory::state::CoordinatesBroker;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianPosition;
using ostk::astro::trajectory::state::coordinatessubsets::CartesianVelocity;
using ostk::astro::trajectory::orbit::models::kepler::COE;
using ostk::astro::Dynamics;
using ostk::astro::dynamics::AtmosphericDrag;
using ostk::astro::dynamics::CentralBodyGravity;
using ostk::astro::dynamics::PositionDerivative;
using ostk::astro::dynamics::Thruster;
using ostk::astro::guidancelaw::ConstantThrust;
using ostk::astro::EventCondition;
using ostk::astro::eventcondition::COECondition;
using ostk::astro::eventcondition::AngularCondition;
using ostk::astro::eventcondition::RealCondition;
using ostk::astro::eventcondition::InstantCondition;

class OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence : public ::testing::Test
{
   protected:
    const State defaultState_ = {
        Instant::DateTime(DateTime(2021, 3, 20, 12, 0, 0), Scale::UTC),
        Position::Meters({7000000.0, 0.0, 0.0}, Frame::GCRF()),
        Velocity::MetersPerSecond({0.0, 7546.05329, 0.0}, Frame::GCRF()),
    };

    const Duration propagationDuration_ = Duration::Minutes(15.0);

    const Shared<Celestial> earthSpherical_ = std::make_shared<Celestial>(Earth::Spherical());
    const Array<Shared<Dynamics>> defaultDynamics_ = {
        std::make_shared<PositionDerivative>(),
        std::make_shared<CentralBodyGravity>(earthSpherical_),
    };

    const NumericalSolver defaultNumericalSolver_ = {
        NumericalSolver::LogType::NoLog,
        NumericalSolver::StepperType::RungeKuttaDopri5,
        1e-3,
        1.0e-12,
        1.0e-12,
    };

    const Shared<AngularCondition> defaultCondition_ = std::make_shared<AngularCondition>(COECondition::TrueAnomaly(
        AngularCondition::Criterion::AnyCrossing,
        Frame::GCRF(),
        Angle::Degrees(0.0),
        EarthGravitationalModel::EGM2008.gravitationalParameter_
    ));

    const Segment coastSegment_ = Segment::Coast("Coast", defaultCondition_, defaultDynamics_, defaultNumericalSolver_);

    const Array<Segment> defaultSegments_ = {coastSegment_};

    const Segment::Solution defaultSegmentSolution_ = {
        "A Segment",
        defaultDynamics_,
        {defaultState_},
        true,
        Segment::Type::Coast,
    };

    const Size defaultRepetitionCount_ = 2;
    const Duration defaultMaximumPropagationDuration_ = Duration::Days(7.0);
    Sequence defaultSequence_ = {
        defaultSegments_,
        defaultNumericalSolver_,
        defaultDynamics_,
        defaultMaximumPropagationDuration_,
    };
};

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, SequenceSolution_GetStates)
{
    // Test throw with empty sequence solving
    {
        Sequence::Solution sequenceSolution = {{}, true};
        EXPECT_THROW(sequenceSolution.getStates(), ostk::core::error::RuntimeError);
    }

    const State state1 = defaultState_;
    const State state2 = {
        defaultState_.getInstant() + Duration::Minutes(1.0),
        defaultState_.getPosition(),
        defaultState_.getVelocity(),
    };
    const State state3 = {
        defaultState_.getInstant() + Duration::Minutes(2.0),
        defaultState_.getPosition(),
        defaultState_.getVelocity(),
    };

    const Segment::Solution segmentSolution1 = {
        "A Segment",
        defaultDynamics_,
        {state1, state2},
        true,
        Segment::Type::Coast,
    };

    const Segment::Solution segmentSolution2 = {
        "A Segment",
        defaultDynamics_,
        {state2, state3},
        true,
        Segment::Type::Coast,
    };

    // Test regular states
    {
        const Sequence::Solution sequenceSolution = {{segmentSolution1, segmentSolution2}, true};

        const Array<State> states = sequenceSolution.getStates();

        EXPECT_EQ(states[0], state1);
        EXPECT_EQ(states[1], state2);
        EXPECT_EQ(states[2], state3);
        EXPECT_EQ(states.getSize(), 3);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, SequenceSolution_CalculateStatesAt)
{
    // Test throw with empty sequence solving
    {
        Sequence::Solution sequenceSolution = {{}, true};
        EXPECT_THROW(
            sequenceSolution.calculateStatesAt({Instant::J2000()}, defaultNumericalSolver_),
            ostk::core::error::RuntimeError
        );
    }

    const State state1 = defaultState_;
    const State state2 = {
        defaultState_.getInstant() + Duration::Minutes(1.0),
        defaultState_.getPosition(),
        defaultState_.getVelocity(),
    };
    const State state3 = {
        defaultState_.getInstant() + Duration::Minutes(2.0),
        defaultState_.getPosition(),
        defaultState_.getVelocity(),
    };

    const Segment::Solution segmentSolution1 = {
        "A Segment",
        defaultDynamics_,
        {state1, state2},
        true,
        Segment::Type::Coast,
    };

    const Segment::Solution segmentSolution2 = {
        "A Segment",
        defaultDynamics_,
        {state2, state3},
        true,
        Segment::Type::Coast,
    };

    // Test regular states success
    {
        const Sequence::Solution sequenceSolution = {{segmentSolution1, segmentSolution2}, true};

        const Array<State> propagatedStates = sequenceSolution.calculateStatesAt(
            {
                state1.getInstant(),
                state2.getInstant(),
                state3.getInstant(),
            },
            defaultNumericalSolver_
        );

        EXPECT_EQ(propagatedStates[0].getInstant(), state1.getInstant());
        EXPECT_EQ(propagatedStates[1].getInstant(), state2.getInstant());
        EXPECT_EQ(propagatedStates[2].getInstant(), state3.getInstant());
        EXPECT_EQ(propagatedStates.getSize(), 3);
    }

    // Test regular states in between success
    {
        const Sequence::Solution sequenceSolution = {{segmentSolution1, segmentSolution2}, true};

        const Array<State> propagatedStatesInBetween = sequenceSolution.calculateStatesAt(
            {
                state1.getInstant() + Duration::Minutes(0.5),
                state2.getInstant() + Duration::Minutes(0.5),
            },
            defaultNumericalSolver_
        );

        EXPECT_EQ(propagatedStatesInBetween[0].getInstant(), state1.getInstant() + Duration::Minutes(0.5));
        EXPECT_EQ(propagatedStatesInBetween[1].getInstant(), state2.getInstant() + Duration::Minutes(0.5));
        EXPECT_EQ(propagatedStatesInBetween.getSize(), 2);
    }

    // Test regular states out of range failure
    {
        const Sequence::Solution sequenceSolution = {{segmentSolution1, segmentSolution2}, true};

        const Array<State> propagatedStatesOutsideSequence = sequenceSolution.calculateStatesAt(
            {
                state1.getInstant() - Duration::Minutes(0.5),
                state3.getInstant() + Duration::Minutes(0.5),
            },
            defaultNumericalSolver_
        );

        EXPECT_EQ(propagatedStatesOutsideSequence.getSize(), 0);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, SequenceSolution_Print)
{
    {
        testing::internal::CaptureStdout();

        Sequence::Solution sequenceSolution = {{defaultSegmentSolution_}, true};

        EXPECT_NO_THROW(sequenceSolution.print(std::cout, true));
        EXPECT_NO_THROW(sequenceSolution.print(std::cout, false));

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, SequenceSolution_StreamOperator)
{
    {
        testing::internal::CaptureStdout();

        Sequence::Solution sequenceSolution = {{defaultSegmentSolution_}, true};

        EXPECT_NO_THROW(std::cout << sequenceSolution << std::endl);

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, Constructor)
{
    {
        EXPECT_NO_THROW(Sequence sequence());
    }

    {
        EXPECT_NO_THROW(Sequence sequence(defaultSegments_));
    }

    {
        EXPECT_NO_THROW(Sequence sequence(defaultSegments_, defaultNumericalSolver_));
    }

    {
        EXPECT_NO_THROW(Sequence sequence(defaultSegments_, defaultNumericalSolver_, defaultDynamics_));
    }

    {
        EXPECT_NO_THROW(Sequence sequence(
            defaultSegments_, defaultNumericalSolver_, defaultDynamics_, defaultMaximumPropagationDuration_
        ));
    }

    {
        {
            for (Size verbosity = 0; verbosity <= 5; ++verbosity)
            {
                EXPECT_NO_THROW(Sequence sequence(
                    defaultSegments_,
                    defaultNumericalSolver_,
                    defaultDynamics_,
                    defaultMaximumPropagationDuration_,
                    verbosity
                ));
            }
        }

        {
            EXPECT_THROW(
                Sequence sequence(
                    defaultSegments_, defaultNumericalSolver_, defaultDynamics_, defaultMaximumPropagationDuration_, 6
                ),
                ostk::core::error::runtime::Wrong
            );
        }
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, StreamOperator)
{
    {
        testing::internal::CaptureStdout();

        EXPECT_NO_THROW(std::cout << defaultSequence_ << std::endl);

        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, GetSegments)
{
    EXPECT_EQ(defaultSegments_.getSize(), defaultSequence_.getSegments().getSize());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, GetNumericalSolver)
{
    EXPECT_EQ(defaultNumericalSolver_, defaultSequence_.getNumericalSolver());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, GetDynamics)
{
    EXPECT_EQ(defaultDynamics_, defaultSequence_.getDynamics());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, GetMaximumPropagationDuration)
{
    EXPECT_EQ(defaultMaximumPropagationDuration_, defaultSequence_.getMaximumPropagationDuration());
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, AddSegment)
{
    {
        EXPECT_NO_THROW(defaultSequence_.addSegment(coastSegment_));
    }

    {
        EXPECT_NO_THROW(defaultSequence_.addSegments({coastSegment_, coastSegment_}));
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, AddCoastSegment)
{
    {
        const Size segmentsCount = defaultSequence_.getSegments().getSize();

        defaultSequence_.addCoastSegment(std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
            RealCondition::Criterion::AnyCrossing,
            Frame::GCRF(),
            Length::Kilometers(6999.5),
            EarthGravitationalModel::EGM2008.gravitationalParameter_
        )));

        EXPECT_TRUE(defaultSequence_.getSegments().getSize() == segmentsCount + 1);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, AddManeuverSegment)
{
    {
        const Size segmentsCount = defaultSequence_.getSegments().getSize();

        const Shared<const ConstantThrust> constantThrustSPtr =
            std::make_shared<ConstantThrust>(ConstantThrust::Intrack());

        const Shared<Thruster> thrustDynamicsSPtr =
            std::make_shared<Thruster>(SatelliteSystem::Default(), constantThrustSPtr);

        defaultSequence_.addManeuverSegment(
            std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
                RealCondition::Criterion::AnyCrossing,
                Frame::GCRF(),
                Length::Kilometers(7000.0),
                EarthGravitationalModel::EGM2008.gravitationalParameter_
            )),
            thrustDynamicsSPtr
        );

        EXPECT_TRUE(defaultSequence_.getSegments().getSize() == segmentsCount + 1);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, Solve)
{
    // default solve
    {
        EXPECT_THROW(defaultSequence_.solve(defaultState_, 0), ostk::core::error::runtime::Wrong);
    }

    {
        const Sequence::Solution solution = defaultSequence_.solve(defaultState_, defaultRepetitionCount_);

        EXPECT_TRUE(
            solution.segmentSolutions.getSize() == defaultSequence_.getSegments().getSize() * defaultRepetitionCount_
        );

        Size statesSize = 1;
        for (const Segment::Solution& segmentSolution : solution.segmentSolutions)
        {
            EXPECT_TRUE(segmentSolution.states.getSize() > 0);

            const Real targetAngle = defaultCondition_->getEvaluator()(segmentSolution.states.accessLast());
            EXPECT_NEAR(targetAngle, defaultCondition_->getTargetAngle().inRadians(0.0, Real::TwoPi()), 1e-6);

            statesSize += segmentSolution.states.getSize() - 1;
        }

        EXPECT_EQ(solution.getStates().getSize(), statesSize);
    }

    // segment termination due to maximum propagation duration
    {
        Sequence sequence = {
            defaultSegments_,
            defaultNumericalSolver_,
            defaultDynamics_,
            Duration::Seconds(1.0),
        };

        const Sequence::Solution solution = sequence.solve(defaultState_, defaultRepetitionCount_);

        EXPECT_FALSE(solution.executionIsComplete);
        EXPECT_EQ(solution.segmentSolutions.getSize(), 1);
        EXPECT_FALSE(solution.segmentSolutions[0].conditionIsSatisfied);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, SolveToCondition)
{
    // sequence completion due to event condition
    {
        const Sequence sequence = {
            defaultSegments_,
            defaultNumericalSolver_,
            defaultDynamics_,
            defaultMaximumPropagationDuration_,
        };

        const InstantCondition eventCondition = InstantCondition(
            InstantCondition::Criterion::StrictlyPositive, defaultState_.accessInstant() + Duration::Seconds(1.0)
        );

        const Sequence::Solution solution = sequence.solveToCondition(defaultState_, eventCondition);

        EXPECT_TRUE(solution.executionIsComplete);
        EXPECT_EQ(solution.segmentSolutions.getSize(), 1);
    }

    // sequence failure, segment termination due to maximum propagation duration
    {
        const Sequence sequence = {
            defaultSegments_,
            defaultNumericalSolver_,
            defaultDynamics_,
            Duration::Seconds(1.0),
        };

        const InstantCondition eventCondition = InstantCondition(
            InstantCondition::Criterion::StrictlyPositive, defaultState_.accessInstant() + Duration::Days(1.0)
        );

        const Sequence::Solution solution = sequence.solveToCondition(defaultState_, eventCondition);

        EXPECT_FALSE(solution.executionIsComplete);
        EXPECT_EQ(solution.segmentSolutions.getSize(), 1);
        EXPECT_FALSE(solution.segmentSolutions[0].conditionIsSatisfied);
    }

    // sequence failure, event condition not met
    {
        const Sequence sequence = {
            defaultSegments_,
            defaultNumericalSolver_,
            defaultDynamics_,
            defaultMaximumPropagationDuration_,
        };

        const InstantCondition eventCondition = InstantCondition(
            InstantCondition::Criterion::StrictlyPositive, defaultState_.accessInstant() + Duration::Days(1.0)
        );

        const Sequence::Solution solution =
            sequence.solveToCondition(defaultState_, eventCondition, Duration::Minutes(1.0));

        EXPECT_FALSE(solution.executionIsComplete);
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, Solve_2)
{
    // dynamics
    const Shared<Earth> earthSPtr = std::make_shared<Earth>(Earth::FromModels(
        std::make_shared<EarthGravitationalModel>(EarthGravitationalModel::Type::Spherical),
        std::make_shared<EarthMagneticModel>(EarthMagneticModel::Type::Undefined),
        std::make_shared<EarthAtmosphericModel>(EarthAtmosphericModel::Type::Exponential)
    ));

    const Composite satelliteGeometry(Cuboid(
        {0.0, 0.0, 0.0}, {Vector3d {1.0, 0.0, 0.0}, Vector3d {0.0, 1.0, 0.0}, Vector3d {0.0, 0.0, 1.0}}, {1.0, 2.0, 3.0}
    ));

    const PropulsionSystem propulsionSystem = PropulsionSystem(1.0, 1500.0);

    const SatelliteSystem satelliteSystem = {
        Mass::Kilograms(100.0),
        satelliteGeometry,
        Matrix3d::Identity(),
        500.0,
        2.1,
        propulsionSystem,
    };

    const Real mass = satelliteSystem.getMass().inKilograms();
    const Real surfaceArea = satelliteSystem.getCrossSectionalSurfaceArea();
    const Real dragCoefficient = satelliteSystem.getDragCoefficient();

    const Array<Shared<Dynamics>> dynamics = {
        std::make_shared<PositionDerivative>(),
        std::make_shared<CentralBodyGravity>(earthSPtr),
        std::make_shared<AtmosphericDrag>(earthSPtr),
    };

    Sequence sequence = {
        Array<Segment>::Empty(),
        defaultNumericalSolver_,
        dynamics,
        defaultMaximumPropagationDuration_,
    };

    sequence.addCoastSegment(std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
        RealCondition::Criterion::AnyCrossing,
        Frame::GCRF(),
        Length::Kilometers(6999.5),
        EarthGravitationalModel::EGM2008.gravitationalParameter_
    )));

    sequence.addManeuverSegment(
        std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
            RealCondition::Criterion::AnyCrossing,
            Frame::GCRF(),
            Length::Kilometers(7000.0),
            EarthGravitationalModel::EGM2008.gravitationalParameter_
        )),
        std::make_shared<Thruster>(satelliteSystem, std::make_shared<ConstantThrust>(ConstantThrust::Intrack()))
    );

    const Shared<const CoordinatesBroker> coordinatesBrokerSPtr =
        std::make_shared<CoordinatesBroker>(CoordinatesBroker({
            CartesianPosition::Default(),
            CartesianVelocity::Default(),
            CoordinatesSubset::Mass(),
            CoordinatesSubset::SurfaceArea(),
            CoordinatesSubset::DragCoefficient(),
        }));

    VectorXd coordinates(9);
    coordinates << 7000000.0, 0.0, 0.0, 0.0, 7546.05329, 0.0, mass + 100.0, surfaceArea, dragCoefficient;
    const State state = {
        Instant::J2000(),
        coordinates,
        Frame::GCRF(),
        coordinatesBrokerSPtr,
    };

    const Sequence::Solution solution = sequence.solve(state, defaultRepetitionCount_);

    EXPECT_TRUE(solution.segmentSolutions.getSize() == 2 * defaultRepetitionCount_);

    EXPECT_EQ(solution.accessStartInstant(), Instant::J2000());
    EXPECT_EQ(solution.accessEndInstant(), solution.segmentSolutions.accessLast().states.accessLast().accessInstant());

    const Array<State> solutionStates = solution.getStates();

    for (Index i = 1; i < solutionStates.getSize(); ++i)
    {
        EXPECT_TRUE(solutionStates[i].accessInstant() > solutionStates[i - 1].accessInstant());
    }
    EXPECT_DOUBLE_EQ(solution.getInitialMass().inKilograms(), mass + 100.0);
    EXPECT_DOUBLE_EQ(
        solution.getFinalMass().inKilograms(),
        solution.segmentSolutions.accessLast().states.accessLast().accessCoordinates()[6]
    );
    EXPECT_EQ(solution.getPropagationDuration(), solution.accessEndInstant() - solution.accessStartInstant());

    EXPECT_DOUBLE_EQ(
        solution.computeDeltaMass().inKilograms(),
        solution.getInitialMass().inKilograms() - solution.getFinalMass().inKilograms()
    );
    EXPECT_NEAR(
        solution.computeDeltaV(3000.0),
        3000.0 * EarthGravitationalModel::gravityConstant *
            std::log(solution.getInitialMass().inKilograms() / solution.getFinalMass().inKilograms()),
        1e-3
    );
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, Solve_3)
{
    const Size reptitionCount = 3;

    {
        const Shared<AngularCondition> relativeTrueAnomalyCondition =
            std::make_shared<AngularCondition>(COECondition::TrueAnomaly(
                AngularCondition::Criterion::AnyCrossing,
                Frame::GCRF(),
                EventCondition::Target(Angle::Degrees(5.0), EventCondition::Target::Type::Relative),
                EarthGravitationalModel::EGM2008.gravitationalParameter_
            ));

        const Array<Segment> segments = {
            Segment::Coast(
                "Relative True Anomaly", relativeTrueAnomalyCondition, defaultDynamics_, defaultNumericalSolver_
            ),
        };

        Sequence sequence = {
            segments,
            defaultNumericalSolver_,
            defaultDynamics_,
            Duration::Days(1.0),
        };

        const Sequence::Solution solution = sequence.solve(defaultState_, reptitionCount);

        EXPECT_TRUE(solution.executionIsComplete);
        EXPECT_EQ(solution.segmentSolutions.getSize(), reptitionCount);

        COE initialCOE = COE::Cartesian(
            {defaultState_.getPosition(), defaultState_.getVelocity()},
            EarthGravitationalModel::EGM2008.gravitationalParameter_
        );

        for (const Segment::Solution& segmentSolution : solution.segmentSolutions)
        {
            EXPECT_TRUE(segmentSolution.conditionIsSatisfied);
            const COE coe = COE::Cartesian(
                {segmentSolution.states.accessLast().getPosition(), segmentSolution.states.accessLast().getVelocity()},
                EarthGravitationalModel::EGM2008.gravitationalParameter_
            );
            EXPECT_NEAR(coe.getTrueAnomaly().inDegrees() - initialCOE.getTrueAnomaly().inDegrees(), 5.0, 1e-5);
            initialCOE = coe;
        }
    }

    {
        const Shared<RealCondition> durationCondition = std::make_shared<RealCondition>(
            RealCondition::DurationCondition(RealCondition::Criterion::StrictlyPositive, Duration::Seconds(30.0))
        );

        const Array<Segment> segments = {
            Segment::Coast("Duration", durationCondition, defaultDynamics_, defaultNumericalSolver_),
        };

        Sequence sequence = {
            segments,
            defaultNumericalSolver_,
            defaultDynamics_,
            Duration::Days(1.0),
        };

        const Sequence::Solution solution = sequence.solve(defaultState_, reptitionCount);

        EXPECT_TRUE(solution.executionIsComplete);
        EXPECT_EQ(solution.segmentSolutions.getSize(), reptitionCount);

        Size iter = 1;
        for (const Segment::Solution& segmentSolution : solution.segmentSolutions)
        {
            EXPECT_TRUE(segmentSolution.conditionIsSatisfied);
            EXPECT_NEAR(
                (segmentSolution.accessEndInstant() - defaultState_.accessInstant()).inSeconds(), 30.0 * iter, 1e-6
            );

            ++iter;
        }
    }
}

TEST_F(OpenSpaceToolkit_Astrodynamics_Trajectory_Sequence, Print)
{
    {
        testing::internal::CaptureStdout();

        EXPECT_NO_THROW(defaultSequence_.print(std::cout, true));
        EXPECT_NO_THROW(defaultSequence_.print(std::cout, false));
        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }

    {
        const Composite satelliteGeometry(Cuboid(
            {0.0, 0.0, 0.0},
            {Vector3d {1.0, 0.0, 0.0}, Vector3d {0.0, 1.0, 0.0}, Vector3d {0.0, 0.0, 1.0}},
            {1.0, 2.0, 3.0}
        ));

        const PropulsionSystem propulsionSystem = PropulsionSystem(1.0, 1500.0);

        const SatelliteSystem satelliteSystem = {
            Mass::Kilograms(100.0),
            satelliteGeometry,
            Matrix3d::Identity(),
            500.0,
            2.1,
            propulsionSystem,
        };

        Sequence sequence = {
            Array<Segment>::Empty(),
            defaultNumericalSolver_,
            defaultDynamics_,
            defaultMaximumPropagationDuration_,
        };

        sequence.addCoastSegment(std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
            RealCondition::Criterion::AnyCrossing,
            Frame::GCRF(),
            Length::Kilometers(6999.5),
            EarthGravitationalModel::EGM2008.gravitationalParameter_
        )));

        sequence.addManeuverSegment(
            std::make_shared<RealCondition>(COECondition::SemiMajorAxis(
                RealCondition::Criterion::AnyCrossing,
                Frame::GCRF(),
                Length::Kilometers(7000.0),
                EarthGravitationalModel::EGM2008.gravitationalParameter_
            )),
            std::make_shared<Thruster>(satelliteSystem, std::make_shared<ConstantThrust>(ConstantThrust::Intrack()))
        );

        testing::internal::CaptureStdout();

        EXPECT_NO_THROW(sequence.print(std::cout, true));
        EXPECT_NO_THROW(sequence.print(std::cout, false));
        EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
    }
}
