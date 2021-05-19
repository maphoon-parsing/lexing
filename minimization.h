
#ifndef MAPH_LEXING_MINIMIZATION
#define MAPH_LEXING_MINIMIZATION  1

#include "partition.h"
#include "reachableclasses.h"
#include <stack>
#include <queue>

namespace lexing
{
   
   // Collect the states from from that have a c-transition into
   // into. 

   template< typename C > stateset
   transition_into( const stateset& from,
                    const std::vector< deltafromstate<C>> & delta,
                    const stateset& into, C c )
   {
      stateset result;

      for( auto st : from )
      {
         auto p = delta[ st.nr ].noneps.find(c);
         if( p -> second != blocked &&
                into. contains( st + p -> second ))
         {
            result. insert( st );
         }
      }
      return result; 
   } 

  
   template< typename C, typename T > 
   partition 
   initialpartitionwithfuture( const classifier<C,T> & cl,
                               const std::vector< stateset > & backtransitions )
   {
      std::vector< reachableclasses<T>> reachables; 
      for( size_t c = 0; c != cl. nrstates( ); ++ c )
         reachables. push_back( reachableclasses( cl.classifications[c] ));

      std::queue< size_t > unchecked;
         // Check what works better, stack or queue.

      for( size_t c = 0; c != cl. nrstates( ); ++ c ) 
         unchecked. push(c);

      while( unchecked. size( ))
      {
         auto u = unchecked. front( ); unchecked. pop( ); 
        
         auto r = reachables[u];
         r. add1( ); 
         for( state b : backtransitions[u] )
         {
            auto m = merge( reachables[ b.nr ], r );
            if( m != reachables[ b. nr ] )
            {
               reachables[ b. nr ] = m; 
               unchecked. push( b. nr );
            } 
         }
      }

      if constexpr( false ) 
      {
         for( size_t c = 0; c != cl. nrstates( ); ++ c )
         {
            std::cout << c << "  " << reachables[c] << "\n";
         } 
      }

      std::map< reachableclasses<T>, stateset > classindex; 
      for( size_t c = 0; c != cl. nrstates( ); ++ c )
      {
         const auto& r = reachables[c];
         classindex[r]. insert(c);
      }

      partition part = std::move( classindex ); 
      return part;
   }


   template< typename C, typename T >
   partition
   initialpartition( const classifier<C,T> & cl )
   {
      std::map< T, stateset > classindex;
         // Here we need order on T.

      for( size_t i = 0; i != cl. nrstates( ); ++ i )
      {
         const T& t = cl. classifications[i];
         classindex[t]. insert(i);
      }

      partition part = std::move( classindex );
      return part;
   }

 
   template< typename C, typename T > 
   std::vector< stateset > 
   computebacktransitions( const classifier<C,T> & cl )
   {
      std::vector< stateset > back( cl. nrstates( ), stateset( )); 

      for( size_t i = 0; i != cl. nrstates( ); ++ i ) 
      {
         for( const auto& pr : cl. delta[i]. noneps )
         {
            if( pr. second != blocked )
            {
               state target = state(i) + pr. second; 
               back[ target. nr ]. insert( state(i) );
            }
         }
      }

      if constexpr( false )
      {  
         std::cout << "Back Transitions:\n\n";
         for( size_t i = 0; i != back. size( ); ++ i )
            std::cout << i << ":   " << back[i] << "\n";
      }
      return back;
   }

   
   // Divide the automaton by the given state partition:
 
   template< typename C, typename T > 
   classifier<C,T> 
   quotient( const classifier<C,T> & cl, 
             const partition& part )
   {
      T error = cl. errorclass( );

      classifier<C,T> simp( error ); 
      simp. delta. clear( );
      simp. classifications. clear( );
 
      // We need to be sure that state 0 is mapped to zero.
      // If you see this error, either change the std::swap in partition, 
      // or solve the problem here. Removing the swap is 
      // certainly the easiest solution. You can also 
      // add a method to partition that solves this problem. 
      // I am not even sure if it can happen. 

      if( part.index[0] != 0 )
         throw std::runtime_error( "this must not happen" );
 
      for( size_t st = 0; st != part. eqclass. size( ); ++ st )
      {
         state repr = *part. eqclass[st]. begin( );  

         flatmap< C, int > noneps; 

         for( const auto& p : cl. delta[ repr. nr ]. noneps )
         {
            if( p. second != blocked )
            {
               state to = repr + p. second; 
               to = part. index[ to. nr ];  
               noneps. append( p. first, to - st );
            }
            else
            {
               noneps. append( p. first, blocked ); 
            }
         }
 
         simp. delta. push_back( deltafromstate<C>( std::set<int>(),
                                                    std::move( noneps ) ));
         simp. classifications. push_back( cl. classifications[ repr. nr ] );
      }

      return simp;
   }


   // The token tyes T must be ordered because we index on them.
   // It does not need to be a meaningful order, but
   // we need a total order. 
 
   template< typename C, typename T > 
   classifier<C,T> 
   minimize( const classifier<C,T> & cl )
   {
      if( cl. nrstates( ) == 0 )
         throw std::logic_error( "minimize: need at least one state" );

      for( const auto& d : cl. delta )
      {
         if( d. eps. size( ))
            throw std::runtime_error( 
                           "minimize: classifier is not deterministic" );
      }

      auto backtransitions = computebacktransitions( cl ); 
         // backtransitions[s] is the set of states that have a transition into 
         // state(s). 

      partition part = initialpartitionwithfuture( cl, backtransitions );
      if( !part. wellformed( )) 
         throw std::runtime_error( "obviously a major malfunction" ); 
            // You may start looking looking very carefully into the situation.

      // std::cout << "Initial Partition:\n";
      // std::cout << part << "\n";

      std::stack< size_t > unchecked;
         // It clearly follows from the literature that
         // stack is better than queue.

      for( size_t i = 0; i != part. eqclass. size( ); ++ i )
         unchecked. push(i);

      // unchecked contains indices of part. eqclass[] of classes
      // that need to checked against.

      while( unchecked. size( ))
      {
         size_t u = unchecked. top( ); unchecked. pop( );

         // std::cout << "picked equivalence class #" << u << " = ";
         // std::cout << part. eqclass[u] << "\n";

         stateset pred; 
            // We need the full set of predecessors.

         for( state s : part. eqclass[u] )
         {
            for( auto& b : backtransitions[ s. nr ] )
               pred. insert(b); 
         }

         // std::cout << "set of its predecessors " << pred << "\n";

         std::set<C> bord = borders( cl. delta, pred ); 

         // std::cout << "set of its borders " << bord << "\n";

         for( C b : bord )
         {
            // Determine the set of states that go into u.

            stateset filter = 
               transition_into( pred, cl. delta, part. eqclass[u], b );

            if( filter. size( ))
            {
               // std::cout << "border " << b << "\n";
               // std::cout << "results in non-empty filter " << filter << "\n";

               part. refine( filter, unchecked );
            }
         }
      } 

      part. sortbyminimum( ); 
      return quotient( cl, part ); 
   }


}


#endif

