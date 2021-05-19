
// Written by Hans de Nivelle, May 2021.

#ifndef MAPH_LEXING_REACHABLECLASSES
#define MAPH_LEXING_REACHABLECLASSES  1

namespace lexing
{

   template< typename T > 
   struct reachableclasses
   {
      // Pairs of distance and T,
      // sorted by T.

      static constexpr size_t maxsize = 2;

      std::vector< std::pair< size_t, T >> reach;

      reachableclasses( ) = default;

      reachableclasses( T t )  
      {
         reach. push_back( std::make_pair( 0, t ));
      }

      void add1( )
      {
         for( auto& p : reach )
            ++ p. first;
      }

      void print( std::ostream& out ) const
      {
         out << "( ";
         for( auto p = reach. begin( ); p != reach. end( ); ++ p )
         {
            if( p != reach. begin( ))
               out << ", ";
            out << ( p -> first ) << "/" << ( p -> second );
         }
         out << " )";
      }

      size_t size( ) const { return reach. size( ); }

      using iterator = 
         typename std::vector< std::pair< size_t, T >> :: iterator;

      using const_iterator = 
          typename std::vector< std::pair< size_t, T >> :: const_iterator; 

      iterator begin( ) { return reach. begin( ); }
      iterator end( ) { return reach. end( ); }

      const_iterator begin( ) const { return reach. cbegin( ); }
      const_iterator end( ) const { return reach. cend( ); }

      void push_back( const std::pair< size_t, T > & p )
         { reach. push_back(p); } 
   };

 
   template< typename T >
   reachableclasses<T>
   merge( const reachableclasses<T> & r1, const reachableclasses<T> & r2 )
   {
      reachableclasses<T> res;

      auto p1 = r1. begin( );
      auto p2 = r2. begin( );

      while( p1 != r1. end( ) && p2 != r2. end( ) && 
             res. size( ) <= reachableclasses<T> :: maxsize )
      {
         if( p1 -> second == p2 -> second )
         {
            size_t d1 = p1 -> first;
            size_t d2 = p2 -> first;
            
            res. push_back( 
                   std::make_pair( d1 < d2 ? d1 : d2, p1 -> second ));

            ++ p1; ++ p2;
         }
         else
         {
            if( p1 -> second < p2 -> second )
               res. push_back( *p1 ++ );
            else
               res. push_back( *p2 ++ );
         }
      }

      while( p1 != r1. end( ) && 
             res. size( ) <= reachableclasses<T> :: maxsize )
      {
         res. push_back( *p1 ++ );
      }

      while( p2 != r2. end( ) && 
             res. size( ) <= reachableclasses<T> :: maxsize ) 
      {
         res. push_back( *p2 ++ );
      }

      return res;  
   }

   template< typename T > 
   bool 
   operator == ( const reachableclasses<T> & r1, const reachableclasses<T> & r2 )
   {
      if( r1. size( ) == r2. size( ))
      {
         auto p1 = r1. begin( );
         auto p2 = r2. begin( );

         while( p1 != r1. end( ))
         {
            if( p1 -> first != p2 -> first || p1 -> second != p2 -> second )
               return false;

            ++ p1; ++ p2;
         }
         return true;
      }
      else
         return false; 
   }

   template< typename T >
   bool 
   operator != ( const reachableclasses<T> & r1, const reachableclasses<T> & r2 )
      { return ! ( r1 == r2 ); }  


   template< typename T >
   bool
   operator < ( const reachableclasses<T> & r1, const reachableclasses<T> & r2 )
   {
      if( r1. size( ) != r2. size( )) 
         return r1. size( ) < r2. size( ); 

      auto p1 = r1. begin( );
      auto p2 = r2. begin( );

      while( p1 != r1. end( ))
      {
         if( p1 -> first != p2 -> first )
            return ( p1 -> first ) < ( p2 -> first );

         if( p1 -> second != p2 -> second )
            return ( p1 -> second ) < ( p2 -> second );

         ++ p1; ++ p2;
      }
      return false;
   }

   template< typename T > 
   inline std::ostream& 
   operator << ( std::ostream& out, const lexing::reachableclasses<T> & rc )
   {
      rc. print( out );
      return out;
   }

}

#endif

