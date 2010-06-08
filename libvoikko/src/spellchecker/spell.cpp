/* Libvoikko: Finnish spellchecker and hyphenator library
 * Copyright (C) 2006 - 2009 Harri Pitkänen <hatapitk@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *********************************************************************************/

#include "utils/utils.hpp"
#include "utils/StringUtils.hpp"
#include "character/charset.hpp"
#include "character/SimpleChar.hpp"
#include "spellchecker/spell.hpp"
#include "spellchecker/SpellUtils.hpp"
#include "porting.h"
#include <cstdlib>
#include <cstring>
#include <cwchar>

using namespace libvoikko::morphology;
using namespace libvoikko::spellchecker;
using namespace libvoikko::character;
using namespace std;

namespace libvoikko {

spellresult voikko_do_spell(voikko_options_t * voikkoOptions,
	                   const wchar_t * word, size_t len) {
	spellresult result_with_border = SPELL_FAILED;
	spellresult result_without_border = SPELL_FAILED;
	
	spellresult result = voikkoOptions->speller->spell(word, len);
	const wchar_t * hyphen_pos;
	if (result != SPELL_OK && len > 3) {
		hyphen_pos = wmemchr(word + 1, L'-', len - 2);
	}
	else {
		hyphen_pos = 0;
	}
	
	if (hyphen_pos) { /* Check optional hyphens */
		size_t leading_len = hyphen_pos - word;
		wchar_t * buffer = new wchar_t[len];
		wcsncpy(buffer, word, leading_len);
		wcsncpy(buffer + leading_len, hyphen_pos + 1, len - leading_len - 1);
		buffer[len - 1] = L'\0';
		
		if (voikkoOptions->accept_extra_hyphens && leading_len > 1 &&
		    buffer[leading_len] != L'-') {
			/* All hyphens are optional */
			/* FIXME: deep recursion */
			spellresult spres = voikko_do_spell(voikkoOptions, buffer, len - 1);
			if (spres == SPELL_OK) {
				delete[] buffer;
				return spres;
			}
		}
		
		/* Leading part ends with the same VC pair as the trailing part starts ('pop-opisto') */
		if (leading_len >= 2 && len - leading_len >= 3) {
			wchar_t vctest1 = SimpleChar::lower(word[leading_len - 2]);
			wchar_t vctest2 = SimpleChar::lower(word[leading_len - 1]);
			if (wcschr(VOIKKO_VOWELS, vctest1) &&
			    wcschr(VOIKKO_CONSONANTS, vctest2) &&
			    SimpleChar::lower(word[leading_len + 1]) == vctest1 &&
			    SimpleChar::lower(word[leading_len + 2]) == vctest2) {
				spellresult spres = voikkoOptions->speller->spell(buffer, len - 1);
				if (spres != SPELL_FAILED && (result == SPELL_FAILED || result > spres)) {
					delete[] buffer;
					return spres;
				}
			}
		}
		
		/* Ambiguous compound ('syy-silta', 'syys-ilta') */
		list<Analysis *> * analyses = voikkoOptions->morAnalyzer->analyze(buffer);
		
		if (analyses->empty()) {
			Analyzer::deleteAnalyses(analyses);
			delete[] buffer;
			return result;
		}
		
		list<Analysis *>::const_iterator it = analyses->begin();
		while (it != analyses->end()) {
			const wchar_t * structure = (*it)->getValue("STRUCTURE");
			size_t j = 0;
			size_t i;
			for (i = 0; i < leading_len; i++) {
				while (structure[j] == L'=') {
					j++;
				}
				if (structure[j] == L'\0') {
					break;
				}
				j++;
			}
			if (i == leading_len) {
				spellresult spres = SpellUtils::matchWordAndAnalysis(buffer, len - 1, structure);
				if (structure[j] == L'=' && (result_with_border == SPELL_FAILED ||
				    result_with_border > spres)) {
					result_with_border = spres;
				}
				if (structure[j] != L'=' && (result_without_border == SPELL_FAILED ||
				    result_without_border > spres)) {
					result_without_border = spres;
				}
			}
			it++;
		}
		
		Analyzer::deleteAnalyses(analyses);
		delete[] buffer;
		if (result_with_border != SPELL_FAILED && result_without_border != SPELL_FAILED &&
		    (result == SPELL_FAILED || result > result_with_border)) {
			return result_with_border;
		}
	}
	
	return result;
}

/**
 * Checks the spelling of given word. Missing hyphens at the start or end of the
 * word are ignored if requested in voikkoOptions.
 * @param word word to check (does not need to be null terminated)
 * @param len length of the word to check
 * @return spelling result
 */
static spellresult hyphenAwareSpell(voikko_options_t * voikkoOptions,
	                                   const wchar_t * word, size_t len) {
	spellresult spres = voikko_do_spell(voikkoOptions, word, len);
	if (spres != SPELL_FAILED || !voikkoOptions->accept_missing_hyphens) {
		return spres;
	}
	
	// Hyphens were already present, so we cannot do anything more
	if (len < 2 || (word[0] == L'-' && word[len - 1] == L'-')) return SPELL_FAILED;
	
	wchar_t * buffer = new wchar_t[len + 2];
	size_t newlen = len + 1;
	if (word[0] == L'-') {
		wcsncpy(buffer, word, len);
		buffer[len] = L'-';
	}
	else {
		buffer[0] = L'-';
		wcsncpy(buffer + 1, word, len);
		if (word[len - 1] != L'-') {
			buffer[len + 1] = L'-';
			newlen++;
		}
	}
	spres = voikko_do_spell(voikkoOptions, buffer, newlen);
	delete[] buffer;
	return spres;
}


/** Checks the spelling of given word and uses cache if possible
 * @param word word to check. Word does not need to be null terminated and it must
 *             not contain upper case letters
 * @param len length of the word to check
 * @return spelling result
 */
static spellresult voikko_cached_spell(voikko_options_t * voikkoOptions, const wchar_t * buffer, size_t len) {
	SpellerCache * cache = voikkoOptions->spellerCache;
	if (cache) {
		if (cache->isInCache(buffer, len)) {
			/* is in cache */
			return cache->getSpellResult(buffer, len);
		}
		/* not in cache */
		spellresult result = hyphenAwareSpell(voikkoOptions, buffer, len);
		cache->setSpellResult(buffer, len, result);
		return result;
	}
	/* no cache available */
	return hyphenAwareSpell(voikkoOptions, buffer, len);
}


VOIKKOEXPORT int voikkoSpellUcs4(voikko_options_t * voikkoOptions, const wchar_t * word) {
	size_t nchars = wcslen(word);
	int result;
	spellresult sres;
	if (nchars == 0) {
		return VOIKKO_SPELL_OK;
	}
	if (nchars > LIBVOIKKO_MAX_WORD_CHARS) {
		return VOIKKO_INTERNAL_ERROR;
	}
	
	wchar_t * nword = voikko_normalise(word, nchars);
	if (nword == 0) {
		return VOIKKO_INTERNAL_ERROR;
	}
	nchars = wcslen(nword);
	
	if (voikkoOptions->ignore_numbers) {
		for (size_t i = 0; i < nchars; i++) {
			if (SimpleChar::isDigit(nword[i])) {
				delete[] nword;
				return VOIKKO_SPELL_OK;
			}
		}
	}
	casetype caps = voikko_casetype(nword, nchars);
	if ((voikkoOptions->ignore_uppercase && caps == CT_ALL_UPPER) ||
	    (voikkoOptions->ignore_nonwords && voikko_is_nonword(nword, nchars))) {
		delete[] nword;
		return VOIKKO_SPELL_OK;
	}
	if (caps == CT_ALL_UPPER && !voikkoOptions->accept_all_uppercase) {
		// all upper case is nothing special
		caps = CT_COMPLEX;
	}
	
	wchar_t * buffer = new wchar_t[nchars + 1];

	for (size_t i = 0; i < nchars; i++) {
		buffer[i] = SimpleChar::lower(nword[i]);
	}
	buffer[nchars] = L'\0';
	
	int dot_index;
	size_t realChars;
	if (voikkoOptions->ignore_dot && buffer[nchars - 1] == L'.') {
		dot_index = static_cast<int>(nchars - 1);
		buffer[dot_index] = L'\0';
		realChars = nchars - 1;
	}
	else {
		dot_index = -1;
		realChars = nchars;
	}
	
	/* Check words that require exact captialisation */
	if (caps == CT_COMPLEX || caps == CT_NO_LETTERS) {
		wcsncpy(buffer, nword, nchars);
		buffer[0] = SimpleChar::lower(buffer[0]);
		sres = hyphenAwareSpell(voikkoOptions, buffer, nchars);
		if (sres == SPELL_OK ||
		    (sres == SPELL_CAP_FIRST && voikkoOptions->accept_first_uppercase && SimpleChar::isUpper(nword[0]))) {
			result = VOIKKO_SPELL_OK;
		}
		else {
			result = VOIKKO_SPELL_FAILED;
		}
		if (result == VOIKKO_SPELL_FAILED && dot_index != -1) { /* remove dot */
			buffer[dot_index] = L'\0';
			sres = hyphenAwareSpell(voikkoOptions, buffer, realChars);
			if (sres == SPELL_OK ||
			    (sres == SPELL_CAP_FIRST && voikkoOptions->accept_first_uppercase && SimpleChar::isUpper(nword[0]))) {
				result = VOIKKO_SPELL_OK;
			}
		}
		delete[] nword;
		delete[] buffer;
		return result;
	}
	
	
	/* Check without trailing dot */
	sres = voikko_cached_spell(voikkoOptions, buffer, realChars);
	switch (caps) {
		case CT_ALL_LOWER:
			result = (sres == SPELL_OK) ? VOIKKO_SPELL_OK : VOIKKO_SPELL_FAILED;
			break;
		case CT_FIRST_UPPER:
			if ((sres == SPELL_OK && voikkoOptions->accept_first_uppercase) || sres == SPELL_CAP_FIRST)
				result = VOIKKO_SPELL_OK;
			else result = VOIKKO_SPELL_FAILED;
			break;
		case CT_ALL_UPPER:
			result = (sres == SPELL_FAILED) ? VOIKKO_SPELL_FAILED : VOIKKO_SPELL_OK;
			break;
		default: /* should not happen */
			result = VOIKKO_INTERNAL_ERROR;
	}
	if (result == VOIKKO_SPELL_OK) {
		delete[] nword;
		delete[] buffer;
		return VOIKKO_SPELL_OK;
	}
	
	/* Check with trailing dot */
	if (dot_index != -1) {
		buffer[dot_index] = L'.';
		switch (caps) {
			case CT_ALL_LOWER:
				sres = voikko_cached_spell(voikkoOptions, buffer, nchars);
				result = (sres == SPELL_OK) ? VOIKKO_SPELL_OK : VOIKKO_SPELL_FAILED;
				break;
			case CT_FIRST_UPPER:
				sres = voikko_cached_spell(voikkoOptions, buffer, nchars);
				if ((sres == SPELL_OK && voikkoOptions->accept_first_uppercase) || sres == SPELL_CAP_FIRST)
					result = VOIKKO_SPELL_OK;
				else result = VOIKKO_SPELL_FAILED;
				break;
			case CT_ALL_UPPER:
				sres = voikko_cached_spell(voikkoOptions, buffer, nchars);
				result = (sres == SPELL_FAILED) ? VOIKKO_SPELL_FAILED : VOIKKO_SPELL_OK;
			default: /* should not happen */
				result = VOIKKO_INTERNAL_ERROR;
		}
	}
	delete[] nword;
	delete[] buffer;
	return result;
}

VOIKKOEXPORT int voikkoSpellCstr(voikko_options_t * handle, const char * word) {
	if (word == 0 || word[0] == '\0') {
		return VOIKKO_SPELL_OK;
	}
	size_t len = strlen(word);
	if (len > LIBVOIKKO_MAX_WORD_CHARS) {
		return 0;
	}
	wchar_t * word_ucs4 = utils::StringUtils::ucs4FromUtf8(word, len);
	if (word_ucs4 == 0) {
		return VOIKKO_CHARSET_CONVERSION_FAILED;
	}
	int result = voikkoSpellUcs4(handle, word_ucs4);
	delete[] word_ucs4;
	return result;
}

}
