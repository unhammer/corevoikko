/* Copyright (C) 1997 Bjoern Beutel. */

/* Description. =============================================================*/

/* Options for malaga and functions to start and terminate malaga. */

/* Includes. ================================================================*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#include "morphology/malaga/basic.hpp"
#include "morphology/malaga/pools.hpp"
#include "morphology/malaga/values.hpp"
#include "morphology/malaga/rule_type.hpp"
#include "morphology/malaga/rules.hpp"
#include "morphology/malaga/files.hpp"
#include "morphology/malaga/analysis.hpp"
#include "morphology/malaga/symbols.hpp"
#include "morphology/malaga/lexicon.hpp"
#include "morphology/malaga/patterns.hpp"
#include "morphology/malaga/malaga_lib.hpp"

namespace libvoikko { namespace morphology { namespace malaga {

/* Variables. ===============================================================*/

static const char * const project_file = "voikko-fi_FI.pro";
static const char * const morphology_file = "voikko-fi_FI.mor";
static const char * const lexicon_file = "voikko-fi_FI.lex";
static const char * const symbol_file = "voikko-fi_FI.sym";

/* Functions. ===============================================================*/

static const char * const pathSeparator() {
#ifdef WIN32
  return "\\";
#else
  return "/";
#endif
}

/*---------------------------------------------------------------------------*/

static const char * const
binarySuffix() {
  union { char_t chars[4]; int_t integer; } format;

  format.integer = 0x12345678;
  if (sizeof( int_t ) != 4) {
    return "_c";
  }
  else if (format.chars[0] == 0x12 && format.chars[1] == 0x34
	   && format.chars[2] == 0x56 && format.chars[3] == 0x78) {
    return "_b";
  }
  else if (format.chars[0] == 0x78 && format.chars[1] == 0x56
	   && format.chars[2] == 0x34 && format.chars[3] == 0x12) {
     return "_l";
  }
  else {
    return "_c";
  }
}

/*---------------------------------------------------------------------------*/

void 
init_malaga(string_t directoryName)
/* Initialise this module. */
{ 
  /* Init modules. */
  char * fullSymbolFile = concat_strings(directoryName, pathSeparator(), symbol_file, binarySuffix(), NULL);
  char * fullLexiconFile = concat_strings(directoryName, pathSeparator(), lexicon_file, binarySuffix(), NULL);
  char * fullMorphologyFile = concat_strings(directoryName, pathSeparator(), morphology_file, binarySuffix(), NULL);
  
  init_values();
  init_symbols(fullSymbolFile);
  init_lexicon(fullLexiconFile);
  init_analysis(fullMorphologyFile);
  
  free(fullSymbolFile);
  free(fullLexiconFile);
  free(fullMorphologyFile);
}

/*---------------------------------------------------------------------------*/

void 
terminate_malaga( void )
/* Terminate this module. */
{
  terminate_analysis();
  terminate_patterns();
  terminate_lexicon();
  terminate_symbols();
  terminate_values();
}

}}}
