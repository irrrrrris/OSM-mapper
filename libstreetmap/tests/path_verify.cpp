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
#include <vector>
#include <algorithm>
#include <cassert>
#include "StreetsDatabaseAPI.h"

#include "path_verify.h"
#include "m3.h"

namespace ece297test {
bool path_is_legal(const IntersectionIdx start_intersection, const IntersectionIdx end_intersection, const std::vector<StreetSegmentIdx>& path) {
    std::vector<IntersectionIdx> end_intersections = {end_intersection};

    return path_is_legal(start_intersection, end_intersections, path);
}

bool path_is_legal(const IntersectionIdx start_intersection, const std::vector<IntersectionIdx>& end_intersections, const std::vector<StreetSegmentIdx>& path) {
    //'start_intersection' is the intersection id of the starting intersection
    //'end_intersection' is the intersection id of the ending intersection
    //'path' is a vector street segment id's

    if (path.size() < 1) {
        std::cerr << "Invalid path: path was empty\n";

        return false; //If it is a valid path it must contain at-least one segment

    } else {
        //General case
        //To verify the path by walking along each segment checking:
        //  * That we are going a legal direction (i.e. not against one-ways)
        //  * That each segment is connected to the previous intersection
        //  * That the final segment is connected to a valid end_intersection
        //We start our intersection at the start_intersection
        assert(path.size() >= 1);

        int curr_intersection = static_cast<int>(start_intersection);

        for (size_t i = 0; i < path.size(); i++) {
            StreetSegmentInfo seg_info = getStreetSegmentInfo(path[i]);

            //Are we moving forward or back along the segment?
            bool seg_traverse_forward;
            if (seg_info.from == curr_intersection) {
                //We take care to check 'from' first. This ensures
                //we get a reasonable traversal direction even in the 
                //case of a self-looping one-way segment

                //Moving forwards
                seg_traverse_forward = true;

            } else if (seg_info.to == curr_intersection) {
                //Moving backwards
                seg_traverse_forward = false;

            } else {
                assert(seg_info.from != curr_intersection && seg_info.to != curr_intersection);

                std::cerr << "Invalid path: path disconnected"
                          << " (intersection " << curr_intersection 
                          << " does not connect to street segment " << path[i] 
                          << " at path index " << i << ")\n";

                //This segment isn't connected to the current intersection
                return false;
            }

            //Are we going the wrong way along the segment?
            if (!seg_traverse_forward && seg_info.oneWay) {
                std::cerr << "Invalid path: Attempting to traverse against one-way"
                          << " (from intersection " << curr_intersection 
                          << " along street segment " << path[i] 
                          << " at path index " << i << ")\n";
                return false;
            }

            //Advance to the next intersection
            curr_intersection = (seg_traverse_forward) ? seg_info.to : seg_info.from;

        }

        //We should be at a valid end intersection
        auto iter = std::find(end_intersections.begin(), end_intersections.end(), curr_intersection);
        if (iter == end_intersections.end()) {
            //Not at a valid end intersection
            std::cerr << "Invalid courier path: last path segment not connected to a valid end intersection\n";
            return false;
        }

    }

    //Everything validated
    return true;
}

}
