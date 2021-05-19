
// Written by Dina Muktubayeva, March/April 2021.

#ifndef MAPH_LEXING_STATESET
#define MAPH_LEXING_STATESET  1

#include <iostream>
#include <set>

#include "state.h"

namespace lexing
{

   class stateset 
   {
      std::set< state > states; 

   public:
      stateset( ) = default;  

      struct hash
      {
         size_t operator( ) ( const stateset& set ) const;
      };

      struct equal_to
      {
         bool operator( ) ( const stateset& s1, const stateset& s2 ) const;
      };

      void print( std::ostream& out ) const; 

      bool insert( state s ) { return states. insert(s). second; } 
      bool erase( state s ) { return states. erase(s); }

      bool contains( state s ) const 
         { return states. find(s) != states. end( ); } 

      using const_iterator = std::set< state > :: const_iterator;
      const_iterator begin( ) const { return states. begin( ); }
      const_iterator end( ) const { return states. end( ); } 


      size_t size( ) const { return states. size( ); }  

      void clear( ) { states. clear( ); } 
   };

   
   inline bool subset( const stateset& set1, const stateset& set2 )
   {
      if( set1. size( ) <= set2. size( ))
      {
         for( state s : set1 )
         {
            if( !set2. contains(s))
               return false;
         }
         return true; 
      }
      else
         return false; 
   }

   inline std::ostream& operator<< ( std::ostream& out, const stateset& set ) 
   {
      set. print( out );
      return out;
   }

}

#endif


