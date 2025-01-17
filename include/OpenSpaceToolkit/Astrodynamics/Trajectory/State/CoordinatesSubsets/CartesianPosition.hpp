/// Apache License 2.0

#ifndef __OpenSpaceToolkit_Astrodynamics_Trajectory_State_CoordinatesSubsets_CartesianPosition__
#define __OpenSpaceToolkit_Astrodynamics_Trajectory_State_CoordinatesSubsets_CartesianPosition__

#include <OpenSpaceToolkit/Astrodynamics/Trajectory/State/CoordinatesBroker.hpp>

namespace ostk
{
namespace astro
{
namespace trajectory
{
namespace state
{
namespace coordinatessubsets
{

using ostk::core::types::Shared;
using ostk::core::types::String;

using ostk::physics::time::Instant;

using ostk::astro::trajectory::state::CoordinatesSubset;
using ostk::astro::trajectory::state::CoordinatesBroker;

/// @brief Cartesian Position.
class CartesianPosition : public CoordinatesSubset
{
   public:
    /// @brief Constructor
    ///
    /// @code{.cpp}
    ///     CartesianPosition cartesianPosition = {aName};
    /// @endcode
    ///
    /// @param aName a name
    CartesianPosition(const String& aName);

    /// @brief Destructor
    ~CartesianPosition();

    /// @brief Add two coordinates subsets
    ///
    /// @param anInstant the instant associated to the coordinates
    /// @param aFullCoordinatesVector first set of all coordinates
    /// @param anotherFullCoordinatesVector second set of all coordinates
    /// @param aFrameSPtr the reference frame in which the coordinates are resolved
    /// @param aCoordinatesBrokerSPtr a coordinates broker
    ///
    /// @return The resulting coordinates subset value (subset_1 + subset_2)
    virtual VectorXd add(
        const Instant& anInstant,
        const VectorXd& aFullCoordinatesVector,
        const VectorXd& anotherFullCoordinatesVector,
        const Shared<const Frame>& aFrameSPtr,
        const Shared<const CoordinatesBroker>& aCoordinatesBrokerSPtr
    ) const override;

    /// @brief Subtract two coordinates subsets
    ///
    /// @param anInstant the instant associated to the coordinates
    /// @param aFullCoordinatesVector first set of all coordinates
    /// @param anotherFullCoordinatesVector second set of all coordinates
    /// @param aFrameSPtr the reference frame associated to the coordinates
    /// @param aCoordinatesBrokerSPtr a coordinates broker
    ///
    /// @return The resulting coordinates subset value (subset_1 - subset_2)
    virtual VectorXd subtract(
        const Instant& anInstant,
        const VectorXd& aFullCoordinatesVector,
        const VectorXd& anotherFullCoordinatesVector,
        const Shared<const Frame>& aFrameSPtr,
        const Shared<const CoordinatesBroker>& aCoordinatesBrokerSPtr
    ) const override;

    /// @brief Transforms the coordinate subset from one frame to another
    ///
    /// @param anInstant the reference frame associated to the coordinates
    /// @param aFullCoordinatesVector all coordinates
    /// @param fromFrame the reference frame associated to the coordinates
    /// @param toFrame the reference frame in which the coordinates are to be transformed
    /// @param aCoordinatesBrokerSPtr a coordinates broker
    ///
    /// @return The resulting coordinates subset value expressed in the desired reference frame
    VectorXd inFrame(
        const Instant& anInstant,
        const VectorXd& aFullCoordinatesVector,
        const Shared<const Frame>& fromFrame,
        const Shared<const Frame>& toFrame,
        const Shared<const CoordinatesBroker>& aCoordinatesBrokerSPtr
    ) const override;

    /// @brief Return the default instance
    ///
    /// @return The default instance
    static Shared<const CartesianPosition> Default();
};

}  // namespace coordinatessubsets
}  // namespace state
}  // namespace trajectory
}  // namespace astro
}  // namespace ostk

#endif
