
#ifndef MAPH_STRINGREADER 
#define MAPH_STRINGREADER   1

#include <string>
#include <iostream>
#include <string_view>

// This is an inputsource that reads from a string.
// It has the same functionality as filereader. 

struct stringreader
{
   std::string buffer;
   size_t p;

   stringreader( const std::string& s )
      : buffer(s),
        p(0)
   { }

   stringreader( std::string&& s )
      : buffer(s),
        p(0)
   { }

   stringreader( const char* c )
      : buffer( std::string(c) ),
        p(0)
   { }

   char peek( size_t i ) const
      { return buffer[ p + i ]; }

   bool has( size_t len ) const
      { return p + len < buffer. size( ); }

   std::string_view view( size_t i ) const
      { return std::string_view( buffer.data( ) + p, i ); } 

   void commit( size_t len )
      { p += len; }
};

#endif


