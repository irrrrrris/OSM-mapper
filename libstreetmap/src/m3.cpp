#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "gtk/gtk.h"

#include <iostream>
#include <string>
#include <sstream>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "dataStructures.h"
#include <chrono>
#include <fstream>
#include <math.h>

#include <queue>
#include <map>
#include <utility>
#include <limits>
#include <tuple>

#include <cmath> 
#include <cctype>
#include <algorithm>
#include <list>
#include <map>
#include <fstream>

#include <chrono>
#include <bits/stdc++.h>
using namespace std::chrono;

double heuristic(IntersectionIdx current_intersection, IntersectionIdx end_intersection);



// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0. The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path. If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty) {
    double travel_time = 0;
    double turns = 0;
    int path_size = path.size();

    if (path_size <= 0) {
        return 0.0;
    }

    StreetSegmentInfo current_street = getStreetSegmentInfo(path[0]);
    std::string current_name = getStreetName(current_street.streetID);
    std::string next_name;

    for (int count = 0; count < path_size; count++) {
        travel_time = travel_time + findStreetSegmentTravelTime(path[count]);

        current_street = getStreetSegmentInfo(path[count]);
        next_name = getStreetName(current_street.streetID);
        
        if (current_name != next_name) {
            turns++;
            current_name = next_name;
        }
    }

    travel_time = travel_time + turns*turn_penalty;
    return travel_time;
}

// Returns a path (route) between the start intersection (intersect_id.first)
// and the destination intersection (intersect_id.second), if one exists.
// This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds). If no path exists, this routine
// returns an empty (size == 0) vector. If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.

/*Iris notes
Node Data Structure:
1. node name
2. shortest time to reach 
3. parent (the previous node)
4. has been traversed 
//when node is popped from priority queue -> traversed 
*/





std::vector<StreetSegmentIdx> findPathBetweenIntersections(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids, const double turn_penalty) {
    auto start = high_resolution_clock::now();
    IntersectionIdx start_intersection = intersect_ids.first;
    IntersectionIdx end_intersection = intersect_ids.second;

    std::priority_queue<Node> pq;
    std::vector<SegTime> seg_times(getNumIntersections(), SegTime(INFINITY));

    pq.push(Node(0, start_intersection, 0.0, 0.0, -1));

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (current.real_time < seg_times[current.name].best_time) {
            seg_times[current.name].best_time = current.real_time;
            
            if (current.parent != -1) {
                seg_times[current.name].prevIntersection = current.parent;
                seg_times[current.name].seg = current.seg;
            }

            // Path found, reconstruct path:
            if (current.name == end_intersection) {
                std::vector<StreetSegmentIdx> path;
                IntersectionIdx node_id = current.name;
                IntersectionIdx prev_id = seg_times[node_id].prevIntersection;

                while (node_id != start_intersection) {
                    path.insert(path.begin(), seg_times[node_id].seg);
                    node_id = prev_id;
                    prev_id = seg_times[node_id].prevIntersection;
                }
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(stop - start);
                std::cout << (double) (duration.count() / 1000000.0) << std::endl;
                return path;
            }
            

            for (const auto& current_seg : findSegments(current.name)) {
                StreetSegmentInfo current_info = getStreetSegmentInfo(current_seg);
                StreetIdx current_street = current_info.streetID;

                if (!current_info.oneWay || (current_info.oneWay && current_info.from == current.name)) {

                    IntersectionIdx next_intersection;
                    if (current_info.to == current.name) {
                        next_intersection = current_info.from;
                    } else {
                        next_intersection = current_info.to;
                    }
                    
                    StreetSegmentIdx previous_seg;
                    StreetSegmentInfo previous_info;
                    StreetIdx previous_street;

                    double penalty = 0.0;

                    if (current.parent != -1) {
                        previous_seg = seg_times[current.name].seg;
                        previous_info = getStreetSegmentInfo(previous_seg);
                        previous_street = previous_info.streetID;
                        
                        if (current_street != previous_street) {
                            penalty = penalty + turn_penalty;
                        }
                    }
                    
                    double travel_time = findStreetSegmentTravelTime(current_seg);
                    double new_time = current.real_time + travel_time + penalty;
                    double _heuristic = heuristic(next_intersection, end_intersection);        

                    pq.push(Node(current_seg, next_intersection, new_time, new_time + _heuristic, current.name));
                    
                    // std::cout << << std::endl;
                }
            }
        }
    }
    return std::vector<StreetSegmentIdx>(); // No path found
}

double heuristic(IntersectionIdx current_intersection, IntersectionIdx end_intersection) {
    LatLon point_1 = getIntersectionPosition(current_intersection);
    LatLon point_2 = getIntersectionPosition(end_intersection);
    double x1, x2, y1, y2;

    double lat_avg1 = ((point_1).latitude()*M_PI/180 + (point_2).latitude()*M_PI/180)/2;

    // (x1, y1) for point_1:
    x1 = kEarthRadiusInMeters*((point_1).longitude()*(M_PI/180))*cos(lat_avg1);
    y1 = kEarthRadiusInMeters*((point_1).latitude()*(M_PI/180));

    // (x2, y2) for point_2:
    x2 = kEarthRadiusInMeters*((point_2).longitude()*(M_PI/180)*cos(lat_avg1));
    y2 = kEarthRadiusInMeters*(point_2).latitude()*(M_PI/180);

    double dx = x2 - x1;
    double dy = y2 - y1;

    // Distance:
    double distance;
    distance = sqrt(dx*dx + dy*dy);

    return distance/max_speed;
}

// Finds the street segment that connects 2 intersections
// returns a vector of all segment indices connects to the pass-in intersection
std::vector<StreetSegmentIdx> findSegments(IntersectionIdx intersection_1) {
    std::vector<StreetSegmentIdx> segments;
    //get the number of segments on this intersection
    int connections = getNumIntersectionStreetSegment(intersection_1);

    for (int i = 0; i < connections; i++) {
        StreetSegmentIdx segment = getIntersectionStreetSegment(intersection_1, i);
        segments.push_back(segment);
    }

    return segments;
}


