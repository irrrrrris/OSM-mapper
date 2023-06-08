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
 * StringFlyweight.hpp
 *
 *  Created on: Jan 8, 2017
 *      Author: jcassidy
 */

#ifndef STREETSDATABASE_SRC_INDEXEDFLYWEIGHT_HPP_
#define STREETSDATABASE_SRC_INDEXEDFLYWEIGHT_HPP_

#include <boost/optional.hpp>
#include <unordered_map>

#include <boost/range/algorithm.hpp>
#include <vector>

namespace boost { namespace serialization { class access; }}


/** This file is for internal use; students don't need to read it **/


/** A flyweight factory that holds an unordered_map of values to indices, plus a vector to provide efficient
 * mapping back to the values.
 *
 * Unlike a flyweight based on pointers, this is easily serialized/deserialized and doesn't require a container
 * that preserves element locations.
 *
 * 	The Flyweight simply holds the index, and provides equality comparison between indices. A Flyweight must be bound to
 * 	the underlying vector to return the original value. It is default-constructed to a special invalid value. The validity
 * 	can be checked with valid().
 *
 * 	The BoundFlyweight holds the index plus a reference to the factory that permits const Value& access.
 *
 */

template<typename Value,typename Index=std::size_t>class IndexedFlyweightFactory
{
public:
	IndexedFlyweightFactory();
	IndexedFlyweightFactory(const IndexedFlyweightFactory&);
	IndexedFlyweightFactory(IndexedFlyweightFactory&&);

	IndexedFlyweightFactory& operator=(IndexedFlyweightFactory&&)=default;

	class Flyweight;
	class BoundFlyweight;

	/// Create a flyweight object from a given value, adding to the table if necessary (always returns valid)
	Flyweight		insert(const Value& v);

	/// Return a flyweight if the value exists, otherwise an invalid value
	Flyweight		find(const Value& v) const;

	/// Bind a flyweight to this table, and return the bound object so it can be used as the underlying value type
	BoundFlyweight bind(Flyweight f) const;

	/// Convenience function to bind a (key,value) that reference the same flyweight factory
	std::pair<BoundFlyweight,BoundFlyweight> bind(std::pair<Flyweight,Flyweight> p) const
		{ return std::make_pair(bind(p.first),bind(p.second)); }

    void clear()
    {
        m_indexMap.clear();
        m_values.clear();
    }

private:
	typedef std::unordered_map<Value,Index>		IndexMap;

	// mapping of values to indices
	IndexMap									m_indexMap;

	// values stored by index
	std::vector<typename IndexMap::const_iterator>	m_values;
	void rebuildVector();

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned)
		{
			ar & m_indexMap;
			if(Archive::is_loading::value)
				rebuildVector();
		}
};

template<typename Value,typename Index>IndexedFlyweightFactory<Value,Index>::IndexedFlyweightFactory()
		{}

template<typename Value,typename Index>IndexedFlyweightFactory<Value,Index>::IndexedFlyweightFactory(IndexedFlyweightFactory&& f) :
		m_indexMap(std::move(f.m_indexMap)),
		m_values(std::move(f.m_values))
		{}

template<typename Value,typename Index>IndexedFlyweightFactory<Value,Index>::IndexedFlyweightFactory(const IndexedFlyweightFactory& f) :
		m_indexMap(std::move(f.m_indexMap))
		{
	rebuildVector();
		}



template<typename Value,typename Index>class IndexedFlyweightFactory<Value,Index>::Flyweight
{
public:
	Flyweight(){}

	/** Equality means equal flyweight index, which means they refer to the same value entry
	 * NOTE: invalid != invalid
	 * This is to avoid a situation where you do find("invalid-X") == find("invalid-Y") and falsely conclude
	 * that "invalid-X" == "invalid-Y"
	 */
	bool operator==(const Flyweight& rhs) const
		{ return m_index==rhs.m_index && m_index != Invalid; }

	/// comparison object to place flyweights in index order (arbitrary order wrt. contents)
	struct IndexOrder
	{
		bool operator()(const Flyweight lhs,const Flyweight rhs) const
			{ return lhs.m_index < rhs.m_index; }
	};

	bool 	valid() 		const { return m_index != Invalid; }
	Index	index() 		const { return m_index; }

protected:
	explicit constexpr Flyweight(Index i) :
		m_index(i){}

private:
	static constexpr Index Invalid = std::numeric_limits<Index>::max();
	Index			m_index=Invalid;

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned)
		{ ar & m_index; }

	friend class IndexedFlyweightFactory<Value,Index>;
};

template<typename Value,typename Index>class IndexedFlyweightFactory<Value,Index>::BoundFlyweight : public IndexedFlyweightFactory<Value,Index>::Flyweight
{
public:
	operator 	const Value&() 		const;
	Flyweight			unbind() 	const
		{ return Flyweight(*this); };

	BoundFlyweight(){}
	BoundFlyweight(const IndexedFlyweightFactory<Value,Index>* f,Flyweight i) :
		Flyweight(i),
		m_factory(f){}

private:
	const IndexedFlyweightFactory<Value,Index>*	m_factory=nullptr;
};

template<typename Value,typename Index>typename IndexedFlyweightFactory<Value,Index>::Flyweight IndexedFlyweightFactory<Value,Index>::insert(const Value& v)
{
	typename IndexMap::iterator it;
	bool inserted;

	std::tie(it,inserted) = m_indexMap.emplace(v,m_indexMap.size());

	if (inserted)
		m_values.emplace_back(it);

	return Flyweight(it->second);
}

template<typename Value,typename Index>typename IndexedFlyweightFactory<Value,Index>::Flyweight IndexedFlyweightFactory<Value,Index>::find(const Value& v) const
{
	typename IndexMap::const_iterator it = m_indexMap.find(v);
	return it == m_indexMap.end() ? Flyweight(): Flyweight(it->second);
}

template<typename Value,typename Index>typename IndexedFlyweightFactory<Value,Index>::BoundFlyweight IndexedFlyweightFactory<Value,Index>::bind(typename IndexedFlyweightFactory<Value,Index>::Flyweight flyweight) const
{
	if (flyweight.valid())
		return BoundFlyweight(this,flyweight);
	else
		throw std::out_of_range("IndexedFlyweightFactory::bind() called with invalid flyweight");
}

template<typename Value,typename Index>IndexedFlyweightFactory<Value,Index>::BoundFlyweight::operator const Value&() const
{
	return m_factory->m_values[this->index()]->first;
}

template<typename Value,typename Index>void IndexedFlyweightFactory<Value,Index>::rebuildVector()
		{
	m_values.resize(m_indexMap.size(),m_indexMap.end());
	for(auto it = m_indexMap.cbegin(); it != m_indexMap.cend(); ++it)
		m_values[it->second] = it;
		}



/** Provide serialization for unordered_map.
 *
 * Unfortunately the ECE labs only have Boost 1.55.0.
 * Later versions actually provide this as part of the library.
 */

#ifdef USE_BOOST_UNORDERED_MAP_SERIALIZE

// not available in Boost 1.55.0
#include <boost/serialization/unordered_map.hpp>

#else

#include <boost/serialization/split_free.hpp>

namespace boost { namespace serialization {

template<class Archive,typename Key,typename T,class Hash,class Pred,class Alloc>
	inline void serialize(Archive& ar,std::unordered_map<Key,T,Hash,Pred,Alloc>& H,const unsigned file_version)
{
	split_free(ar,H,file_version);
}

template<class Archive,typename Key,typename T,class Hash,class Pred,class Alloc>
	void save(Archive& ar,const std::unordered_map<Key,T,Hash,Pred,Alloc>& H,const unsigned /*file_version*/ )
{
	std::vector<std::pair<Key,T>> v(H.size());
	boost::copy(H,v.begin());
	ar & v;
}

template<class Archive,typename Key,typename T,class Hash,class Pred,class Alloc>
	void load(Archive& ar,std::unordered_map<Key,T,Hash,Pred,Alloc>& H,const unsigned /*file_version*/ )
{
	std::vector<std::pair<Key,T>> v;
	ar & v;
	H = std::unordered_map<Key,T,Hash,Pred,Alloc>(v.begin(),v.end(),v.size(),H.hash_function(),H.key_eq(),H.get_allocator());

}

}}

#endif


#endif /* STREETSDATABASE_SRC_INDEXEDFLYWEIGHT_HPP_ */
