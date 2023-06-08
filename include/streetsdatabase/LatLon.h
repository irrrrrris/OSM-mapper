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
/*
 * LatLon.h
 *
 *  Created on: Dec 17, 2015
 *      Author: jcassidy
 */

#ifndef LATLON_H_
#define LATLON_H_

#include <limits>
#include <iostream>
#include <utility>
#include <array>

// Turn off some warnings before inluding boost serialization as we 
// can't remove the non-warning-clean code in it. Then turn them back
// on again.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include <boost/serialization/access.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/* This file provides two simple classes for dealing with positions that are 
 * latitude and longitude coordinates.
 * LatLon stores a latitude and longitude in decimal degrees.
 * +latitude is north
 * +longitude is east
 *
 * Students only need to use the .latitude() and .longitude() functions in LatLon
 * which return the double precision latitude and longitude in degrees. 
 *
 * LatLonBounds stores two LatLon objects, one (.min) for the minimum latiude and 
 * longitude and one (.max) for the maximum. This is useful for combputing bounding
 * boxes around parts of a map. 
 *
 */

class LatLon
{
public: 
    // Accessors, which convert to latitude or longitude to double and return it
    double latitude() const { return m_lat; }
    double longitude() const { return m_lon; }

    // Constructors
    LatLon(){}
    explicit LatLon(float lat_, float lon_) : m_lat(lat_),m_lon(lon_){}

private:
 /* (latitude,longitude) are held as single-precision float to save space and match the precision 
  * given by OSM, but returned by the latitude() and longitude() accessors as double because
  * single-precision arithmetic on small changes in latitude and longitude has been found to cause 
  * too much error in distance calculations etc. Returning double type ensures computation with 
  * latitude and longitude will be done in double precision by default, which is what you want.
  */
    float m_lat = std::numeric_limits<float>::quiet_NaN();
    float m_lon = std::numeric_limits<float>::quiet_NaN();

    // For creating .bin files for rapid loading; students don't need to use
    friend class boost::serialization::access;
    template<class Archive>void serialize(Archive& ar, unsigned int)
    	{ ar & m_lat & m_lon; }
};

std::ostream& operator<<(std::ostream& os,LatLon);  // Output a LatLon

// true if a.latitude < b.latitude && a.longitude < b.longitude. Useful if you need to sort LatLon values
bool operator<(const LatLon& a, const LatLon& b);   
bool operator==(const LatLon& a, const LatLon& b);  // true if both latitude and longitude are exactly equal

// A struct that holds two LatLon points representing the minimum and maximum 
// latitudes and longitudes in a bounding box 
struct LatLonBounds {
    LatLon min;
    LatLon max;
};


#endif /* LATLON_H_ */
