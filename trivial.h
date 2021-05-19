
// Written by Dina Muktubayeva and
// Hans de Nivelle, May 2021.

// This makes it possible to replace an automaton
// by something trivial. 

#ifndef MAPH_LEXING_TRIVIAL
#define MAPH_LEXING_TRIVIAL  1

#include <climits>
#include <iostream>
#include <set>
#include <stack>
#include "flatmap.h"
#include "state.h"

namespace lexing
{
   struct trivial 
   { 
      constexpr trivial( ) = default;
   };

   inline std::ostream& operator << ( std::ostream& out, trivial tr )
   {
      out << "(trivial)";
      return out;
   }
}

#endif

