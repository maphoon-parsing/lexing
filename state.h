
// Written by Dina Muktubayeva, March/April 2021.

#ifndef MAPH_LEXING_STATE
#define MAPH_LEXING_STATE  1

#include <iostream>

// states are represented by unsigned int.
// In an acceptor, states are represented by
// (signed) integers, and relative to the
// present state. Because of that, we have operators
// + which adds an integer to a state
// - which subtracts an integer from a state
// and - which substracts to states and returns 
// a state. 

namespace lexing
{

   struct state
   {
      unsigned int nr;

      state(unsigned int nr)
         : nr(nr)
      { }

      state operator + ( int i ) const
      {
         return nr + i;
      }

      state& operator ++ () 
      {
         nr ++;
         return *this;
      }

      state& operator -- ()
      {
         nr --;
         return *this;
      }

      state operator ++ ( int ) 
      {
         state old = *this;
         nr++ ;
         return old;
      }

      state operator -- ( int ) 
      {
         state old = *this;
         nr --;
         return old;
      }

      state& operator += ( int i )
      {
         nr += i; return *this;
      }

      int operator - ( state s ) const
      {
         return (int)nr - (int)s. nr;
      }

   };

   inline bool operator < ( state s1, state s2 )
   {
      return s1. nr < s2. nr;
   }

   inline bool operator <= ( state s1, state s2 ) 
   {
      return s1. nr <= s2. nr;
   }

   inline bool operator > ( state s1, state s2 )
   {
      return s1. nr > s2. nr; 
   }

   inline bool operator >= ( state s1, state s2 )
   {
      return s1. nr >= s2. nr;
   }

   inline bool operator == ( state s1, state s2 ) 
   {
      return s1. nr == s2. nr;
   }

   inline bool operator != ( state s1, state s2 ) 
   {
      return s1. nr != s2. nr; 
   }


   inline std::ostream& operator<< ( std::ostream& out, const state& s )
   {
      out << "Q";
      if( s. nr < 10 ) out << '0';
      out << s. nr;
      return out;
   }

}

#endif


