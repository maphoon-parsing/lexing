
#ifndef MAPH_LEXING_ALGORITHMS
#define MAPH_LEXING_ALGORITHMS  1

#include "acceptor.h"
#include "classifier.h"
#include "state.h"
#include "stateset.h" 
#include <stack>

#include <unordered_map>

namespace lexing
{

   template< typename C > stateset
   eps_closure( const std::vector< deltafromstate<C>> & delta,
                const stateset& start )
   {
      stateset closure;

      std::stack<state> unchecked;

      for( auto st : start )
      {
         closure. insert( st );
         unchecked. push( st );
      }

      // unchecked is always a subset of closure.

      while( !unchecked. empty() ) 
      {
         state s = unchecked.top();
         unchecked.pop();

         // i enumerates the states that are reachable by epsilon 
         // transition from s. 
         // Since i is always relative, we have to check ( s + i ).

	 for( int i : delta[ s.nr ]. eps ) 
         {
	    // check if the state has been visited

	    if( !closure. contains( s + i ))
            {
               unchecked.push( s + i );
               closure.insert( s + i );
            }
         }
      }

      return closure;
   }


   // Returns the c-transition from the given set of states

   template< typename C > stateset
   transition( const std::vector< deltafromstate<C>> & delta,
               const stateset& from, C c ) 
   {
      stateset result; 

      // Insert c-transitions from every state in from:

      for( auto st : from ) 
      {
         auto p = delta[ st.nr ].noneps.find(c); 

         // check if p points to valid offset value

         if( p -> second != blocked ) 
            result. insert( st + p -> second );
      }

      return result;
   }


   // Returns the set of borders of the transition functions
   // of the states in set. 

   template< typename C >
   std::set<C> borders( const std::vector< deltafromstate<C>> & delta, 
                        const stateset& set ) 
   {
      std::set<C> res;

      for( auto s : set ) 
      {
         for( auto b : delta[ s.nr ]. noneps )
            res. insert( b. first );
      }

      return res;
   }

   // So that we can print character sets:

   template< typename C >
   std::ostream& operator << ( std::ostream& out, const std::set<C> & charset )
   {
      out << "{ ";
      for( auto p = charset. begin( ); p != charset. end( ); ++ p )
      {
         if( p != charset. begin( ))
            out << ", "; 
         out << *p;
      }
      out << " }";
      return out;
   }

         
   // Returns the classification given by the maximal state in
   // set from the states that do not classify as error, if at least
   // one state in set does not classify as error. 

   template< typename T > 
   T maxnoterror( const std::vector<T> & cl, T error, 
                  const stateset& set ) 
   {
      auto best = set. begin( );
      while( best != set. end( ) && cl[ best -> nr ] == error )
         ++ best;

      if( best == set. end( ))
         return error;           // No non-error state was found.

      auto p = best;
      ++ p;

      while( p != set. end( )) 
      {
         if( cl[ p -> nr ] != error && *p > *best ) 
         {
            best = p;
         }
         ++ p;
      }      
      return cl[ best -> nr ];
   }


   // Read and classify also works when the classifier is nondeterministic:
   // It is not very efficient.

   template< typename C, typename T, typename S > 
   std::pair< T, size_t >  
   readandclassify( const classifier<C,T> & cl, S& inp )
   {
      //  std::cout << "non deterministic read and classify\n";

      stateset reachable; 
      reachable. insert( state(0) );
      reachable = eps_closure( cl. delta, reachable );

      std::pair<T,size_t> latest = std::make_pair( cl. errorclass( ), 0 );
         // We remember the latest classification that was not error,
         // as soon as we encounter one.

      if( !inp. has(1))
         return latest; 
 
      size_t i = 0; 

      while( true )
      {
         T t = maxnoterror( cl.classifications, cl.errorclass( ), reachable );
         if( t != cl. errorclass( ))
         {
            latest. first = t; 
            latest. second = i; 
         }
 
         // std::cout << "latest classification ";
         // std::cout << latest. first << "/" << latest. second << "\n";

         if( !inp. has( i + 1 )) 
            return latest;
     
         C c = inp. peek(i);

         reachable = transition( cl. delta, reachable, c );

         if( reachable. size( ) == 0 )
            return latest;          // No possible states left. 

         reachable = eps_closure( cl. delta, reachable );

         // std::cout << "move to states " << reachable << "\n";
         ++ i;
      }
   }



   template< typename C, typename T >
   classifier<C,T> 
   make_deterministic( const classifier<C,T> & cl )
   {
      stateset initial;
      initial. insert(0);
      initial = eps_closure( cl. delta, initial ); 

      T error = cl. errorclass( ); 

      std::cout << "initial = " << initial << "\n";

      for( auto s : initial )
      {
         if( cl. classifications[ s. nr ] != error )
         {
            std::cout << "empty word classifies as " << 
            cl. classifications[ s. nr ] << "\n";
            throw std::logic_error( "cannot make deterministic" ); 
         }
      }

      std::cout << "error class = " << error << "\n";
      std::cout << "\n";

      classifier<C,T> det( error );
      det. delta. clear( );
      det. classifications. clear( );
 
      // Index of subsets that we have obtained in 
      // the subset construction. It maps the constructed subsets
      // to states of det.

      std::unordered_map< stateset, unsigned int, 
                          stateset::hash, stateset::equal_to > index; 

      std::vector< stateset > states; 
         // Constructed states. states[i] is the i-th state in det. 

      // We are now storing subsets twice, but I think it is no problem.
      // States are stored closed, because otherwise we may miss 
      // equal states.

      index. insert( std::make_pair( initial, 0 ));
      states. push_back( initial );
      size_t checking = 0;

      while( checking < states. size( ))
      {
         // std::cout << "checking state " << checking << "\n";
         // std::cout << "set = " << states[ checking ] << "\n";

         std::set<C> bord = borders( cl.delta, states[ checking ] ); 
         // std::cout << "borders = " << bord << "\n";

         T t = maxnoterror( cl. classifications, cl. errorclass( ),
                         states[ checking ] );

         // std::cout << "classification will be " << t << "\n\n";

         flatmap< C, int > noneps; 

         // We will append a transition for each border to noneps: 

         for( auto c : bord )
         {
            // discover the states reachable over the c-transition

            stateset next = transition( cl.delta, states[ checking ], c );
          
	    next  = eps_closure( cl.delta,  next );

            if( next. size( )) 
            {
               auto p = index. insert( std::make_pair( next, states. size( ))); 
               if( p. second )
                  states. push_back( next );

               int rel = state( p. first -> second ) - state( checking ); 
                 // relative difference to the next state. 
                 // If you write it without conversion to state,
                 // result will be unsigned. It will work, but
                 // only coincidentally.  

               noneps. append( c, rel ); 
            }
            else
               noneps. append( c, blocked );
         }

         det. delta. push_back( deltafromstate<C>( std::set<int> ( ), 
                                                   std::move( noneps ) ));
         det. classifications. push_back(t);
         ++ checking; 
      }

      return det; 
   }

}

#endif
