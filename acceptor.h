
// Written by Dina Muktubayeva and
// Hans de Nivelle, March/April 2021.

#ifndef MAPH_LEXING_ACCEPTOR_
#define MAPH_LEXING_ACCEPTOR_  1

#include <climits>
#include <iostream>
#include <set>
#include <stack>
#include "flatmap.h"
#include "state.h"

#include "minmax.h"

namespace lexing
{
   constexpr static int blocked = std::numeric_limits<int>::max( );

   template< typename C > 
   struct deltafromstate
   {
      std::set< int > eps;           // epsilon transitions 
      flatmap< C, int > noneps;      // other transitions 

      // The integers represent states, relative to the present
      // state. 0 means a transition to the present state.
      
      deltafromstate( ) 
      {
         noneps. append( minval<C> (), blocked );
      }

      deltafromstate( std::set< int > && eps, flatmap< C, int > && noneps )
         : eps( std::move( eps )),
           noneps( std::move( noneps ))
      { }

      void print( std::ostream& out, C c, state base, int offset ) const 
      {
         if( c == minval<C> () ) 
            out << "MIN";
         else 
         {
            if( offset == blocked )
            {
               C c1 = c; -- c1;
               out << c1 << "+1";
            }
            else
               out << c;
         }

         if( offset == blocked )
            out << " / blocked";
         else
            out << " / " << base + offset; 
      }

      void print( std::ostream& out, state base ) const
      {
         out << "   " << base << " : ";
         out << "{";
         for( auto p = eps.begin(); p != eps.end(); ++p )
         {
            if( p != eps.begin())
               out << ", ";
            else
               out << " ";

            if( *p != blocked)
               out << base + *p;
            else
               out << *p;
         }

         out << " }"; 

         // If eps is short, we put more spaces, hoping that 
         // the column with noneps will be better formatted:

         for( size_t i = 3 * eps. size( ); i < 14; i += 3 ) 
            out << "   ";
         out << "  ";

         out << "{ ";
         for( auto p = noneps. begin( ); p != noneps. end( ); ++ p ) 
         {
            if( p != noneps. begin( ))
               out << ", ";
            else
               out << " ";

            print( out, p -> first, base, p -> second ); 
         }
         out << " }";
      }
   };


   // We call an acceptor 'committing' if it never returns to the 
   // initial state. This makes all operations much easier.

   template< typename C > 
   struct acceptor 
   {
      std::vector< deltafromstate<C>> delta;

      acceptor( ) = default;  
      acceptor( const acceptor& ) = default;
      acceptor( acceptor&& ) noexcept = default;
      acceptor& operator = ( const acceptor& ) = default; 
      acceptor& operator = ( acceptor&& acc ) noexcept = default;
      ~acceptor( ) = default;

      acceptor copy( ) const
         { return *this; }

      size_t nrstates( ) const 
         { return delta. size( ); } 

      acceptor& operator *= ( acceptor<C> a )
      {
         for( auto& d : a. delta )
            delta. push_back( std::move(d) );

         return *this;
      }

      acceptor& operator |= ( acceptor<C> a ) 
      {
         delta. push_back( deltafromstate<C> ( ));
         delta. back( ). eps. insert( a. nrstates( ) + 1 );

         delta. front( ). eps. insert( state( nrstates( )) - 0 );

         for( auto& d : a. delta ) 
            delta. push_back( std::move(d) );

         return *this;
      }   

      // Star has to make a copy, because otherwise we would
      // introduce a transition to the beginning:

      acceptor&& star( ) && 
      {
         delta. insert( delta. begin( ), deltafromstate<C> ( )); 
         delta. front( ). eps. insert(1);

         delta[1]. eps. insert( nrstates( ));
 
         delta. push_back( deltafromstate<C> ( ));
         delta. back( ). eps. insert( -( nrstates( ) - 2 ));

         return std::move( *this );
      }

      acceptor star( ) const & 
      {
         auto res = copy( ). star( );
         return res; 
      }

      acceptor&& plus( ) &&
      {
         delta. insert( delta. begin( ), deltafromstate<C> ( ));
         delta. front( ). eps. insert(1);

         delta. push_back( deltafromstate<C> ( ));
         delta. back( ). eps. insert(1); 
         delta. back( ). eps. insert( -( nrstates( ) - 2 ));

         return std::move( *this );
      }

      acceptor plus( ) const & 
      {
         auto res = copy( ). plus( );
         return res; 
      }

      acceptor&& optional( ) &&
      {
         if( nrstates( ))
            delta. front( ). eps. insert( nrstates( ));  
         return std::move( *this );
      }
 
      acceptor optional( ) const & 
      {
         auto res = copy( ). optional( );
         return res;
      }

      acceptor&& without( C c1, C c2 ) &&
      {
         if( c1 > c2 )
            std::swap( c1, c2 );

         flatmap< C, bool > m;
         if( c1 != minval<C> ( ))
            m. append( minval<C>( ), false );
         m. append( c1, true );
         if( c2 != maxval<C>( ))
         {
            ++ c2;   
            m. append( c2, false );
         }
   
         if constexpr( false ) 
         {
            for( const auto& p : m )
               std::cout << p.first << " -> " << p. second << "\n";
         }

         for( auto& s : delta )
         {
            s. noneps = transform<C,int>( s. noneps, m, []( int x, bool b ) 
               { return b ? blocked : x; } );
         }
         return std::move( *this );
      }

      acceptor without( C c1, C c2 ) const &
      {
         auto res = copy( ). without( c1, c2 ); 
         return res;
      }

      acceptor&& without( C c ) && 
      {  
         return std::move( *this ). without( c, c );
      }
 
      void print( std::ostream& out ) const
      {
         out << "Acceptor:\n";

         for( size_t s = 0; s != nrstates( ); ++s )
         {
            state base(s);
            delta[s].print( out, base );
            out << "\n";
         }
      }

   };

   template< typename C >
   acceptor<C> epsilon( )
   {
      acceptor<C> res;
      return res; 
   }

   template< typename C > 
   acceptor<C> empty( )
   {
      acceptor<C> res;
      res. delta. push_back( deltafromstate<C> ( ));
      return res; 
   }

   template< typename C > 
   acceptor<C> range( C c1, C c2 )
   {
      if( c1 > c2 ) 
         std::swap( c1, c2 ); 

      flatmap< C, int > noneps;

      if( c1 != minval<C>( ) )
         noneps. append( minval<C> ( ), blocked );

      noneps. append( c1, 1 );

      // If c2 != maxval<C> ( ), we add (c2+1)/blocked.

      if( c2 != maxval<C> ( ))
      {
         ++ c2;
         noneps. append( c2, blocked );
      }

      std::set< int > eps;

      acceptor<C> res;
      res.delta. push_back( 
             deltafromstate<C> ( std::move(eps), std::move(noneps) )); 
      return res; 
   }

   template< typename C > 
   acceptor<C> every( ) 
   {
      return range( minval<C> ( ), maxval<C> ( ));
   }

   template< typename C > 
   acceptor<C> just( C c )
   {
      return range(c,c);
   }

   template< typename C > acceptor<C> 
   operator * ( acceptor<C> a1, acceptor<C> a2 )
   {
      a1 *= std::move(a2);
      return a1;
   }

   template< typename C > 
   acceptor<C> word( const C* w )
   {
      acceptor<C> res;

      while( *w != 0 )
         res *= just( *w ++ );
      return res; 
   }

   template< typename C >
   acceptor<C> 
   operator | ( acceptor<C> a1, acceptor<C> a2 )
   {
      a1 |= std::move(a2);
      return a1;
   }

   template< typename C > 
   inline std::ostream& 
   operator << ( std::ostream& out, const acceptor<C> & acc )
   {
      acc. print( out );
      return out;
   }

}

#endif

