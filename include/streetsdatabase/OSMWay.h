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

#include <vector>

/** Layer-1 OSM way
 *
 * An OSM way is an OSMEntity with a sequence of node references.
 *
 * The way may be an open, linear feature (eg. road) or a closed polygon (eg. park)
 *
 * You should use the top-level methods in OSMDatabaseAPI.h to get the
 * members and type of an OSM Way instead of the lower-level implementation
 * methods in this file.
 *
 * Students don't need to read this file.
 */

class OSMWay : public OSMEntity
{
public:
	OSMWay(){}
    OSMWay(const OSMWay&) = default;
    OSMWay(OSMWay&& w) = default;
    OSMWay& operator=(const OSMWay&) = default;

    OSMWay(OSMID id_,std::vector<std::pair<TagStringFlyweight,TagStringFlyweight>>&& tags_,std::vector<OSMID>&& ndrefs_) :
    	OSMEntity(id_,std::move(tags_)),
		m_ndrefs(std::move(ndrefs_)){}

    /// Determine if the way is closed (represents a polygon) or open (represents a linear feature)
    bool isClosed() const {
        return m_ndrefs.front() == m_ndrefs.back();
    }

    // data access
    const std::vector<OSMID>& ndrefs() const {
        return m_ndrefs;
    }

private:
    std::vector<OSMID> m_ndrefs;

    friend std::ostream& operator<<(std::ostream& os,const OSMWay& w)
    {
    	return os << "Way id  W" << w.id() << " with " << w.ndrefs().size() << " node refs";
    }

    friend class boost::serialization::access;
    template<class Archive>void serialize(Archive& ar, unsigned int)
    	{ ar & boost::serialization::base_object<OSMEntity>(*this) & m_ndrefs; }
};
