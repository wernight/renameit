#pragma once

#include "Path.h"

/**
 * Real content of the displayed virtual list of elements to rename.
 */
class CMemoryFileList
{
// Definitions
private:
	class CInputIterator;
	class COutputIterator;

public:
	struct ITEM
	{
		bool bChecked;
		CPath fnBefore;
		CPath fnAfter;
	};

	typedef list<ITEM>::iterator iterator;
	typedef list<ITEM>::const_iterator const_iterator;

	typedef CInputIterator InputIterator;
	typedef COutputIterator OutputIterator;

// Construction

// Attributes
	inline bool IsEmpty() const {
		return m_filesNames.empty();
	}

	inline int GetFileCount() const {
		return (int) m_filesNames.size();
	}

	inline iterator GetHead() {
		return m_filesNames.begin();
	}

	inline const_iterator GetHead() const {
		return m_filesNames.begin();
	}

	inline iterator GetTail() {
		return m_filesNames.end();
	}

	inline const_iterator GetTail() const {
		return m_filesNames.end();
	}

	// BOOST_FOREACH
	inline iterator begin() { return m_filesNames.begin(); }
	inline const_iterator begin() const { return m_filesNames.begin(); }
	inline iterator end() { return m_filesNames.end(); }
	inline const_iterator end() const { return m_filesNames.end(); }

// Operations
	iterator GetIteratorAt(int nIndex)
	{
		iterator iter = m_filesNames.begin();
		for (; nIndex > 0; --nIndex)
			++iter;
		return iter;
	}

	const_iterator GetIteratorAt(int nIndex) const
	{
		const_iterator iter = m_filesNames.begin();
		for (; nIndex > 0; --nIndex)
			++iter;
		return iter;
	}

	int FindIndexOf(const_iterator iterAt) const
	{
		int nIndex = 0;
		for (const_iterator iter=m_filesNames.begin(); iter!=m_filesNames.end(); ++iter, ++nIndex)
			if (iter == iterAt)
				return nIndex;
		return -1;
	}

	/**
	 * Return an iterator pointing to the first checked file,
	 * or GetTail() if there is none.
	 */
	iterator GetFirstChecked()
	{
		iterator iter = m_filesNames.begin();
		while (iter != m_filesNames.end() && !iter->bChecked)
			++iter;
		return iter;
	}

	const_iterator GetFirstChecked() const
	{
		return GetFirstChecked();	// Use the implicit conversion
	}

	InputIterator GetInputIteratorAt(const_iterator iter) const
	{
		return CInputIterator(iter, m_filesNames.end());
	}

	OutputIterator GetOutputIteratorAt(iterator iter)
	{
		return COutputIterator(iter, m_filesNames.end());
	}

	/**
	 * Search for a file named "fileName".
	 * @param fnFileName The file name to search.
	 * @return An iterator positioned at the found file if it's found,
	 *         or GetTail() if it's not found.
	 */
	const_iterator FindFile(const CPath& fnFileName) const
	{
		const_iterator iter;
		for (iter=m_filesNames.begin(); iter!=m_filesNames.end(); ++iter)
			if (iter->fnBefore == fnFileName)
				break;
		return iter;
	}

	void AddFile(CPath fnFileName, bool bChecked=true)
	{
		ITEM item = {bChecked, fnFileName, fnFileName};
		m_filesNames.push_back(item);
	}

	void RemoveAt(iterator at)
	{
		m_filesNames.erase(at);
	}

	void RemoveAll()
	{
		m_filesNames.clear();
	}

	/**
	 * Move internally a group of files [first, last) up or down.
	 * @param dest The destination before which the group [first, last) will be moved to.
	 * @param first The first element to move.
	 * @param last The last element to move.
	 */
	void ReorderFiles(iterator dest, iterator first, iterator last)
	{
		m_filesNames.splice(dest, m_filesNames, first, last);
	}

// Implementation
private:
	/**
	 * A class conforming to the concept of Input iterator that iterates throught all checked fnBefore.
	 * @warning The last element of a foreach or similar must be a checked item.
	 */
	class CInputIterator
	{
	public:
		CInputIterator()
		{
		}

		explicit CInputIterator(const_iterator iter, const_iterator end)
		{
			ASSERT(iter == end || iter->bChecked);	// The item must be checked (or be the last item of the container).
			m_iter = iter;
			m_end = end;
		}

		// Assignable by default.

		// Equality Comparable (not so easy what that should mean here)
		bool operator==( const CInputIterator& value) const { return m_iter == value.m_iter; }
		bool operator!=( const CInputIterator& value) const { return m_iter != value.m_iter; }

		// Trivial Iterator:
		const CPath& operator* () const { return m_iter->fnBefore; }
		const CPath* operator->() const { return & operator*(); }

		// Input Iterator
		CInputIterator& operator++()
		{
			// preincrement
			do
				++m_iter;
			while (m_iter != m_end && !m_iter->bChecked);
			return *this;
		}

		CInputIterator  operator++(int)
		{
			// postincrement
			CInputIterator tmp = *this;
			++*this;
			return tmp;
		}

	private:
		const_iterator m_iter;
		const_iterator m_end;
	};

	/**
	 * A class conforming to the concept of Output Iterator that iterates throught all checked fnAfter.
	 * @warning The last element of a foreach or similar must be a checked item.
	 */
	class COutputIterator
	{
	public:
		COutputIterator()
		{
		}

		explicit COutputIterator(iterator iter, iterator end)
		{
			ASSERT(iter == end || iter->bChecked);	// The item must be checked (or be the last item of the container).
			m_iter = iter;
			m_end = end;
		}

		// Output Iterator
		CPath& operator* () const { return m_iter->fnAfter; }
		CPath* operator->() const { return & operator*(); }

		// Input Iterator
		COutputIterator& operator++()
		{
			// preincrement
			do
				++m_iter;
			while (m_iter != m_end && !m_iter->bChecked);
			return *this;
		}

		COutputIterator  operator++(int)
		{
			// postincrement
			COutputIterator tmp = *this;
			++*this;
			return tmp;
		}

	private:
		iterator m_iter;
		iterator m_end;
	};

	list<ITEM> m_filesNames;
};