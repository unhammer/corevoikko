/* The contents of this file are subject to the Mozilla Public License Version 
 * 1.1 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at 
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is Libvoikko: Library of natural language processing tools.
 * The Initial Developer of the Original Code is Harri Pitkänen <hatapitk@iki.fi>.
 * Portions created by the Initial Developer are Copyright (C) 2009 - 2012
 * the Initial Developer. All Rights Reserved.
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *********************************************************************************/

#include "grammar/GrammarCheckerFactory.hpp"
#include "grammar/NullGrammarChecker.hpp"
#include "grammar/FinnishGrammarChecker.hpp"
#include "porting.h"

#include <iostream>


#ifdef HAVE_VISLCG3
#include "grammar/CgGrammarChecker.hpp"
#endif

using namespace std;

namespace libvoikko { namespace grammar {

GrammarChecker * GrammarCheckerFactory::getGrammarChecker(voikko_options_t * voikkoOptions, const setup::Dictionary & dictionary)
	                              throw(setup::DictionaryException) {

	cerr << "morPath: " << dictionary.getMorPath() << endl; 
	cerr << "gramMorPath: " << dictionary.getGramMorPath() << endl; 
	cerr << "grammarPath: " << dictionary.getGrammarPath() << endl; 
	cerr << "morBackend: " << dictionary.getMorBackend() << endl; 
	cerr << "gramMorBackend: " << dictionary.getGramMorBackend() << endl; 
	cerr << "grammarBackend: " << dictionary.getGrammarBackend() << endl; 

	if (dictionary.getGrammarBackend() == "null") {
		return new NullGrammarChecker();
	}
	if (dictionary.getGrammarBackend() == "finnish") {
		cerr << "GrammarCheckerFactory::getGrammarChecker: " <<endl; 
		cerr << "   " << dictionary.getMorPath() << endl;	
		return new FinnishGrammarChecker(voikkoOptions, dictionary.getMorPath());
	}
	#ifdef HAVE_VISLCG3
	if (dictionary.getGrammarBackend() == "vislcg3") {
		cerr << "GrammarCheckerFactory::getGrammarChecker: " <<endl; 
		cerr << "   " << dictionary.getGramMorPath() << endl;	
		cerr << "   " << dictionary.getGrammarBackend() << endl;	
		return new CgGrammarChecker(dictionary.getGramMorPath(), dictionary.getGrammarPath());
	}
	#endif

	string err = "Failed to create checker because of unknown grammar backend: " + dictionary.getGrammarBackend();
	throw setup::DictionaryException(err.c_str());
}

} }
