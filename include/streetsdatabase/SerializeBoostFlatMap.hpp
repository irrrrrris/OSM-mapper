/* 
 * Copyright 2022 University of Toronto
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
 * SerializeBoostFlatMap.hpp
 *
 *  Created on: Jan 6, 2017
 *      Author: jcassidy
 */

#ifndef LIBS_STREETSDATABASE_SRC_SERIALIZEBOOSTFLATMAP_HPP_
#define LIBS_STREETSDATABASE_SRC_SERIALIZEBOOSTFLATMAP_HPP_

#include <boost/container/flat_map.hpp>
#include <boost/range/algorithm.hpp>

/** This file is for internal use; students don't need to read it **/

template<class Archive,typename Key,typename T,typename Compare,typename Allocator>
	void serialize_flat_map(Archive& ar,boost::container::flat_map<Key,T,Compare,Allocator>& m,const unsigned)
{
	BOOST_STATIC_ASSERT(Archive::is_saving::value ^ Archive::is_loading::value);

	if (Archive::is_saving::value)
	{
		std::vector<std::pair<Key,T>> v(m.size());
		boost::copy(m,v.begin());

		ar & v;
	}
	else if (Archive::is_loading::value)
	{
		std::vector<std::pair<Key,T>> v;
		ar & v;

		m.clear();
		m.insert(boost::container::ordered_unique_range,v.begin(),v.end());
	}
}


#endif /* LIBS_STREETSDATABASE_SRC_SERIALIZEBOOSTFLATMAP_HPP_ */
