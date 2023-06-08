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

#define PI 3.14159265
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "dataStructures.h"

#include "ezgl/point.hpp"

#include <tuple>
#include <cmath> 
#include <cctype>
#include <algorithm>
#include <list>
#include <map>
#include <fstream>
// Included math library for trigonometric functions

// Helper function definitions (Can move to a new .h file later):
std::vector<StreetSegmentIdx> intersection_streetSegmentVectors(IntersectionIdx intersection_id);
void removeDuplicates();


// Global variables
int numOfIntersections;
int numOfStreets;
int numOfStreetSegments;
int numOfPOIs;
int numOfFeatures;
double lat_avg;

std::unordered_map<std::string, std::string const> MapAndFont;

// Global Variable(s): Nested vector
std::vector<std::vector<StreetSegmentIdx>> intersection_streetSegments; 
std::vector<std::vector<IntersectionIdx>> street_intersections;
std::vector<std::vector<IntersectionIdx>> adjacent_intersections; 
std::vector<double> streetLength;
std::unordered_map<OSMID, const OSMNode*> nodes;
std::unordered_map<OSMID, const OSMWay*> ways;
std::vector<const OSMNode*>subway_stations;
std::map<std::string, std::vector<StreetIdx>*> streetNames;
std::map<std::string, int> fullStreetNames;
std::vector<double> streetSegmentTime;
std::vector<int> empty;
std::vector<std::vector<StreetSegmentIdx>> street_streetSegments;
std::vector<StreetSegmentIdx> motorway_segments;
std::vector<StreetSegmentIdx> secondary_way_segments;
std::vector<StreetSegmentIdx> trunk_segments;
std::vector<StreetSegmentIdx> primary_segments;
std::vector<StreetSegmentIdx> tertiary_segments;
std::vector<StreetSegmentIdx> unclassified_segments;
std::vector<StreetSegmentIdx> residential_segments;
std::vector<StreetSegmentIdx> remaining_segments;



std::vector<featureInfo> ListOfFeaturePointsInFeaturePARK;
std::vector<featureInfo> ListOfFeaturePointsInFeatureBEACH;
std::vector<featureInfo> ListOfFeaturePointsInFeatureLAKE;
std::vector<featureInfo> ListOfFeaturePointsInFeatureRIVER;
std::vector<featureInfo> ListOfFeaturePointsInFeatureISLAND;
std::vector<featureInfo> ListOfFeaturePointsInFeatureBUILDING;
std::vector<featureInfo> ListOfFeaturePointsInFeatureGREENSPACE;
std::vector<featureInfo> ListOfFeaturePointsInFeatureGOLFCOURSE;
std::vector<featureInfo> ListOfFeaturePointsInFeatureSTREAM;
std::vector<featureInfo> ListOfFeaturePointsInFeatureGLACIER;

std::vector<ezgl::point2d> ListOfPOIsInType_restaurant;
std::vector<ezgl::point2d> ListOfPOIsInType_cafe;
std::vector<ezgl::point2d> ListOfPOIsInType_fast_food;
std::vector<ezgl::point2d> ListOfPOIsInType_ice_cream;
std::vector<ezgl::point2d> ListOfPOIsInType_vending_machine;

std::vector<ezgl::point2d> ListOfPOIsInType_bicycle_rental;
std::vector<ezgl::point2d> ListOfPOIsInType_bus_station;

std::vector<ezgl::point2d> ListOfPOIsInType_pharmacy;
std::vector<ezgl::point2d> ListOfPOIsInType_doctors;

std::vector<ezgl::point2d> ListOfPOIsInType_school;
std::vector<ezgl::point2d> ListOfPOIsInType_place_of_worship;
double max_speed;

//std::unordered_map < FeatureType, std::vector< std::vector<ezgl::point2d> > >features;
std::map<std::string, LatLon> saved_locations;

// Shannon:
// Loads a map streets.bin and the corresponding osm.bin file
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    // OSM Database requires a different filename
    // Will have osm.bin instead of streets.bin:
    std::string OSM_Name = "osm.bin"; //21 & 3
    std::string osm_filename = map_streets_database_filename;

    // If the filename is not valid:
    if (map_streets_database_filename.find("streets.bin") == 0) {
        return false;
    }

    // Replace streets.bin with osm.bin:
    int count;
    while ((count = osm_filename.find("streets.bin")) != std::string::npos) { 
        osm_filename.replace(count, osm_filename.length(), OSM_Name);
    }

    // Loading map related data structures:
    if (loadStreetsDatabaseBIN(map_streets_database_filename) && loadOSMDatabaseBIN(osm_filename)) {
        load_successful = true;
    } else {
        return false;
    }
    
    // Initialize global variables.
    numOfIntersections = getNumIntersections();
    numOfStreets = getNumStreets();
    numOfStreetSegments= getNumStreetSegments();
    numOfPOIs = getNumPointsOfInterest();
    numOfFeatures = getNumFeatures();
     
    // Uses layer 2 API function to load {map}.streets.bin file
    // Returns true if load successful, false otherwise

    // Initialize 2D vector: intersection_streetSegments
    // Element i is a vector of streetSegment indices that connect to intersectionIdx i.
    adjacent_intersections.resize(numOfIntersections);
    for (int j = 0; j < numOfIntersections; j++) {
        intersection_streetSegments.push_back(intersection_streetSegmentVectors(j));   //iterate through all intersections, 
    }
    
    // Initialize 2D vector: street_intersections
    // Element i is a vector of intersection in street i.
    street_intersections.resize(getNumStreets());
    streetLength.resize(getNumStreets());
    streetSegmentTime.resize(getNumStreetSegments());
    street_streetSegments.resize(getNumStreets());
    std::fill(streetLength.begin(), streetLength.end(), 0);
    std::fill(streetSegmentTime.begin(), streetSegmentTime.end(), 0);

    // For loop adds elements to streetLength and streetSegmentTime:
    // goes through every street segment in the map
    
    // Unordered Map: Key: OSMID; Value: OSM Way Pointer
    int totalNumOfWays = getNumberOfWays();
    for (int wayIdx = 0; wayIdx < totalNumOfWays; wayIdx++) {
        const OSMWay* wayPtr = getWayByIndex(wayIdx);
        OSMID id = wayPtr->id();
        ways[id] = wayPtr; // Giving input id would output an OSM way Pointer
    }
    max_speed = 0;
    for (int streetSegIndex = 0; streetSegIndex < numOfStreetSegments; streetSegIndex++) {
        double length = findStreetSegmentLength(streetSegIndex);
        
        StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegIndex);

        if (segInfo.speedLimit > max_speed) {
            max_speed = segInfo.speedLimit;
        }
        
        OSMID way_OSMID = segInfo.wayOSMID;
        std::string way_type = getOSMWayTagValue(way_OSMID, "highway");
               
        if (way_type == "motorway")  {
            motorway_segments.push_back(streetSegIndex); 

        } 
        else if (way_type == "secondary") {
            secondary_way_segments.push_back(streetSegIndex);
        }
        else{ 
            remaining_segments.push_back(streetSegIndex);
        }
            
        // StreetSegmentTime uses the street segment length calculated above
        // and divides by the speed limit
        streetSegmentTime[streetSegIndex] = length/segInfo.speedLimit;
        StreetIdx st_id = segInfo.streetID;
        
        // Initialize vector: street_streetSegments.
        street_streetSegments[st_id].push_back(streetSegIndex);

        // Initialize vector: streetLength.
        // To find the length of each street - find street segment's street id
        // then add it to the corresponding element of the vector
        streetLength[st_id] += length;
        street_intersections[st_id].push_back(segInfo.from);
        street_intersections[st_id].push_back(segInfo.to);
    }

    removeDuplicates();
    
    // Unordered Map: Key: OSMID; Value: OSM Node Pointer
    int totalNumOfNodes = getNumberOfNodes();
    for (int nodeIdx = 0; nodeIdx < totalNumOfNodes; nodeIdx++) {
        const OSMNode* nodePtr = getNodeByIndex(nodeIdx);
        OSMID id = nodePtr->id();
        nodes[id] = nodePtr; // Giving input id would output an OSM Node Pointer

        for (unsigned j = 0; j < getTagCount(nodePtr); j++) {
            std::pair<std::string, std::string> tagPair = getTagPair(nodePtr, j); 

            if (tagPair.first == "station" && tagPair.second == "subway") {
                subway_stations.push_back(nodePtr);
                break;
            }      
        }
    }
    
    // Unordered Map "streetNames"
    // Key: (string) street names, Value: a VECTOR of StreetIdxs;
    for (int i = 0; i < numOfStreets; i++) {   
        std::string StreetName = getStreetName(i);
        fullStreetNames[StreetName] = i;
        std::transform(StreetName.begin(), StreetName.end(), StreetName.begin(), ::tolower);
        StreetName.erase(std::remove(StreetName.begin(), StreetName.end(), ' '), StreetName.end());

        if (streetNames.find(StreetName) == streetNames.end()) { //if street name key already exists TODO
            streetNames[StreetName] = new std::vector<StreetIdx>();
        }

        streetNames[StreetName]->push_back(i);
        
        // Allows associating multiple StreetIdx values with each string key
        // Allows to use the key to look up a list of street indices
        // Adds elements to the vector using the push_back method.
        // Unordered map having vectors as Values
    }
    
    for (int i = 0; i < getNumIntersections(); i++) {
        lat_avg = lat_avg + getIntersectionPosition(i).latitude()*M_PI/180;
    }
    
    lat_avg = lat_avg / getNumIntersections();
   
    //creating an unordered map of feature type and feature pts data
    //key: feature types
    //value: vectors of feature points
    for (int featureNum = 0; featureNum < getNumFeatures(); featureNum++) {
        int totalNumFeaturePoints = getNumFeaturePoints(featureNum);
        
        std::vector<ezgl::point2d> temp = {};
        
        double minX = -1;
        double minY = -1;
        double maxX = -1;
        double maxY = -1;
        
        for (int featurePointNum = 0; featurePointNum < totalNumFeaturePoints; featurePointNum++) {
            LatLon point_latlon = getFeaturePoint(featureNum, featurePointNum);
            double point_x = kEarthRadiusInMeters*(point_latlon.longitude()*(M_PI/180))*std::cos(lat_avg);
            
            if (minX == -1) {
                minX = point_x;
            } else if (point_x < minX) {
                minX = point_x;
            }
          
            if (maxX == -1) {
                maxX = point_x;
            } else if (point_x > maxX) {
                maxX = point_x;
            }
            
            double point_y = kEarthRadiusInMeters*(point_latlon.latitude()*(M_PI/180));
            
            if (minY == -1) {
                minY = point_y;
            } else if (point_y < minY) {
                minY = point_y;
            }

            if (maxY == -1) {
                maxY = point_y;
            } else if (point_y > maxY) {
                maxY = point_y;
            }
        
            ezgl::point2d point = ezgl::point2d(point_x, point_y);
            temp.push_back(point);
        }
        
        if (getFeatureType(featureNum) == PARK) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeaturePARK.push_back(temp_struct);  

        } else if (getFeatureType(featureNum) == BEACH) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureBEACH.push_back(temp_struct);
            
        } else if(getFeatureType(featureNum) == LAKE) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureLAKE.push_back(temp_struct);
           
        } else if (getFeatureType(featureNum) == RIVER) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            //if( (temp[0].x == temp.back().x) && (temp[0].y == temp.back().y) )
            //{
                //temp_struct.closedPoly = true;
            //}

            if (findFeatureArea(featureNum) != 0) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureRIVER.push_back(temp_struct);

        } else if (getFeatureType(featureNum) == ISLAND) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureISLAND.push_back(temp_struct);
            
        } else if (getFeatureType(featureNum) == BUILDING) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureBUILDING.push_back(temp_struct);

        } else if (getFeatureType(featureNum) == GREENSPACE) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureGREENSPACE.push_back(temp_struct);
            
        } else if (getFeatureType(featureNum) == GOLFCOURSE) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureGOLFCOURSE.push_back(temp_struct);
            
        } else if(getFeatureType(featureNum) == STREAM) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureSTREAM.push_back(temp_struct);
            
        } else if (getFeatureType(featureNum) == GLACIER) {
            featureInfo temp_struct;
            temp_struct.pointsInAFeature = temp;
            
            if ((temp[0].x == temp.back().x) && (temp[0].y == temp.back().y)) {
                temp_struct.closedPoly = true;
            } else {
                temp_struct.closedPoly = false;
            }
            
            temp_struct.min_x = minX;
            temp_struct.max_x = maxX;

            temp_struct.min_y = minY;
            temp_struct.max_y = maxY;
            
            temp_struct.FeatureName = getFeatureName(featureNum);
            temp_struct.featurePtNum = getNumFeaturePoints(featureNum);
            
            ListOfFeaturePointsInFeatureGLACIER.push_back(temp_struct);    
        }
    }
    
   /* features.insert({PARK, ListOfFeaturePointsInFeaturePARK});
    features.insert({BEACH, ListOfFeaturePointsInFeatureBEACH});
    features.insert({LAKE, ListOfFeaturePointsInFeatureLAKE});
    features.insert({RIVER, ListOfFeaturePointsInFeatureRIVER});
    features.insert({ISLAND, ListOfFeaturePointsInFeatureISLAND});
    features.insert({BUILDING, ListOfFeaturePointsInFeatureBUILDING});
    features.insert({GREENSPACE, ListOfFeaturePointsInFeatureGREENSPACE});
    features.insert({GOLFCOURSE, ListOfFeaturePointsInFeatureGOLFCOURSE});
    features.insert({STREAM, ListOfFeaturePointsInFeatureSTREAM});
    features.insert({GLACIER, ListOfFeaturePointsInFeatureGLACIER});
    */
    //Data structure help display different City maps with different fonts
    MapAndFont.insert({"The rest", "Noto Serif CJK SC" });
    MapAndFont.insert({"Cairo or Tehran", "DejaVu Serif Mono"});
    MapAndFont.insert({"Kyiv", "Noto Serif"});
    
    //for (int poiIdx = 0; poiIdx < getNumPointsOfInterest(); poiIdx++) {
     //   std::cout << getPOIType(poiIdx) << std::endl;}
    //Data structure for Points of Interests
    for (int poiIdx = 0; poiIdx < getNumPointsOfInterest(); poiIdx++) {
        LatLon poi_latlon = getPOIPosition(poiIdx);
        double point_x = kEarthRadiusInMeters*(poi_latlon.longitude()*(M_PI/180))*std::cos(lat_avg);
        double point_y = kEarthRadiusInMeters*(poi_latlon.latitude()*(M_PI/180));
        ezgl::point2d poiPoint = ezgl::point2d(point_x, point_y);
        
        if (getPOIType(poiIdx) == "restaurant") {
            ListOfPOIsInType_restaurant.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "cafe") {
            ListOfPOIsInType_cafe.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "fast_food") {
            ListOfPOIsInType_fast_food.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "ice_cream") {
            ListOfPOIsInType_ice_cream.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "vending_machine") {
            ListOfPOIsInType_vending_machine.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "bicycle_rental") {
            ListOfPOIsInType_bicycle_rental.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "bus_station") {
            ListOfPOIsInType_bus_station.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "pharmacy") {
            ListOfPOIsInType_pharmacy.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "doctors") {
            ListOfPOIsInType_doctors.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "school") {
            ListOfPOIsInType_school.push_back(poiPoint);

        } else if (getPOIType(poiIdx) == "place_of_worship") {
            ListOfPOIsInType_place_of_worship.push_back(poiPoint);
        }
    }

    // Load Saved Locations:
    std::ifstream locations;
    locations.open("saved_locations.txt");
    double latitude, longitude;
    std::string name, name_appnd;

    while (locations >> latitude >> longitude >> name) {
        LatLon new_location(latitude, longitude);

        std::getline(locations, name_appnd);
        name = name + name_appnd;

        saved_locations[name] = new_location;
    }

    locations.close();
    
    return load_successful;
}

// Close the map (if loaded)
void closeMap() {
    // Save all the locations that were saved:
    std::ofstream locations;
    locations.open("saved_locations.txt");

    for (std::map<std::string,LatLon>::iterator it = saved_locations.begin(); it != saved_locations.end(); ++it) {
        locations << (it->second).latitude() << "\n" << (it->second).longitude() << "\n" << it->first << "\n";
    }

    locations.close();

    // Clean-up map related data structures here:

    intersection_streetSegments.clear();
    street_intersections.clear();
    adjacent_intersections.clear();
    streetLength.clear();
    streetSegmentTime.clear();
    nodes.clear();
    ways.clear();
    street_streetSegments.clear();

    MapAndFont.clear();

    ListOfFeaturePointsInFeaturePARK.clear();
    ListOfFeaturePointsInFeatureBEACH.clear();
    ListOfFeaturePointsInFeatureLAKE.clear();
    ListOfFeaturePointsInFeatureRIVER.clear();
    ListOfFeaturePointsInFeatureISLAND.clear();
    ListOfFeaturePointsInFeatureBUILDING.clear();
    ListOfFeaturePointsInFeatureGREENSPACE.clear();
    ListOfFeaturePointsInFeatureGOLFCOURSE.clear();
    ListOfFeaturePointsInFeatureSTREAM.clear();
    ListOfFeaturePointsInFeatureGLACIER.clear();
    saved_locations.clear();
    empty.clear();
    
    ListOfPOIsInType_restaurant.clear();
    ListOfPOIsInType_cafe.clear();
    ListOfPOIsInType_fast_food.clear();
    ListOfPOIsInType_ice_cream.clear();
    ListOfPOIsInType_vending_machine.clear();

    ListOfPOIsInType_bicycle_rental.clear();
    ListOfPOIsInType_bus_station.clear();

    ListOfPOIsInType_pharmacy.clear();
    ListOfPOIsInType_doctors.clear();

    ListOfPOIsInType_school.clear();
    ListOfPOIsInType_place_of_worship.clear();

    for (auto const& x : streetNames) {
        delete x.second;
    }

    streetNames.clear();
    fullStreetNames.clear();

    saved_locations.clear();
    motorway_segments.clear();
    secondary_way_segments.clear();
    trunk_segments.clear();
    primary_segments.clear();
    tertiary_segments.clear();
    unclassified_segments.clear();
    residential_segments.clear();
    remaining_segments.clear();

    // Close each database:
    closeStreetDatabase();
    closeOSMDatabase();

    // Uses layer 2 API function to unload map and free memory
    // Other API calls cannot be called until a new map is loaded (only 1 map at a time)
}

// loadMap() & closeMap() Helper Function(s):
// Initialize intersection_streetSegments.
// Creates a new vector for each street intersection
// Create empty vector for each intersection
// iterate through all segments at intersection: 
std::vector<StreetSegmentIdx> intersection_streetSegmentVectors(IntersectionIdx intersection_id) {
    std::vector<StreetSegmentIdx> SSI;

    for (int i = 0; i < getNumIntersectionStreetSegment(intersection_id); i++) {
        int id = getIntersectionStreetSegment(intersection_id, i); //id is the street segment id that incident on the intersection.
        SSI.push_back(id);
        // Put adjacent intersections into the adjacent_intersections vector.  

        StreetSegmentInfo segInfo = getStreetSegmentInfo(id);
        
        if (segInfo.oneWay) {
            if (intersection_id == segInfo.from) {
                adjacent_intersections[intersection_id].push_back(segInfo.to);
            }
        } else { //not one-way
            if (intersection_id == segInfo.from) {
                adjacent_intersections[intersection_id].push_back(segInfo.to);
            } else {
                adjacent_intersections[intersection_id].push_back(segInfo.from);
            }              
        }
    }

    return SSI;
}

// Removes all the duplicates in street_intersections and adjacent_intersections.
void removeDuplicates() {
    std::vector<IntersectionIdx>::iterator it;

    for (int i = 0; i < numOfStreets; i++) {
        std::vector<IntersectionIdx> vec1 = street_intersections[i];
        std::sort(vec1.begin(),vec1.end());
        it = std::unique(vec1.begin(),vec1.end()); 

        // Resizing the vector so as to remove the undefined terms
        vec1.resize(std::distance(vec1.begin(), it));
        street_intersections[i] = vec1;      
    }
    
    for (int i = 0; i < numOfIntersections; i++) {
        std::vector<IntersectionIdx> vec2 = adjacent_intersections[i];
        std::sort(vec2.begin(),vec2.end());
        it = std::unique(vec2.begin(),vec2.end()); 

        // Resizing the vector so as to remove the undefined terms
        vec2.resize(std::distance(vec2.begin(), it));
        adjacent_intersections[i] = vec2;      
    }

    return;
}

// Returns the distance between two (latitude,longitude) coordinates in meters
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2) {
   // Convert LatLon to (x,y):
   // Given by the equations:
       // x = (Earth's Radius)*(longitude)*cos(average latitude)
       // y = (Earth's Radius)*(latitude)
   double x1, x2, y1, y2;

   double lat_avg1 = ((point_1).latitude()*M_PI/180 + (point_2).latitude()*M_PI/180)/2;

   // (x1, y1) for point_1:
   x1 = kEarthRadiusInMeters*((point_1).longitude()*(M_PI/180))*cos(lat_avg1);
   y1 = kEarthRadiusInMeters*((point_1).latitude()*(M_PI/180));

   // (x2, y2) for point_2:
   x2 = kEarthRadiusInMeters*((point_2).longitude()*(M_PI/180)*cos(lat_avg1));
   y2 = kEarthRadiusInMeters*(point_2).latitude()*(M_PI/180);

   // Distance:
   double distance;
   distance = sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) );

   return distance;
}

// Finds length of the street in meters
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {
    if (street_segment_id >= numOfStreetSegments || street_segment_id < 0) {
        return 0;
    }

    StreetSegmentInfo Street_Info = getStreetSegmentInfo(street_segment_id);
    LatLon point_1 = getIntersectionPosition(Street_Info.from);
    LatLon point_2 = getIntersectionPosition(Street_Info.to);
    double length = 0;
    
    if (Street_Info.numCurvePoints == 0) {
        // If the street segment is a straight line:
        return findDistanceBetweenTwoPoints(point_1, point_2);
        
    } else {
        // If not a straight line, split up the street segment into small straight lines
        // and add all of the lengths up
        int curve_total = Street_Info.numCurvePoints;
        int counter = 0;

        LatLon curve_1, curve_2;
        curve_1 = getStreetSegmentCurvePoint(street_segment_id, counter);
        length = length + findDistanceBetweenTwoPoints(point_1, curve_1);

        if (curve_total == 1) {
            return length + findDistanceBetweenTwoPoints(curve_1, point_2);;
        }

        for (; counter < curve_total-1; counter++) {
            curve_1 = getStreetSegmentCurvePoint(street_segment_id, counter);
            curve_2 = getStreetSegmentCurvePoint(street_segment_id, counter+1);
            length = length + findDistanceBetweenTwoPoints(curve_1, curve_2);
        }
        
        return length + findDistanceBetweenTwoPoints(curve_2, point_2);
    }
}

// Uses speed limit and distance of street segment to find travel time (T = distance/speed limit)
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    // Uses a vector streetSegmentTime for performance
    // Elements are instantiated in loadMap
    if (street_segment_id >= numOfStreetSegments || street_segment_id < 0) {
        return 0;
    }

    return streetSegmentTime[street_segment_id];
}

// Iris:
// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
    if (street_id >= numOfStreets || street_id < 0) {
        return 0;
    }

    // Uses a vector streetLength for performance:
    return streetLength[street_id];
}

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POItype) {
    // Find total number of POIs
    int totalNumberOfPOI = getNumPointsOfInterest();

    // Created variables to store: 
    // 1. distance from my_position
    // 2. smallest distance from my_position
    // Initialized smallestDistance to a large number
    // 3. string closestPOI to store the name of the POI
    double distance = 0;
    double smallestDistance = 999999999999999999;
    POIIdx closestPOI = 0; 

    // Looping through all POI
    // Find the POIs with the type we searching for
    // Get the distance and compare to smallest distance existed
    // Update smallest distance, and the closestPOI to the new POI name if needed
    for (int poiIdx = 0; poiIdx < totalNumberOfPOI; poiIdx++) {
        std::cout << getPOIType(poiIdx) << std::endl;
        if (getPOIType(poiIdx) == POItype ) {

            LatLon POIPosition = getPOIPosition(poiIdx);
            distance = findDistanceBetweenTwoPoints(my_position, POIPosition);

            if (distance < smallestDistance) {
                smallestDistance = distance;
                closestPOI = poiIdx;
            }
        }
    }
    
    // Looped through all POIs, return the name of the closest one found
    return closestPOI;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id) {
    // Check if it's closed polygon
    double areaOfPolygon = 0;
    LatLon firstFeaturePointPosition = getFeaturePoint(feature_id, 0);
    LatLon lastFeaturePointPosition = getFeaturePoint(feature_id, getNumFeaturePoints(feature_id) - 1);

    double x_1, xn, y_1, yn;
    double lat_avg2 = 0;

    // Calculates the average latitude of all the feature points:
    for (int i = 0; i < getNumFeaturePoints(feature_id); i++) {
        LatLon FeaturePointPosition = getFeaturePoint(feature_id, i);
        lat_avg2 = lat_avg2 + (FeaturePointPosition).latitude()*M_PI/180;
    }

    lat_avg2 = lat_avg2/getNumFeaturePoints(feature_id);
    
    // (x1, y1) for point_1:
    x_1 = kEarthRadiusInMeters*((firstFeaturePointPosition).longitude()*(M_PI/180))*cos(lat_avg2);
    y_1 = kEarthRadiusInMeters*((firstFeaturePointPosition).latitude()*(M_PI/180));

    // (x2, y2) for point_2:
    xn = kEarthRadiusInMeters*((lastFeaturePointPosition).longitude()*(M_PI/180)*cos(lat_avg2));
    yn = kEarthRadiusInMeters*((lastFeaturePointPosition).latitude()*(M_PI/180));

    if (x_1 != xn && y_1 != yn) {
        // It's a polyline
        return 0;
    }

    // It's a closed polygon
    // Area = 1/2 *|(𝑥1𝑦2–𝑥2𝑦1)+(𝑥2𝑦3–𝑥3𝑦2)+(𝑥3𝑦4–𝑥4𝑦3)+(𝑥4𝑦1–𝑥1𝑦4)|. 
    int totalNumOfFeaturePoint = getNumFeaturePoints(feature_id);
    for (int featurePointNum = 0; featurePointNum < totalNumOfFeaturePoint - 1 ; featurePointNum++) {
        LatLon featurePointPosition_1 = getFeaturePoint(feature_id, featurePointNum);
        LatLon featurePointPosition_2 = getFeaturePoint(feature_id, featurePointNum + 1);
        double x1, x2, y1, y2;

        // (x1, y1) for point_1:
        x1 = kEarthRadiusInMeters*((featurePointPosition_1).longitude()*(M_PI/180))*cos(lat_avg2);
        y1 = kEarthRadiusInMeters*((featurePointPosition_1).latitude()*(M_PI/180));

        // (x2, y2) for point_2:
        x2 = kEarthRadiusInMeters*((featurePointPosition_2).longitude()*(M_PI/180)*cos(lat_avg2));
        y2 = kEarthRadiusInMeters*((featurePointPosition_2).latitude()*(M_PI/180));

        areaOfPolygon = areaOfPolygon + (x1 * y2 - x2 * y1);
    }

    areaOfPolygon += xn * y_1 - x_1 * yn;
    areaOfPolygon = 0.5 * std::abs(areaOfPolygon);

    return areaOfPolygon; 
}

// Return the value associated with this key on the specified OSMNode. 
// If this OSMNode does not exist in the current map, 
// or the specified key is not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue(OSMID OSMid, std::string key) {
    const auto found = nodes.find(OSMid);
    if (found == nodes.end()) {
        return "";
    }
    // const OSMNode* nodePtrWithCorrectId = found -> second; // second is a const OSMNode pointer

    for (int i = 0; i < getTagCount(found -> second); ++i) {
        std::string key1, value1;
        std::tie(key1, value1) = getTagPair(found -> second, i);

        if (key1 == key) {
            return value1;
        }
    }

    return "";
}

std::string getOSMWayTagValue(OSMID OSMid, std::string key) {
    const auto found = ways.find(OSMid);
    if (found == ways.end()) {
        return "";
    }

    for (int i = 0; i < getTagCount(found -> second); ++i) {
        std::string key1, value1;
        std::tie(key1, value1) = getTagPair(found -> second, i);

        if (key1 == key) {
            return value1;
        }
    }

    return "";
}

// Returns all street ids corresponding to street names that start with the given prefix
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
// For example, both "bloor " and "BloOrst" are prefixes to "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty (length 0) string, 
// but your program must not crash if street_prefix is a length 0 string.
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {   
    std::vector<StreetIdx> StreetVector;        
    
    if (street_prefix.empty()) {
        return StreetVector;
    }

    std::transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower);
    street_prefix.erase(std::remove(street_prefix.begin(), street_prefix.end(), ' '), street_prefix.end());

    auto search = streetNames.lower_bound( street_prefix );

    for (;(search != streetNames.end()) && (search->first).find(street_prefix) == 0; search++) {
        StreetVector.insert(StreetVector.end(), (*(search->second)).begin(), (*(search->second)).end());

    }

    return StreetVector;
}

// Sean
// Returns all intersections reachable by traveling down one street segment from the given intersection 
// (hint: you can't travel the wrong way on a 1-way street)
// the returned vector should NOT contain duplicate intersections
// Corner case: cul-de-sacs can connect an intersection to itself (from and to intersection on  street segment are the same). 
// In that case include the intersection in the returned vector (no special handling needed).
// Speed Requirement --> high 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id) {
    if (0 <= intersection_id && intersection_id < numOfIntersections) {
        return adjacent_intersections[intersection_id]; // Access 2d vector adjacent_intersections.
    }

    // Invalid inputs: return an empty vector
    else {
        return empty;
    }
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {
    double distance, min_distance = 0;
    IntersectionIdx id = 0;

    // Check the distance between given point and all the intersections.
    for (IntersectionIdx i = 0; i < numOfIntersections; i++) {
        distance = findDistanceBetweenTwoPoints(getIntersectionPosition(i),my_position);

        // Update the shortest distance and the closest intersection.
        if (i == 0 || distance < min_distance) {
            id = i;
            min_distance = distance;
        }
    }

    return id;
}

// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    if (0 <= intersection_id && intersection_id < numOfIntersections) {
        return intersection_streetSegments[intersection_id]; // Access 2d vector intersection_streetSegments.
    }
        
    // Invalid inputs: return an empty vector
    else {
        return empty;
    }
}

// Returns all intersections along the given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    if (0 <= street_id && street_id < numOfStreets) {
        return street_intersections[street_id]; // access 2d vector street_intersections.
    }
        
    // Invalid inputs: return an empty vector
    else {
        return empty;
    }
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets that intersect and a length 0 vector for streets that do not.
// For unusual curved streets it is possible to have more than one intersection at which two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2) {
    if ((0 <= street_id1 && street_id1 < numOfStreets) && (0 <= street_id2 && street_id2 < numOfStreets)) {
       std::vector<IntersectionIdx> vec1 = street_intersections[street_id1]; //vec1 has all the intersections in street id1;
       std::vector<IntersectionIdx> vec2 = street_intersections[street_id2]; //vec2 has all the intersections in street id2;
       std::vector<IntersectionIdx> vec3(vec1.size() + vec2.size());

       // Find common intersections in vec1 and vec2.
       std::vector<IntersectionIdx>::iterator it, end;
       end = std::set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(),vec3.begin());

       int count = 0; // Count the number of valid intersections in vec3.
       for (int i = 0; i < vec3.size(); i++) {
           if (vec3[i] != 0) {
                vec3[count] = vec3[i]; 
                count++;
            }
        }

       vec3.resize(count);  
       return vec3;
    }

    // Invalid inputs: return an empty vector
    else {
        return empty;
    }
}