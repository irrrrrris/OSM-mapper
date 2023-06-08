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


/** Layer-1 OSM relation
 *
 * A relation is an OSM entity that also holds a set of members.
 * Each member has a role and is referred to by type (node/way/rel) and OSM ID.
 *
 * Role is stored as an integer index into a string table.
 *
 * You should use the top-level methods in OSMDatabaseAPI.h to get the
 * members and member roles of an OSM Relation instead of the lower-level implementation
 * methods in this file.
 *
 * Students don't need to read this file.
 */

class OSMRelation : public OSMEntity
{
public:
	struct Member;

	OSMRelation(){}
    OSMRelation(OSMID id_,std::vector<std::pair<TagStringFlyweight,TagStringFlyweight>>&& tags_,std::vector<Member>&& members_) :
    	OSMEntity(id_,std::move(tags_)),
    	m_members(std::move(members_))
		{}

    const std::vector<Member>& members() const { return m_members; }

private:
    std::vector<Member> m_members;

    template<class Archive>void serialize(Archive& ar, const unsigned)
    	{ ar & boost::serialization::base_object<OSMEntity>(*this) & m_members; }
    friend boost::serialization::access;

};

struct OSMRelation::Member
{
	TypedOSMID			tid;
    TagStringFlyweight 	role; 		// Don't use this directly, use getRelationMemberRole in OSMDatabaseAPI.h

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& ar, const unsigned)
    	{ ar & tid & role; }
};
