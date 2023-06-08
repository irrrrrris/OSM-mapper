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
#include <boost/container/flat_map.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>

#include "SerializeBoostFlatMap.hpp"

#include "OSMID.h"
#include "IndexedFlyweight.hpp"

/** OSMEntity is a base class for OSMNode, OSMWay, and OSMRelation that holds the OSM ID and the tags.
 * Use .id() to get the OSMID for this entity.
 * Tags are key,value pairs that provide information about that OSMEntity;
 * see the OpenStreetMap wiki for information on various key,value pairs that
 * may be set on an OSM Entity.
 * You should use the tag access methods in OSMDatabaseAPI.h to get the
 * key,value pairs on an OSM Entity instead of the lower-level implementation
 * methods in this file.
 *
 * The rest of this file defines the internals of OSMEntity, 
 * but students don't need to read them
 */

typedef IndexedFlyweightFactory<std::string,unsigned> 	TagStringFlyweightFactory;
typedef TagStringFlyweightFactory::Flyweight 			TagStringFlyweight;
typedef TagStringFlyweightFactory::BoundFlyweight		TagStringBoundFlyweight;


/* To save space, tag keys and values are stored as "flyweights" that store a reference (index) into a vector
 * of strings held in the OSMDatabase. That permits encoding a key-value pair as two integers rather than saving
 * all of the bytes of the string, which has advantages when there is heavy reuse.
 *
 * On its own, the flyweight (which is really just an index) can only be assigned and compared for equality.
 *
 * It must be bound to the table that generated it to be able to produce a string by looking up.
 * Once bound, the implicit conversion operator permits accessing the bound flyweight as if it was a const std::string&.
 *
 */

class OSMEntity
{
public:
    // Get the OSMID id. Only function needed by students.
    OSMID id() const { return m_id; }  

private:
	typedef boost::container::flat_map<TagStringFlyweight,TagStringFlyweight,TagStringFlyweight::IndexOrder>	container_type;

public:

	OSMEntity(){}
    OSMEntity(
    		OSMID id,
			std::vector<std::pair<TagStringFlyweight,TagStringFlyweight>>&& tags);

    OSMEntity(OSMEntity&&) = default;
    OSMEntity(const OSMEntity&) = default;
    OSMEntity& operator=(const OSMEntity&) = default;
    OSMEntity& operator=(OSMEntity&&) = default;

    virtual ~OSMEntity();

    /** Provides an object that permits iteration ( .begin()/.end() ) and lookup by TagStringFlyweight key []
     * Values returned are TagStringFlyweight type, and must be passed to the corresponding TagStringFlyweightFactory to be
     * converted to strings. */
    class TagLookup;
    TagLookup tags() const;

private:
    OSMID			m_id;  
    container_type 	m_tags;  

    // load/save via boost serialization
    template<class Archive>void serialize(Archive& ar, const unsigned ver)
    	{ ar & m_id;
    		serialize_flat_map(ar,m_tags,ver); }
    friend class boost::serialization::access;
};

class OSMEntity::TagLookup
{
public:
	TagLookup(const container_type& tags) :
		m_tags(tags){}

	// provide lookup, returning an invalid TagStringFlyweight if key does not exist
	TagStringFlyweight operator[](const TagStringFlyweight k) const;
	std::pair<TagStringFlyweight,TagStringFlyweight> operator[](unsigned i) const;

	std::size_t			size() const { return m_tags.size(); }

	// provide iteration
	OSMEntity::container_type::const_iterator begin() 	const { return m_tags.begin(); }
	OSMEntity::container_type::const_iterator end() 	const { return m_tags.end(); }

private:
	const container_type& 		m_tags;
};

inline OSMEntity::TagLookup OSMEntity::tags() const { return TagLookup(m_tags); }
