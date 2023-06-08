/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include <vector>
#include "StreetsDatabaseAPI.h"


//Specifies a delivery order (input to your algorithm).
//To satisfy the order the item-to-be-delivered must have been picked-up 
//from the pickUp intersection before visiting the dropOff intersection.
struct DeliveryInf {

    // Constructor
    DeliveryInf(IntersectionIdx pick_up, IntersectionIdx drop_off)
        : pickUp(pick_up), dropOff(drop_off) {}

    //The intersection id where the item-to-be-delivered is picked-up.
    IntersectionIdx pickUp;

    //The intersection id where the item-to-be-delivered is dropped-off.
    IntersectionIdx dropOff;
};


// Specifies one subpath of the courier truck route
struct CourierSubPath {

    // The intersection id where a start depot, pick-up intersection or 
    // drop-off intersection is located
    IntersectionIdx start_intersection;

    // The intersection id where this subpath ends. This must be the 
    // start_intersection of the next subpath or the intersection of an end depot
    IntersectionIdx end_intersection;

    // Street segment ids of the path between start_intersection and end_intersection 
    // They form a connected path (see m3.h)
    std::vector<StreetSegmentIdx> subpath;
};


// This routine takes in a vector of D deliveries (pickUp, dropOff
// intersection pairs), another vector of N intersections that
// are legal start and end points for the path (depots), and a turn 
// penalty in seconds (see m3.h for details on turn penalties).
//
// The first vector 'deliveries' gives the delivery information.  Each delivery
// in this vector has pickUp and dropOff intersection ids.
// A delivery can only be dropped-off after the associated item has been picked-up. 
// 
// The second vector 'depots' gives the intersection ids of courier company
// depots containing trucks; you start at any one of these depots and end at
// any one of the depots.
//
// This routine returns a vector of CourierSubPath objects that form a delivery route.
// The CourierSubPath is as defined above. The first street segment id in the
// first subpath is connected to a depot intersection, and the last street
// segment id of the last subpath also connects to a depot intersection.
// A package will not be dropped off if you haven't picked it up yet.
//
// The start_intersection of each subpath in the returned vector should be 
// at least one of the following (a pick-up and/or drop-off can only happen at 
// the start_intersection of a CourierSubPath object):
//      1- A start depot.
//      2- A pick-up location
//      3- A drop-off location. 
//
// You can assume that D is always at least one and N is always at least one
// (i.e. both input vectors are non-empty).
//
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages. Additionally, 
// one traversal of an intersection is sufficient to drop off all the 
// (already picked up) packages that need to be dropped off at that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.
std::vector<CourierSubPath> travelingCourier(
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const float turn_penalty);

/**
 * @brief A simple pathfinding function with no turn penalty
 * This function will not compile or run until after Milestone 3 
 * is over.
 *
 * If you do not have a working findPathBetweenIntersections from
 * milestone 3, this file includes a Djikstra algorithm with no turn 
 * penalties that you can use for milestone 4.
 *
 * The function prototype is:
 * std::vector<StreetSegmentIdx> findSimplePath(
 *           const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
 *
 * The function is calling 2 of your own milestone 1 functions:
 *	- findStreetSegmentsOfIntersection
 *	- findStreetSegmentTravelTime
 * so those functions must be implemented and correct.
 *
 * @note If you have a working solution from milestone 3, it is better to use 
 *       it instead of findSimplePath, as it will give smaller travel times 
 *       due to handling turn penalties, and it may also have lower cpu time.
 */
#include "FindSimplePath.h"
