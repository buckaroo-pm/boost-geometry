// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track_box_box.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{


/*!
\brief Strategy functor for distance point to box calculation
\ingroup strategies
\details Class which calculates the distance of a point to a box, for
points and boxes on a sphere or globe
\tparam CalculationType \tparam_calculation
\tparam Strategy underlying point-segment distance strategy, defaults
to cross track
\qbk{
[heading See also]
[link geometry.reference.algorithms.distance.distance_3_with_strategy distance (with strategy)]
}
*/
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_cross_track_box_box
{
public:
    typedef geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> Strategy;

    template <typename Box1, typename Box2>
    struct return_type
        : services::return_type<Strategy, typename point_type<Box1>::type, typename point_type<Box2>::type>
    {};

    inline geographic_cross_track_box_box()
    {}

    template <typename Box1, typename Box2>
    inline typename return_type<Box1, Box2>::type
    apply(Box1 const& box1, Box2 const& box2) const
    {
/*
#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointSegmentDistanceStrategy
                    <
                        Strategy,
                        typename point_type<Box1>::type,
                        typename point_type<Box2>::type
                    >)
            );
#endif
*/
        typedef typename return_type<Box1, Box2>::type return_type;
        return details::cross_track_box_box_generic
                                       <return_type>::apply(box1, box2, Strategy());
    }
};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Strategy, typename Spheroid, typename CalculationType>
struct tag<geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_box_box type;
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename Box1, typename Box2>
struct return_type<geographic_cross_track_box_box<Strategy, Spheroid, CalculationType>, Box1, Box2>
    : geographic_cross_track_box_box
        <
            Strategy, Spheroid, CalculationType
        >::template return_type<Box1, Box2>
{};

template <typename Strategy, typename Spheroid, typename Box1, typename Box2>
struct return_type<geographic_cross_track_box_box<Strategy, Spheroid>, Box1, Box2>
    : geographic_cross_track_box_box
        <
            Strategy, Spheroid
        >::template return_type<Box1, Box2>
{};

template <typename Strategy, typename Box1, typename Box2>
struct return_type<geographic_cross_track_box_box<Strategy>, Box1, Box2>
    : geographic_cross_track_box_box
        <
            Strategy
        >::template return_type<Box1, Box2>
{};

template <typename Strategy, typename Spheroid, typename CalculationType>
struct comparable_type<geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> >
{
    typedef geographic_cross_track_box_box
        <
            typename comparable_type<Strategy>::type, Spheroid, CalculationType
        > type;
};


template <typename Strategy, typename Spheroid, typename CalculationType>
struct get_comparable<geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> >
{
    typedef geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> this_strategy;
    typedef typename comparable_type<this_strategy>::type comparable_type;

public:
    static inline comparable_type apply(this_strategy const& strategy)
    {
        return comparable_type();
    }
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename Box1, typename Box2>
struct result_from_distance
    <
        geographic_cross_track_box_box<Strategy, Spheroid, CalculationType>, Box1, Box2
    >
{
private:
    typedef geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> this_strategy;

    typedef typename this_strategy::template return_type
        <
            Box1, Box2
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        result_from_distance
            <
                Strategy,
                typename point_type<Box1>::type,
                typename point_type<Box2>::type
            >::apply(strategy, distance);
    }
};

template <typename Box1, typename Box2>
struct default_strategy
    <
        box_tag, box_tag, Box1, Box2,
        geographic_tag, geographic_tag
    >
{
    typedef geographic_cross_track_box_box<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
