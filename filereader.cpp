
// Written by Hans de Nivelle, 2021.

#include "filereader.h"
#include <utility>

filereader::filereader( filereader&& read )
   : in( std::exchange( read. in, nullptr )),
     filename( std::exchange( read. filename, std::string( ))),
     line( std::exchange( read. line, 0 )), 
     column( std::exchange( read. column, 0 )),
     buffer( std::exchange( read. buffer, std::string( )))
{ }

filereader& filereader::operator = ( filereader&& read )
{
   in = std::exchange( read. in, nullptr ); 
   filename = std::exchange( read. filename, std::string( )); 
   line = std::exchange( read. line, 0 );
   column = std::exchange( read. column, 0 ); 
   buffer = std::exchange( read. buffer, std::string( )); 
   return *this; 
}

char filereader::peek( size_t i ) const 
{
   if( i >= buffer. size( ))
      throw std::runtime_error( "filereader: peek outside of buffer" );
   return buffer[i];
}

std::string_view filereader::view( size_t i ) const
{
   if( i > buffer. size( ))
      throw std::runtime_error( "filereader: view outside of buffer" );

   return std::string_view( buffer.data( ), i );
}

bool filereader::has( size_t len )
{
   if( len > buffer. size( ))
   {
      if( !good( )) 
         return false; 

      while( len > buffer. size( ))
      {
         int c = in -> get( );
         if( !good( ))
            return false;
         buffer. push_back(c);
      }
   }
   return true;  
}

// In a real application, this operation would be unprotected:

void filereader::commit( size_t len ) 
{
   if( len > buffer. size( ))
      throw std::runtime_error( "filereader: commit beyond buffer" ); 

   for( size_t i = 0; i < len; ++ i ) 
   {
      if( buffer[i] == '\n' )
      {
         ++ line;
         column = 0; 
      }
      else
         ++ column;
   }

   buffer. erase( buffer. begin( ), buffer. begin( ) + len ); 
} 

namespace
{
   char hex( char c ) 
   {
      if( c >= 10 )
         return 'A' + ( c - 10 );
      else
         return '0' + c;
   }
}


void filereader::print( std::ostream& out ) const 
{
   if( in == nullptr )
   {
      out << "filereader( nofile )"; 
      return; 
   }

   out << "filereader( " << filename << ", " << line << ", " << 
           column << " ) : ";

   for( char c : buffer )
   {
      if( isprint(c)) 
         out << c; 
      else 
         out << "{" << hex( c >> 4 ) << hex( c & 0X0F ) << "}";
   }
   if( eof( ) ) 
      out << " (end of file)";
   else
   {
      if( !good( ))  
         out << "(file is not good)"; 
   }
   out << "\n";
}


