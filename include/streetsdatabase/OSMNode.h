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
#include "OSMEntity.h"

#include <boost/serialization/vector.hpp>

#include "LatLon.h"

#include <limits>

/** Layer-1 OSM node (attributes + coordinates)
 *
 * You should use the top-level methods in OSMDatabaseAPI.h to get the
 * coordinates of an OSM Node instead of the lower-level implementation
 * methods in this file.
 *
 * Students don't need to read this file.
 */

class OSMNode : public OSMEntity
{
public:
	OSMNode(){}
    OSMNode(OSMID id,std::vector<std::pair<TagStringFlyweight,TagStringFlyweight>>&& tags,LatLon coords);

    OSMNode(OSMNode&&) = delete;
    OSMNode(const OSMNode&) = default;
    OSMNode& operator=(const OSMNode&)=default;
    OSMNode& operator=(OSMNode&&) = delete;

    LatLon	coords()	const { return m_coords; }

private:
    LatLon m_coords;

    friend class boost::serialization::access;
    template<class Archive>void serialize(Archive& ar, const unsigned)
    	{ ar & boost::serialization::base_object<OSMEntity>(*this) & m_coords; }

};

std::ostream& operator<<(std::ostream&, const OSMNode&);
