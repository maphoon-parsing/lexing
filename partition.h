
// Written by Hans de Nivelle, May 2021.

#ifndef MAPH_LEXING_PARTITION
#define MAPH_LEXING_PARTITION  1

#include <vector> 
#include <stack> 
#include <map>

#include "stateset.h"

// `partition' means 'state partition'.
// The implementation follows the paper very accurately.

namespace lexing
{

   struct partition
   {

      std::vector< stateset > eqclass;
         // The equivalence classes.

      std::vector< size_t > index;
         // Maps states into their eqclass.

      partition( ) = default;

      template< typename K, typename O > 
      partition( std::map<K,stateset,O> && part )
      {
         for( auto& p : part )
            eqclass. push_back( std::move( p. second ));
       
         part. clear( );
            // To remove the last remaining doubt about
            // our destructiveness.
 
         setindex( );
      }
  
      void refine( const stateset& filter, 
                   std::stack< size_t > & unchecked );
         // If s1 in filter, and s2 not in filter, then 
         // s1 and s2 cannot be in the same equivalence class.
         // We check all eqclasses for which this happens 
         // and split them into two classes. 
         // If we split a class, we append the smaller
         // to eqclass, and put its index on unchecked. 

      void sortbyminimum( ); 
         // Sort eqclass by minimum elements in the classes.
         // This is useful for two reasons: 
         // The initial state must be Q0. This means that
         // Q0 of the old automaton must be in class 0 of 
         // the new automaton. Sorting by smallest element 
         // ensures this.  
         // It is easier to read the new automaton when it  
         // is similar to the old. Sorting somewhat preserves
         // the order of states in the original, 
         // nondeterministic classifier, which preserves
         // readability. 

      void print( std::ostream& out ) const; 

      void setindex( );
         // Set the index from the classes.  
         // Note that if there are open places, their index 
         // will be eqclass. size( ), which means undefined. 

      bool wellformed( ) const;
         // Check if the partition is well-formed,
         // and complain if it is not.
   };


   inline 
   std::ostream& operator << ( std::ostream& out, const partition& part )
   {
      part. print( out );
      return out;
   }

}

#endif

