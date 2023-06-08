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
#pragma once //protects against multiple inclusions of this header file

#include <string>
#include <vector>

#include "StreetsDatabaseAPI.h"

#include "LatLon.h"
#include "OSMID.h"

// For milestone 4 only! Keep this #define above out until milestone 4. It gives
// a simple solution to milestone 3, for teams that had trouble on m3.
#define FIND_SIMPLE_PATH

// Returns a simple path (route) between the start intersection (1st object in intersect_ids pair)
// and the destination intersection(2nd object in intersect_ids pair),
// if one exists. This routine should return the shortest path
// between the given intersections but it does not consider turn penalty (in seconds).  
// If no path exists, this routine returns an empty (size == 0) vector.  
// If more than one path exists, the path with the shortest travel time is returned. 
// The path is returned as a vector of street segment ids; traversing these street segments, 
// in the returned order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findSimplePath(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
