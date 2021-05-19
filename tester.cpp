#include <iostream>
#include <fstream>
#include "filereader.h"
#include "stringreader.h"

#include "algorithms.h"
#include "minimization.h"

#include "generators.h"
#include "deterministic.h"
#include "trivial.h"

enum tclass 
{
   tc_error = 0, 
   tc_float = 1,
   tc_ident = 2,
   tc_add = 10, tc_sub = 11, tc_mul = 12, tc_div = 13, tc_mod = 14,
   tc_lpar = 20, tc_rpar = 21,
   tc_semicolon = 30,
   tc_whitespace = 40 
};


lexing::classifier< char, tclass > calculator( )
{
   using namespace lexing;

   classifier< char, tclass > cl( tc_error ); 

   auto idfirst = range('a', 'z') | range('A', 'Z');
   auto idnext = range('a', 'z') | range('A', 'Z') |
                 just('_') | range('0', '9');


   auto number = (just('0') |
               (range('1', '9') * range('0', '9').star()));

   cl. insert( idfirst * idnext.star(), tc_ident );

   auto exp = ( just( 'e' ) | just( 'E' )) *
                 ( just( '-' ) | just( '+' )). optional( ) *
                 ( range( '0', '9' ). optional( ) *
                   range( '0', '9' ). optional( ) *
                   range( '0', '9' ). optional( ));

   auto floatconst = number *
               ( just('.') * ( range('0', '9').plus()) ). optional( ) *
                 exp. optional( );

   cl. insert( floatconst, tc_float );

   cl. insert( just( '+' ), tc_add );
   cl. insert( just( '-' ), tc_sub );
   cl. insert( just( '*' ), tc_mul );
   cl. insert( just( '/' ), tc_div );
   cl. insert( just( '*' ), tc_mod );

   cl. insert( just( '(' ), tc_lpar );
   cl. insert( just( ')' ), tc_rpar );
   
   cl. insert( just( ';' ), tc_semicolon );

   cl. insert( just( ' ' ) | just( '\t' ), tc_whitespace );

   auto linecomment = word("//") *
                    ( every<char>(). without('\n') ).star() * just('\n');
   cl. insert( linecomment, tc_whitespace ); 

   auto blockcomment = word("/*") *
          ( every<char>().without('*') |
            (just('*').plus() * every<char>().without('/').without('*')).star()).star()
             * just('*').plus( ) * just('/');
   cl. insert(blockcomment, tc_whitespace );

   std::cout << cl << "\n";

   cl = make_deterministic(cl);
   cl = minimize(cl);

   std::ofstream file( "code.h" );
   lexing::printcode<char,tclass> (
   "char", "tclass", "",
    cl, file,
    []( std::ostream& out, char c ) { out << (int)c; },
    []( std::ostream& out, const tclass t ) { out << "((tclass) " << t << ")"; } );
   file. close( );
   return cl; 
}

lexing::classifier< char, std::string > capitals( )
{
   using namespace lexing;

   auto cls = classifier<char,std::string> ( "#error" );

   auto idfirst = range('a', 'z') | range('A', 'Z');
   auto idnext = range('a', 'z') | range('A', 'Z') |
                 just('_') | range('0', '9');

   cls.insert( idfirst * idnext.star(), "#ident");

#if 1
   auto number = just('0') | (range('1', '9') * range('0', '9').star());
   cls.insert(number, "#number");
#endif

   cls.insert( ( just(' ') | just( '\n' ) | just( '\n' )). plus( ),
               "#whitespace" );

   cls.insert( word("warszawa"), "#warszawa");
   cls.insert( word("nursultan"), "#nursultan");
   cls.insert( word("amsterdam"), "#amsterdam");
   cls.insert( word( "brazilia" ), "#brazilia" );

   cls. insert( word( "end" ), "#end" ); 

   cls = minimize( make_deterministic(cls));

   std::cout << cls << "\n";

   filereader read( &std::cin, "stdin" );

   std::ofstream file( "code.h" );

   printdeterministic<char, std::string> (
      "char", "std::string", "readandclassify",
      cls, file,
      []( std::ostream& out, char c ) { out << (int)c; },
      []( std::ostream& out, const std::string& s ) { out << "\"" << s << "\""; } 
      );

   file. close( );

   return cls;  
}


lexing::classifier< char, std::string > cflatlanguage( )
{
   using namespace lexing;

   classifier< char, std::string > cls("#error");

   auto idfirst = range('a', 'z') | range('A', 'Z');
   auto idnext = range('a', 'z') | range('A', 'Z') |
		just('_') | range('0', '9');

   cls.insert( idfirst * idnext.star(), "#identifier");

   cls.insert( word("void"), "#void");
   cls.insert( word("bool"), "#bool");
   cls.insert( word("char"), "#char");
   cls.insert( word("integer"), "#integer");
   cls.insert( word("double"), "#double");
   cls.insert( word("pointer"), "#pointer");
   cls.insert( word("array"), "#array");

   cls.insert( word("if"), "#if");
   cls.insert( word("then"), "#then");
   cls.insert( word("else"), "#else");
   cls.insert( word("while"), "#while");
   cls.insert( word("do"), "#do");
   cls.insert( word("print"), "#print");
   cls.insert( word("newline"), "#newline");
   cls.insert( word("trace"), "#trace");
   cls.insert( word("return"), "#return");

	cls.insert( word("function"), "#function");
	cls.insert( word("structdef"), "#structdef");
	cls.insert( word("constant"), "#constant");

	cls.insert( word("begin"), "#begin");
	cls.insert( word("end"), "#end");

	cls.insert( word("("), "#lpar");
	cls.insert( word(")"), "#rpar");
	cls.insert( word("["), "#lsqpar");
	cls.insert( word("]"), "#rsqpar");
	cls.insert( word(","), "#comma");
	cls.insert( word("."), "#dot");
	cls.insert( word("->"), "#arrow");

	cls.insert( word(":"), "#colon");
	cls.insert( word(";"), "#semicolon");

	cls.insert( word("="), "#assign");
	cls.insert( word("?"), "#question");
	cls.insert( word("||"), "#or");
	cls.insert( word("&&"), "#and");
	cls.insert( word("!"), "#not");

	cls.insert( word("=="), "#eq");
	cls.insert( word("!="), "#ne");
	cls.insert( word("<"), "#lt");
	cls.insert( word(">"), "#gt");
	cls.insert( word("<="), "#le");
	cls.insert( word(">="), "#ge");

	cls.insert( word("+"), "#add");
	cls.insert( word("-"), "#sub");
	cls.insert( word("*"), "#mul");
	cls.insert( word("/"), "#truediv");
	cls.insert( word("%"), "#mod");
 
 	cls.insert( word("&"), "#ampersand");
	cls.insert( word("++"), "#plusplus");
	cls.insert( word("--"), "#minusminus");

	cls.insert( word("false"), "#false");
	cls.insert( word("true"), "#true");
	cls.insert( word("null"), "#null");

	auto number = (just('0') | 
		(range('1', '9') * range('0', '9').star()));

        auto exp = ( just( 'e' ) | just( 'E' )) * 
                   ( just( '-' ) | just( '+' )). optional( ) *
                   ( range( '0', '9' ). optional( ) *
                     range( '0', '9' ). optional( ) *
                     range( '0', '9' ). optional( ));

	auto doubleconst = number * 
                           ( just('.') * (range('0', '9').plus())). optional( ) *
                           exp. optional( ); 
        
	cls.insert( doubleconst, "#doubleconst");

	auto integerconst = just('0') | 
		(just('-').optional() * range('1', '9') * range('0', '9').star());

	cls.insert(integerconst, "#integerconst");


	auto charconst = just('\'') * 
                 every<char>().without('\\').without('\'') * just('\'');
	cls.insert(charconst, "#charconst");

	auto specialsym = just('\'') * just('\\') * 
		(just( '0' ) | just('\'') | just('\"') | just('t') | just('n')) * 
		just('\'');
	cls.insert(specialsym, "#charconst");

	auto linecomment = word("//") * 
                              ( every<char>().without('\n') ).star() * just('\n');
	cls.insert( linecomment, "#comment");

	auto blockcomment = word("/*") * 
                ( every<char>().without('*') | 
		(just('*').plus() * every<char>().without('/').without('*')).star()).star()
                 * just('*').plus( ) * just('/');
	cls.insert(blockcomment, "#comment");

	auto lineterminator = just('\r') | just('\n') | (just('\r') * just('\n'));
	auto whitespace = lineterminator | ( just( ' ' ) | just('\t') | just('\f') );
	cls.insert(whitespace, "#whitespace");	

	auto quotedstring = just('\"') * ( every<char>().without('\"').without('\\') | 
		just('\n') | 
		just('\t') |
		just('\\') * just('\\') ).star() * just('\"');
	cls.insert(quotedstring, "#stringconst");

	auto hashtag = just('#');
	cls.insert(hashtag, "#eof");

	std::cout << cls << "\n";
	auto det = make_deterministic(cls);
	std::cout << det << "\n";
        det = minimize( det ); 

        std::ofstream file( "code.h" );

       printdeterministic<char, std::string> (
      "char", "std::string", "readandclassify",
      det, file,
      []( std::ostream& out, char c ) { out << (int)c; },
      []( std::ostream& out, const std::string& s ) { out << "\"" << s << "\""; }
      );

//      printcode<char, std::string> (
//      "char", "std::string", "filereader", "readandclassify",
//      det, file,
//      []( std::ostream& out, char c ) { out << (int)c; },
//      []( std::ostream& out, const std::string& s ) { out << "\"" << s << "\""; }
//      );

   file. close( );

   return det;
}



int main( int argc, char* argv[] )
{

   using namespace lexing;

   {
      acceptor<char> reg = (just('a') | just('b') ).star( ) |
                           just('a'). star( ); 
      reg *= just( ';');

      std::cout << reg << "\n";

      classifier< char, std::string > cl( "#error" ); 
      cl. insert( reg, "#ourlang" ); 
      std::cout << cl << "\n";
      cl = make_deterministic(cl);
      std::cout << cl << "\n";

      cl = minimize(cl); 
      std::cout << cl << "\n";
   }

   return 0;
#if 0
   auto cls = cflatdeterministic( ); 
   std::cout << cls << "\n";

   filereader read( &std::cin, "stdin" );

   constexpr bool thestarsareshiningandtheworldkeepsturning = true; 
   while( thestarsareshiningandtheworldkeepsturning ) 
   {
      if( !read. has(1) )
      {
         std::cout << "end of input\n";
	 return 0;
      }

      std::pair< std::string, size_t > p = readandclassify( cls, read );

      if( p. first != "#whitespace" )    
      {
         std::cout << "classification = " << p.first << "/" << p.second;
         std::cout << "    " << read. view( p . second ) << "\n";
      }   

      if( p. first == "#eof" ) 
      {
         std::cout << "end of file reached in line " << read. line << "\n";
         return 0;
      }

      if( p. second == 0 )
         read. commit(1);
      else 
         read. commit( p. second );
   }
#endif 
}


