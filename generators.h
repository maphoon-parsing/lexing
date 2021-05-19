
#ifndef MAPH_LEXING_GENERATORS
#define MAPH_LEXING_GENERATORS  1

#include <functional>
#include <iostream>
#include <fstream>
#include <map>

#include "indentation.h"
#include "deterministic.h"
#include "trivial.h"
 
namespace lexing
{

   // Generates a function that returns a deterministic classifier from table

   template< typename C, typename T >
   void printdeterministic( 
      const std::string& Cname, const std::string& Tname,
      const std::string& name, 
      const lexing::classifier<C,T> & cl, std::ostream& file,
      const std::function< void( std::ostream& out, const C& c ) > & printC,
      const std::function< void( std::ostream& out, const T& t ) > & printT,
      indentation ind = indentation( ))
   {
      for( const auto& d : cl. delta )
      {
         if( d. eps. size( ))
            throw std::runtime_error(
                     "printdeterministic: classifier is not deterministic" );
      }

      file << ind << "lexing::deterministic< ";
      file << Cname << ", " << Tname << " > ";
      file << name << "( )\n{\n";

      ind += 3;

      file << ind << "std::vector< lexing::deterministic< ";
      file << Cname << ", " << Tname << " > :: state_info > st = ";
      file << "{";
      
      size_t start = 0;
      for( size_t i = 0; i != cl. nrstates(); ++ i ) 
      {
         if( i != 0 )
            file << ", ";

         if( i % 4 == 0 )
            file << "\n" << (ind+3);

         file << "{ " << start << ", ";
         printT( file, cl. classifications[i] );
         file << " }";  

         start += cl. delta[i]. noneps. size( ); 
      }
      file << "\n" << ind << "};\n";
      file << "\n";

      file << ind << "std::vector< lexing::deterministic< ";
      file << Cname << ", " << Tname << " > :: trans > tr =\n";
      file << ind << "{";

      size_t nr = 0; 
      for( size_t i = 0; i != cl. nrstates(); ++ i )
      {
         for( const auto& p : cl. delta[i]. noneps )
         {
            if( nr != 0 )
               file << ", ";

            if( nr % 5 == 0 )
               file << "\n" << (ind+3);

            file << "{ "; 
            printC( file, p. first );
            file << ", ";  
            if( p. second == blocked )
               file << deterministic<C,T> :: blocked; 
            else
               file << i + p. second;

            file << " }";
            ++ nr;
         }
      }

      file << "\n"; 
      file << ind << "};\n\n";

      file << ind << "return lexing::deterministic< ";
      file << Cname << ", " << Tname << " > ";
      file << "( std::move(st), std::move(tr) );\n}\n";
      file << "\n";
   }


   // The map singleton maps decisions (relative state transitions
   // represented by integers which can be 'blocked') 
   // to the characters (C) that have them.
   // totalsize adds the total number of transitions.
   // If have has a/2 b/2  c/ blocked, d/ blocked,
   // we have 2 -> (a,b)   blocked -> (c,d). 

   template< typename C > 
   size_t totalsize( const std::map< int, std::vector<C>> & singletons )
   {
      size_t s = 0;
      for( const auto& p : singletons )
         s += p. second. size( );
      return s;
   }


   template< typename C >
   std::ostream& 
   operator << ( std::ostream& out, 
                 const std::map< int, std::vector<C>> & singletons )
   {
      for( const auto& s : singletons )
      {
         out << s. first << "      : { " ;
         for( auto p = s. second. begin( ); p != s. second. end( ); ++ p )
         {
            if( p != s. second. begin( ))
               out << ", ";
            out << *p; 
         }
         out << " }\n";
      }
      out << "\n";
      return out; 
   }


   // Separate delta into the singletons, and the rest.

   template< typename C >
   std::pair< std::map< int, std::vector<C>>,
              flatmap< C, int >>
   getsingletons( const flatmap<C,int> & delta )
   {
      lexing::flatmap<C,int> delta_reduced;
      std::map<int, std::vector<C>> singles; 

      for( auto p = delta. begin(); p != delta. end(); ++p )
      {
         // if the beginning of the next interval is the next character
         // after the beginning of the current interval, add it to map
         // otherwise, it stays in the border function

         auto p1 = p;
         ++ p1; 

         if( ( p1 != delta. end( ) &&
               p -> first != maxval<C> ( ) && p -> first + 1 == p1 -> first ) ||
             ( p == delta. end( ) && p -> first == maxval<C> ( ))) 
         {
            singles[ p -> second ]. push_back( p -> first ); 
         }
         else
            delta_reduced.append( p -> first, p -> second );
      }
      return std::make_pair( std::move( singles ), std::move( delta_reduced )); 
   }


   template< typename C, typename T >
   void singleton_iforswitch( state s,
      const std::map< int, std::vector<C>> & singletons, 
      indentation ind, std::ostream& file,
      const std::function< void( std::ostream& out, const C& c )> & printC,
      const std::function< void( std::ostream& out, const T& t )> & printT )
   {
      if( totalsize( singletons ) < 4 )
      {
         for( const auto& opt : singletons )
         {
            file << ind << "if( "; 
            for( auto p = opt. second. begin( ); p != opt. second. end( ); ++ p )
            {
               if( p != opt. second. begin( ))
                  file << " || "; 
               file << "c == ";
               printC( file, *p );
            }
            file << " ) ";

            if( opt. first == blocked )
               file << "return latest;\n";
            else
               file << "{ ++ i; goto " << state( s + opt. first ) << "; }\n";
         }
      }
      else
      {
         file << ind << "switch( c )\n";
         file << ind << "{\n"; 

         for( const auto& opt : singletons )
         {
            for( const auto& c : opt. second ) 	
            {
               file << ind << "case ";
               printC( file, c );
               file << ":\n";
            }

            if( opt. first == blocked )
               file << (ind+3) << "return latest;\n";
            else
               file << (ind+3) << "++ i; goto " << state( s + opt. first ) << ";\n";
         }

         file << ind << "}\n";
      }
   }


   template< typename C, typename T >
   void linear_if( state s, 
      typename flatmap<C,int> :: const_iterator begin,
      typename flatmap<C,int> :: const_iterator end, 
      indentation ind, std::ostream& file, 
      const std::function< void( std::ostream& out, const C& c )> & printC,
      const std::function< void( std::ostream& out, const T& t )> & printT )
   {
      if( begin > end )
         throw std::logic_error( "order is wrong" );

      auto p = end; 
      while( p != begin ) 
      {
         -- p;

         if( p == begin )
            file << ind << "if constexpr( true ) ";
         else
         {
            file << ind << "if( c >= "; 
            printC( file, p -> first ); 
            file << " ) ";
         }
 
         if( p -> second == blocked )
            file << "return latest;\n";
         else
            file << "{ ++ i; goto " << state( s + p -> second ) << "; }\n";
      }
   }


   template< typename C, typename T >
   void binary_if( state s,
      typename flatmap<C,int> :: const_iterator begin,
      typename flatmap<C,int> :: const_iterator end,
      indentation ind, std::ostream& file,
      const std::function< void( std::ostream& out, const C& c )> & printC,
      const std::function< void( std::ostream& out, const T& t )> & printT )
   {
      if( end - begin < 5 )
      {
         linear_if( s, begin, end, ind, file, printC, printT );
      }
      else
      { 
         // We split in the middle:

         auto middle = begin + (( end - begin ) >> 1 ); 

         file << ind << "if( c >= "; 
         printC( file, middle -> first ); 
         file << " )\n";
         file << ind << "{\n";
         binary_if( s, middle, end, ind+3, file, printC, printT ); 
         file << ind << "}\n";
         file << ind << "else\n";
         file << ind << "{\n";
         binary_if( s, begin, middle, ind+3, file, printC, printT );
         file << ind << "}\n";  
      }
   }

   // This function generates C++ code for a working classifier.
   // The resulting program has the same interface as readandclassify.
   // The code follows the paper.
   // Cname : name of the C-type.
   // Tname : name of the T-type
   // Sname : name of the source type. It must have .has( ) and .peek( )
   // name : name of the function that we will create. 
   // Function printC prints a C, while
   // function printT prints a T.

   template< typename C, typename T >
   void printcode( 
      const std::string& Cname, const std::string& Tname, 
      const std::string& space, 
      const lexing::classifier<C,T> & cl, std::ostream& file,
      const std::function< void( std::ostream& out, const C& t ) > & printC,
      const std::function< void( std::ostream& out, const T& t ) > & printT,
      indentation ind = indentation( )) 
   {
      for( const auto& d : cl. delta )
      {
         if( d. eps. size( ))
            throw std::runtime_error(
                           "printcode: classifier is not deterministic" );
      }

      file << "\n";
      file << ind << "// automatically generated code:\n"; 
      file << ind << "// generator written by Hans de Nivelle and ";
      file << ind << "Dina Muktubayeva\n\n";

      file << ind << "template< typename S >\n";
      file << ind << "std::pair< " << Tname << ", size_t > ";
      file << space << "readandclassify";
      file << "( lexing::trivial triv, S& inp )\n";
      file << ind << "{\n";
      ind += 3;
      file << ind << "auto latest = std::make_pair< " << Tname << ", size_t > ( ";
      printT( file, cl. errorclass( )); file << ", 0 );\n";
      file << ind << "if( !inp. has(1))\n";
      file << (ind+3) << "return latest;\n";
      file << ind << Cname << " c = inp. peek(0);\n";
      file << ind << "size_t i = 0;\n";
      file << "\n";

      for( size_t i = 0; i < cl.nrstates(); ++ i )
      {
         file << (ind-3) << state(i) << ":\n";
         if( cl. classifications[i] != cl. errorclass( ))
         {
            file << ind << "latest. first = ";
            printT( file, cl.classifications[i] );
            file << "; latest. second = i;\n";
         }

         // If there are no transitions from i, there is no need to look at the
         // next C. 

         if( cl. delta[i]. noneps. size() != 1 ||
             cl. delta[i]. noneps. begin() -> second != blocked )
         {
            file << ind << "if( !inp. has(i+1))\n";
            file << (ind+3) << "return latest;\n";

            file << ind << "c = inp. peek(i);\n";

            if constexpr( true ) 
            {
               // This is debugging information, make it false if you 
               // don't want to see it. 

               file << ind << "std::cout << \"state Q" << i << ", next ";
               file << Cname << " is \" << c << \"\\n\";";
               file << "\n";
            }

            auto p = getsingletons( cl.delta[i].noneps );
            singleton_iforswitch( state(i), p. first, 
                                  ind, file, printC, printT ); 
            binary_if( state(i), 
                       p. second. begin(),
                       p. second. end(), ind, file, printC, printT );

         }
         else
            file << ind << "return latest;\n"; 
      }

      ind -= 3;
      file << ind << "}\n\n";
   }


}

#endif

