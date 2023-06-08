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
#ifndef OSMDATABASEAPI_H
#define OSMDATABASEAPI_H
#include <vector>
#include "OSMID.h"
#include "LatLon.h"

#include "OSMEntity.h"
#include "OSMNode.h"
#include "OSMWay.h"
#include "OSMRelation.h"


/*******************************************************************************
 * LAYER-1 API INTRODUCTION
 *
 * ** This is a more complex API than layer 2 which is built on top of it; 
 * please start with layer 2. The information accessible here
 * is not _required_ for any milestone, though it may help you provide 
 * additional features. **
 *
 * The Layer-1 API handles parsing the OSM XML data, and removes some features 
 * which are not of interest (eg. timestamps, the user names who made changes) 
 * to conserve time and space. It also loads/saves a binary format that is 
 * pre-parsed and so more compact and faster to load.
 *
 *
 * The object model of this library and API follows the OSM model closely. It 
 * is a lean, flexible model with only three distinct entity types, each of 
 * which carries all of its non-spatial data as attributes.
 *
 * There are three types of OSM Entities:
 *    Node      A point with lat/lon coordinates and zero
 *    Way       A collection of nodes, either a path (eg. street, bike path) or 
 *              closed polygon (eg. pond, building)
 *    Relation  A collection of nodes, ways, and/or relations that share some 
 *              common meaning (eg. large lakes/rivers)
 *
 * Each entity may have associated zero or more attributes of the form key=value,
 * eg. name="CN Tower" or type="tourist trap".
 *
 * After processing by osm2bin, files are stored in {mapname}.osm.bin ready to 
 * use in this API.
 */

// Load the (lower-level) layer-1 OSM database; call this before calling any other
// layer-1 API function. Returns true if successful, false otherwise.
// These files are named {cityname}.osm.bin
bool loadOSMDatabaseBIN(const std::string&);

// Close the layer-1 OSM database and release memory. You must close one map 
// before loading another.
void closeOSMDatabase();


/*******************************************************************************
 * Entity access
 *
 * Provides functions to iterate over all nodes, ways, relations in the database.
 *
 * NOTE: The indices here have no relation at all to the indices used in the 
 * layer-2 API, or to the OSM IDs. You can getNodeByIndex for idx values from
 * 0 to getNumberOfNodes()-1; way and relation indices behave similarly.
 *
 * Once you have the OSMNode/OSMWay/OSMRelation pointer, you can use it to 
 * access methods of those types or the tag interface described below.
 * The most basic method supported by OSMNode, OSMWay, and OSMRelation is
 * id(), which returns the OSMID (a basic class containing a 64-bit integer).
 * The OSMID for any OSMNode/OSMWay/OSMRelation is globally unique (never
 * re-used for any other OSMNode etc. anywehere in the world).
 * For example:
 * const OSMNode *e = getNodeByIndex(0); // Gets the first OSM Node in this city
 * OSMID id = e->id();                   // A 64-bit int (big); unique id
 */

int getNumberOfNodes();
int getNumberOfWays();
int getNumberOfRelations();

const OSMNode*      getNodeByIndex     (int idx);
const OSMWay*       getWayByIndex      (int idx);
const OSMRelation*  getRelationByIndex (int idx);


/*******************************************************************************
 * Entity tag access
 *
 * OSMNode, OSMWay, and OSMRelation are all objects derived from OSMEntity, 
 * which carries attribute tags. The functions below allow you to iterate 
 * through the tags on a given entity acquired above. For example, for 
 * an OSMEntity* e:
 *
 * for(int i=0;i<getTagCount(e); ++i)
 * {
 * 		std::string key,value;
 * 		std::tie(key,value) = getTagPair(e,i);
 * 		// ... do useful stuff ...
 * }
 */

int getTagCount(const OSMEntity* e);
std::pair<std::string, std::string> getTagPair(const OSMEntity* e, int idx);

/*******************************************************************************
 * OSMNode Functions (OSMNode is a point with lat/lon coordinates)
 *
 * The getNodeCoords() function returns the coordinates of the OSMNode
 */
LatLon getNodeCoords(const OSMNode* n);

/*******************************************************************************
 * OSMWay Functions (OSMWay is a collection of nodes)
 *
 * The getWayMembers() function returns the OSMIDs of the nodes that form the way
 *
 * The isClosedWay() function returns false if the OSMWay is open (a path) and 
 * returns true if the OSMWay is closed (closed polygon) 
 */
const std::vector<OSMID>& getWayMembers(const OSMWay* w);
bool isClosedWay(const OSMWay* w);

/*******************************************************************************
 * OSMRelation Functions (OSMRelation is a collection of nodes, ways, and/or relations)
 *
 * The getRelationMembers() function returns the TypedOSMIDs of the nodes/ways/
 * relations that form the relation. 
 * TypedOSMID can do everyting an OSMID can (i.e. it inherits from OSMID), but 
 * it also has a type, which is either Node, Way or Relation.
 * For example:
 * (tid.type() == TypedOSMID::Way) returns true if tid is an OSMID of a way
 *
 * The getRelationMemberRoles() function returns the role of each member of the 
 * relation.
 * A role is a string that may exist (it is optional in OSM) that describes the 
 * role of a member within the relation.
 * For example, a member (a way) of a relation that represents a subway line can 
 * have a "platform" role to represent a platform in that line.
 */
std::vector<TypedOSMID> getRelationMembers(const OSMRelation* r);
std::vector<std::string> getRelationMemberRoles(const OSMRelation* r);

/*******************************************************************************
 * Relationship between StreetsDatabaseAPI elements and OSMDatabaseAPI elements
 * 
 * The StreetsDatabaseAPI intersections, streetSegments, POIs and Features
 * are derived from OSM entities as listed below. You can use StreetDatabaseAPI
 * calls to find the OSMID from which an intersection etc. came.
 * The processing of OSM data to create the StreetsDatabase representation is 
 * complex and is done within the osm2bin program that creates streets.bin files.
 *
 * Intersections: come from certain OSMNodes (only some represent intersections)
 * streetSegments: come from certain OSMWays. A single OSMWay can lead to more 
 *    than one streetSegment.
 * POIs: come from OSMNodes with a tag of amenity; the value is the POI type.
 * Features: can come from either an OSMWay or an OSMRelation (and degenerate
 *      single point features come from OSMNodes). osm2bin extracts features 
 *      when the key or key,value pair on the OSM entity matches our feature 
 *      list, e.g. key = building, or key,value = waterway,river and so on.
 */

#endif
