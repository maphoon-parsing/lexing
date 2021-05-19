
// Code written by Hans de Nivelle, May 2021.

#include "partition.h"
#include <algorithm>

void lexing::partition::refine( const stateset& filter, 
                                std::stack< size_t > & unchecked )
{
   for( state s : filter )
   {
      stateset& overlap = eqclass[ index[ s. nr ]]; 
         // The equivalence class of s. 
         // s is a shared element between filter and overlap. 

      if( !subset( overlap, filter ))
      {
         // std::cout << "filter " << filter << " refines " << overlap << "\n";

         stateset removed; 

         for( state s : filter )
         {
            if( overlap. erase(s)) 
               removed. insert(s); 
         }
 
         if( overlap. size( ) < removed. size( ))
            std::swap( removed, overlap ); 

         // std::cout << "the result is " << overlap << " + " << removed << "\n";

         for( state s : removed )
         {
            index[ s. nr ] = eqclass. size( );
         } 
         unchecked. push( eqclass. size( )); 

         eqclass. push_back( std::move( removed ));
           // At this point, overlap is spoilt, but it does not matter. 

      }
   }   

}


namespace
{
   lexing::state minimum( const lexing::stateset& s )
   {
      return *s. begin( );
   }
}


void lexing::partition::sortbyminimum( )
{
   std::sort( eqclass. begin( ), eqclass. end( ),
              []( const stateset& s1, const stateset& s2 )
                  { return minimum(s1) < minimum(s2); } ); 

   // And we need to correct the indices:

   setindex( );
}


void lexing::partition::setindex( )
{
   for( size_t c = 0; c != eqclass. size( ); ++ c )
   {
      for( state s : eqclass[c] )
      {
         // If s not yet in index, we create undefined values 
         // until we have place for it.

         while( s. nr >= index. size( ))
            index. push_back( eqclass. size( ));
         index[ s. nr ] = c; 
      }      
   }
}


bool lexing::partition::wellformed( ) const
{
   unsigned int err = 0; 

   for( size_t i = 0; i != index. size( ); ++ i )
   {
      size_t c = index[i];
      if( c >= eqclass. size( ))
      {
         std::cout << "index[" << i << "] >= eqclass. size( )\n";
         ++ err;
      }
      else
      { 
         if( !eqclass[c]. contains(i) )
         {
            std::cout << i << " not in eqclass[ " << c << " ]\n";
            ++ err;
         }
      }
   }

   for( size_t cl = 0; cl != eqclass. size( ); ++ cl )
   {
      for( state s : eqclass[ cl ] )
      {
         if( s. nr > index. size( ))
         {
            std::cout << s << " bigger than index\n";
            ++ err;
         }
        
         if( index[ s. nr ] != cl )
         {
            std::cout << s << " not in index[" << s. nr << "]\n";
            ++ err;
         }
      }
   }

   return err == 0; 
}

void lexing::partition::print( std::ostream& out ) const
{
   out << "Equivalence Classes:\n";
   for( size_t i = 0; i != eqclass. size( ); ++ i )
      out << i << "   : " << eqclass[i] << "\n";
   out << "\n";

   if constexpr( true )
   {
      out << "Index:\n";
      for( size_t i = 0; i != index. size( ); ++ i )
      {
         out << "   " << i << " -> " << index[i] << "\n";
      }
      out << "\n";
   }
}


