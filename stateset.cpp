
// Written by Dina Muktubayeva, April 2021.

#include "stateset.h"

// This hash function need not be commutative, because
// state sets are ordered.

size_t lexing::stateset::hash::operator( )( const stateset& set ) const 
{
   size_t h = 0;
   for( auto s : set )
   {
      h ^= h << 13;
      h ^= h >> 7;
      h ^= h << 17;
      h ^= s. nr;
   }
   return h;
}


bool lexing::stateset::equal_to::operator( )(
         const stateset& s1, const stateset& s2 ) const
{
   if( s1.size() != s2.size() )
      return false;

   auto it1 = s1.begin();
   auto it2 = s2.begin();

   while( it1 != s1. end( ))
   {
      if( *it1 != *it2 )
         return false;

      ++ it1;
      ++ it2;
   }

   return true;
}


void lexing::stateset::print( std::ostream& out ) const 
{
   out << "{";
   for( auto st = states.begin(); st != states.end(); ++ st )
   {
      if( st != states.begin())
         out << ", ";
      else
         out << " ";

      out << *st;
   }
   out << " }";
}


