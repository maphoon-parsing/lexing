
#ifndef LEXING_CLASSIFIER_INCLUDED
#define LEXING_CLASSIFIER_INCLUDED  1

// Written by Hans de Nivelle and Dina Muktubayeva, 
// March/April 2021.

#include "acceptor.h"

namespace lexing
{

   // A classifying automaton is able to classify.
   // We assume that state 0, which recognizes the empty word,
   // defines the error class.

   template< typename C, typename T > 
   struct classifier
   {
      std::vector< deltafromstate<C>> delta;
      std::vector<T> classifications;
         // We always use classifications[0] as the error class. 

      classifier( ) = delete; 
         // Because we need to have the error in classifications[0].

      classifier( T error ) 
      {
         delta. push_back( deltafromstate<C> ( ));
       
         classifications. push_back( error ); 
      }

      const T& errorclass( ) const { return classifications[0]; }
         // classifications[0] is always the error class.

      void insert( acceptor<C> && acc, T tok )
      {
         delta. front( ). eps. insert( nrstates( ));

         for( size_t i = 0; i != acc. nrstates( ); ++ i ) 
         {
            delta. push_back( std::move( acc. delta[i] ));
            classifications. push_back( classifications. front( ));
         } 

         delta. push_back( deltafromstate<C> ( )); 
         classifications. push_back( tok );
      }

      void insert( const acceptor<C> & acc, T tok )
         { insert( acc. copy( ), tok ); }

      size_t nrstates( ) const { return delta. size( ); } 

      void print( std::ostream& out ) const
      {
         out << "Classifier:\n";
         for( size_t s = 0; s != delta. size(); ++ s )
         {
            state base(s);
            delta[s].print( out, base );

            if( classifications[s] != errorclass( ))
               out << "   " << classifications[s];
            out << "\n";
         }
      }

   };


   template< typename C, typename T >
   std::ostream& operator << ( std::ostream& out, 
                               const classifier< C, T > & cl )
   {
      cl. print( out ); 
      return out;
   }

}

#endif

