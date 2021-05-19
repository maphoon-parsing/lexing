
#include <iostream>
#include <vector>

#ifndef MAPH_LEXING_FLATMAP
#define MAPH_LEXING_FLATMAP  1

// C must have an order < on it. There are no requirements on
// S.
// Do you know a better name?

namespace lexing
{

   template <typename C, typename S >
   struct flatmap
   {
      std::vector<std::pair<const C, S>> repr;
	// Must be sorted by increasing C.

      flatmap( ) = default;
      flatmap( const flatmap& ) = default;
      flatmap( flatmap&& ) = default;
      flatmap& operator = ( const flatmap& ) = default;
      flatmap& operator = ( flatmap&& ) = default;
      ~flatmap( ) = default;


      flatmap( std::initializer_list<std::pair<const C, S>> init)
      {
         repr. reserve( init. size( ));
         for( auto p : init )
            append( p.first, p.second );
      }

      // c must be bigger than everything encountered so far.

      void append( const C& c, const S& s )
      {
         if( !repr. empty( ))
         {
            if( !( repr. back( ). first < c ))
               throw std::runtime_error( "flatmap append: not increasing" );
                  // check if the transition is already there

            if( repr. back( ). second == s )
               return; 
                  // The addition is redundant, so we don't add. 
         }

         repr. push_back( std::make_pair(c,s) );
      }

      using iterator =
         typename std::vector<std::pair<const C, S>>::iterator;
      using const_iterator =
         typename std::vector<std::pair<const C, S>>::const_iterator;

      iterator begin() { return repr.begin(); }
      iterator end() { return repr.end(); }

      const_iterator cbegin() const { return repr.begin(); }
      const_iterator cend() const { return repr.end(); }
      const_iterator begin() const { return repr.begin(); }
      const_iterator end() const { return repr.end(); }

      // Uses binary search : 

      const_iterator find( const C& c ) const 
      {
         const_iterator low = begin( );
         const_iterator upp = end( );

         while( low + 3 < upp )
         {
            const_iterator mid = low + (( upp - low ) >> 1 );

            if( mid -> first <= c )
               low = mid;
            else
               upp = mid;
         }

         // When the interval is small enough, we search backwards:

         while( upp > low )
         {
            -- upp;
            if( upp -> first <= c )
               return upp;
         }

         return end( );
      }

      void clear() { repr.clear(); }
      size_t size() const { return repr. size(); }
      bool empty() const { return repr. empty( ); }
   };


   template< typename C, typename R, typename S1, typename S2, typename F >
   flatmap<C,R> 
   transform( const flatmap<C,S1> & m1, const flatmap<C,S2> & m2, const F& f ) 
   {
      auto p1 = m1. begin( );
      auto p2 = m2. begin( );
      if( p1 == m1. end( ) || p2 == m2. end( ))
         throw std::runtime_error( "transform, size cannot be zero" );
      if( p1 -> first != p2 -> first )
         throw std::runtime_error( "transform, maps must start at same C" );

      flatmap<C,R> res;
 
      C c = p1 -> first;

      // c is the point that we are looking at.
      // p1,p2 are positions that determine the value of c.
      // p1,p2 reach the end at the same time.

      while( p1 != m1. end( ) || p2 != m2. end( ))
      {
         // std::cout << "c = " << c << "\n"; 
         // std::cout << "f = " << f( p1 -> second, p2 -> second ) << "\n";

         res. append( c, f( p1 -> second, p2 -> second ));
      
         ++ p1; 
         if( p1 == m1. end( ))
         {
            -- p1;
            ++ p2;
            if( p2 == m2. end( ))
               ++ p1;    // that's the end. 
            else
               c = p2 -> first;
         }
         else
         {
            ++ p2;
            if( p2 == m2. end( ))
            { 
               -- p2;
               c = p1 -> first;
            }
            else
            {
               if( p1 -> first == p2 -> first )
               {
                  c = p1 -> first;
               }
               else
               {
                  if( p1 -> first < p2 -> first )
                  {
                     -- p2;
                     c = p1 -> first;
                  }
                  else
                  {
                     -- p1;
                     c = p2 -> first;
                  }
               }
            }
         }
      }

      return res;
   }

}

#endif

