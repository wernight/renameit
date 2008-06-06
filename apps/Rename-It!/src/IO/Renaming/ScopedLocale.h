#pragma once

namespace Beroux{ namespace IO{ namespace Renaming
{
	class CScopedLocale
	{
	public:
		CScopedLocale(const wchar_t* newLocale) {
			CString m_strLocaleBak = _tsetlocale(LC_CTYPE, NULL);
			_tsetlocale(LC_CTYPE, newLocale);
		}

		~CScopedLocale() {
			_tsetlocale(LC_CTYPE, m_strLocaleBak);
		}

	protected:
		CString m_strLocaleBak;
	};
}}}
