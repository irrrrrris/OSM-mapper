/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   dataStructures.h
 * Author: liuxi349
 *
 * Created on March 5, 2023, 1:23 PM
 */

#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <map>
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "dataStructures.h"

#include "ezgl/point.hpp"
extern double lat_avg;

typedef struct featureInfo {
    std::vector<ezgl::point2d> pointsInAFeature;
    bool closedPoly;
    std::string FeatureName;
    int featurePtNum;
    
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    
} featureInfo;


typedef struct zoom_level {
    int zoom_level;
    double lower_bound,upper_bound;  
    
} zoom_level;



typedef struct Node {
    StreetSegmentIdx seg;
    IntersectionIdx name;
    double real_time;
    double heur;
    IntersectionIdx parent;
    Node() : seg(-1), name(0), real_time(0.0), heur(0.0), parent(-1){};
    Node(StreetSegmentIdx _seg, IntersectionIdx _name, double _real_time, double _heuristic, IntersectionIdx _parent)
        : seg(_seg), name(_name), real_time(_real_time), heur(_heuristic), parent(_parent){};
    
    bool operator<(const Node& a) const{
        return heur > a.heur;
    }

} Node;

class SegTime {
public:
    double best_time;
    StreetSegmentIdx seg;
    IntersectionIdx prevIntersection;
    SegTime(double t) {
        best_time = t;
    }
};

extern std::vector<StreetSegmentIdx> findSegments(IntersectionIdx intersection_1);
extern std::unordered_map<std::string, std::string const> MapAndFont;

///Data structure for POIs
extern std::vector<ezgl::point2d> ListOfPOIsInType_restaurant;
extern std::vector<ezgl::point2d> ListOfPOIsInType_cafe;
extern std::vector<ezgl::point2d> ListOfPOIsInType_fast_food;
extern std::vector<ezgl::point2d> ListOfPOIsInType_ice_cream;
extern std::vector<ezgl::point2d> ListOfPOIsInType_vending_machine;

extern std::vector<ezgl::point2d> ListOfPOIsInType_bicycle_rental;
extern std::vector<ezgl::point2d> ListOfPOIsInType_bus_station;

extern std::vector<ezgl::point2d> ListOfPOIsInType_pharmacy;
extern std::vector<ezgl::point2d> ListOfPOIsInType_doctors;

extern std::vector<ezgl::point2d> ListOfPOIsInType_school;
extern std::vector<ezgl::point2d> ListOfPOIsInType_place_of_worship;

///Data structure for drawing features
extern std::vector<featureInfo> ListOfFeaturePointsInFeaturePARK;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureBEACH;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureLAKE;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureRIVER;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureISLAND;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureBUILDING;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureGREENSPACE;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureGOLFCOURSE;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureSTREAM;
extern std::vector<featureInfo> ListOfFeaturePointsInFeatureGLACIER;
//extern std::unordered_map < FeatureType, std::vector< std::vector<ezgl::point2d> > >features;
///
extern std::vector<std::vector<StreetSegmentIdx>> intersection_streetSegments; 
extern std::vector<std::vector<IntersectionIdx>> street_intersections;
extern std::vector<std::vector<IntersectionIdx>> adjacent_intersections; 
extern std::vector<double> streetLength;
extern std::unordered_map<OSMID, const OSMNode*> nodes;
extern std::unordered_map<OSMID, const OSMWay*> ways;
extern std::map<std::string, std::vector<StreetIdx>* > streetNames;
extern std::map<std::string, int> fullStreetNames;
extern std::vector<double> streetSegmentTime;
extern std::vector<int> empty;
extern std::vector<std::vector<StreetSegmentIdx>> street_streetSegments;

extern std::map<std::string, LatLon> saved_locations;
extern std::vector<StreetSegmentIdx> motorway_segments;
extern std::vector<StreetSegmentIdx> secondary_way_segments;
extern std::vector<StreetSegmentIdx> trunk_segments;
extern std::vector<StreetSegmentIdx> primary_segments;
extern std::vector<StreetSegmentIdx> tertiary_segments;
extern std::vector<StreetSegmentIdx> unclassified_segments;
extern std::vector<StreetSegmentIdx> residential_segments;
extern std::vector<StreetSegmentIdx> remaining_segments;

extern std::string getOSMWayTagValue(OSMID, std::string);
extern double x_from_lon(double);
extern double y_from_lat(double);
extern double max_speed;


#endif /* DATASTRUCTURES_H */

