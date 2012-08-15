#pragma once
#include "../Path.h"
#include <boost/exception/info.hpp>

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	struct CExceptionBase: virtual std::exception, virtual boost::exception { };
	struct CRenamingException: virtual CExceptionBase { };
	struct CInvalidFilteredPathPart: virtual CRenamingException { };

	typedef boost::error_info<struct tag_CFilteredPart,unsigned> CFilteredPart;

	/**
	 * A file name that is renamed by a rename-it! filter.
	 * So the part being renamed can be accessed.
	 *
	 * It expects the path to be to a file when the filtered path includes
	 * the file name or extension, and it expects path to be to a folder
	 * when the filtered path doesn't include the file name or extension.
	 *
	 * Unicode paths are temporarily transformed to simple path to
	 * avoid showing the "\\?\" part in the filtered sub-string.
	 *
	 * Usage example:
	 * \code
	 * CFilteredPath filteredPath(_T("\\\\?\\C:\\foo\\bar"),
	 *     CFilteredPath::renameRoot | CFilteredPath::renameFoldersPath);
	 * cout << filteredPath.GetFilteredSubstring() << endl; // Displays: "C:\foo".
	 * cout << filteredPath.GetFilteredPath() << endl; // Displays "\\?\C:\foo\bar".
	 *
	 * filteredPath.SetFilteredSubstring(_T("X:\\xx"));
	 * cout << filteredPath.GetFilteredSubstring() << endl; // Displays: "X:\xx".
	 * cout << filteredPath.GetFilteredPath() << endl; // Displays "\\?\X:\xx\bar".
	 * \endcode
	 */
	class CFilteredPath
	{
	public:
	// Definitions
		/**
		 * Renamed part (must be continuous: (0x01 | 0x04) is not allowed as there is 0x02 in the middle missing).
		 * WARNING: renameLastFolder can only be used alone.
		 * The renameRoot shouldn't be used with folders without the file name, because folders's root cannot be changed.
		 */
		enum ERenamePartFlags {
			renameRoot =		0x01,	// 'C:\'
			renameFoldersPath =	0x02,	// 'ParentFolder\Subfolder\'
			renameLastFolder =	0x04,	// 'Subfolder' -- can only be used alone
			renameFilename =	0x08,	// 'Filename'
			renameExtension =	0x10,	// 'Ext'
			renameVersion = 100			// Some value that should be incremented every time there is an important change in the flags.
		};

	// Construction
		/**
		 * Create a filtered path from a path name.
		 * \param strPath Path to filter.
		 * \param nRenamedPart A set of ERenamePartFlags bit flags.
		 */
		CFilteredPath(const CString& strPath, unsigned nRenamedPart)
		{
			Construct(strPath, nRenamedPart);
		}

		CFilteredPath(const CPath& path, unsigned nRenamedPart)
		{
			Construct(path.GetPath(), nRenamedPart);
		}

	private:
		void Construct(const CString& strPath, unsigned nRenamedPart)
		{
			m_bIsUncPath = CPath::IsUnicodePath(strPath);

			CPathFilter pathFilter(CPath::MakeSimplePath(strPath), nRenamedPart);
			m_strBefore = pathFilter.GetPartBefore();
			m_strFilteredSubstring = pathFilter.GetPartFiltered();
			m_strAfter = pathFilter.GetPartAfter();
		}
	public:

	// Attributes
		/**
		 * The part being renamed by filters.
		 * @return A string corresponding to the renamed part of the full path.
		 */
		CString GetFilteredSubstring() const 
		{
			return m_strFilteredSubstring;
		}

		/**
		 * Modify the renamed part of the file name.
		 * @strValue A string corresponding to the new renamed part of the full path.
		 */
		void SetFilteredSubstring(const CString& strValue) 
		{
			m_strFilteredSubstring = strValue;
		}

	// Operations
		/**
		 * Return the full path (with the updated filtered substring,
		 * meaning it'll be the renamed full path).
		 */
		CString GetFilteredPath() const
		{
			// Change the renamed part.
			CString strNewPath(m_strBefore + m_strFilteredSubstring + m_strAfter);
			if (m_bIsUncPath)
				return CPath::MakeUnicodePath(strNewPath);
			else
				return strNewPath;
		}

		/**
		 * Convert the new filtered path to a CPath.
		 */
		operator CPath() const
		{
			return CPath(GetFilteredPath());
		}

	// Implementation
	private:
		class CPathFilter : private CPath
		{
		public:
			CPathFilter(const CString& strPath, unsigned filteredPart) : CPath(strPath)
			{
				BOOST_STATIC_ASSERT(renameVersion == 100);

				if (filteredPart == 0
					|| filteredPart == CFilteredPath::renameVersion
					|| ((filteredPart & renameLastFolder) && filteredPart != renameLastFolder))
				{
					BOOST_THROW_EXCEPTION(CInvalidFilteredPathPart() << CFilteredPart(filteredPart));
				}

				// Note: It may be awkward to select the file name when renaming the folders,
				//       but that's because when renaming folders, the file name IS the last
				//       folder's name.

				// Find the first character of the filtered substring.
				if (filteredPart & renameRoot)
					m_nBegin = 0;
				else if (filteredPart & renameFoldersPath)
					m_nBegin = m_nPathRootLength;
				else if (filteredPart & (renameLastFolder | renameFilename))
					m_nBegin = m_nFileNameFirst;
				else
				{
					// Only the extension is being renamed.
					ASSERT(filteredPart == renameExtension);
					m_nBegin = m_strPath.GetLength() - m_nExtensionLength;
				}

				// Find the last character of the filtered substring.
				if (filteredPart & (renameLastFolder | renameExtension))
					m_nEnd = m_strPath.GetLength();
				else if (filteredPart & renameFilename)
					m_nEnd = m_strPath.GetLength() - m_nExtensionLength;
				else if (filteredPart & renameFoldersPath)
					m_nEnd = m_strPath.GetLength();
				else
				{
					ASSERT(filteredPart == renameRoot);
					m_nEnd = m_nPathRootLength;
				}

				ASSERT(0 <= m_nBegin && m_nBegin <= m_strPath.GetLength());
				ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());
			}

			CString GetPartBefore() const {
				return m_strPath.Left(m_nBegin);
			}

			CString GetPartFiltered() const {
				return m_strPath.Mid(m_nBegin, m_nEnd - m_nBegin);
			}

			CString GetPartAfter() const {
				return m_strPath.Mid(m_nEnd);
			}

		private:
			int m_nBegin;	// First character of the filtered substring.
			int m_nEnd;		// The character right after last character of the filtered substring.
		};

		bool m_bIsUncPath;	// True when the original and final path should be UNC.
		CString m_strBefore;
		CString m_strFilteredSubstring;
		CString m_strAfter;
	};
}}}}
