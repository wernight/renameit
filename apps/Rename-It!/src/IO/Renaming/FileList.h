#pragma once
#include "Path.h"

namespace Beroux{ namespace IO{ namespace Renaming
{ 
	/**
	 * Contains a list of file names.
	 */
	class CFileList
	{
	// Attributes
	public:
		int GetCount() const {
			return (int) m_vfnFiles.size();
		}

		const CPath& GetPath(int nIndex) const {
			return m_vfnFiles.at(nIndex);
		}

	// Basic list operations
		/**
		 * Add a file to the end of the files list.
		 * @param[in] fnFileName	The file name of the file to be renamed.
		 * @return The index of the added file.
		 */
		int AddPath(const CPath& fnFileName)
		{
			m_vfnFiles.push_back(fnFileName);
			return (int) m_vfnFiles.size() - 1;
		}

		/**
		 * Overloaded method that constructs the CPath from the path.
		 * @param[in] strFileName	Path to the file or folder to be renamed.
		 * @return The index of the added file.
		 */
		inline int AddPath(const CString& strFileName) {
			return AddPath(CPath(strFileName));
		}

		/**
		 * Insert a file at a specific position in the list.
		 * @param[in] fnFileName	The file name of the file to be renamed.
		 * @param[in] nIndex		Index position where the file should be insert (0 means first element of the list).
		 */
		void InsertPath(int nIndex, const CPath& fnFileName)
		{
			if (nIndex == (int) m_vfnFiles.size())
				AddPath(fnFileName);
			else if (nIndex >= (int) m_vfnFiles.size())
				throw out_of_range("Index is out of range.");
			else
				m_vfnFiles.insert(m_vfnFiles.begin() + nIndex, fnFileName);
		}

		/**
		 * Overloaded method that constructs the CPath from the path.
		 * @param[in] strFileName	Path to the file or folder to be renamed.
		 * @param[in] nIndex		See InsertPath above.
		 */
		inline void InsertPath(int nIndex, const CString& strFileName) {
			return InsertPath(nIndex, CPath(strFileName));
		}

		/**
		 * Remove a file from the list of files to be renamed.
		 * @param[in] nIndex		Index of the file to remove.
		 */
		void RemoveFile(int nIndex) {
			if (nIndex < 0 || nIndex >= (int) m_vfnFiles.size())
				throw out_of_range("Index is out of range.");
			else
				m_vfnFiles.erase(m_vfnFiles.begin() + nIndex);
		}

		const CPath& operator[](int nIndex) const {
			return m_vfnFiles.at(nIndex);
		}

	// Implementation
	private:
		vector<CPath> m_vfnFiles;
	};
}}}
