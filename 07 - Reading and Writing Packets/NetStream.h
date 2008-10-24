/*
	Simple Network Library from "Networking for Game Programmers"
	http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#ifndef NET_STREAM_H
#define NET_STREAM_H

#include <assert.h>
#include <algorithm>

namespace net
{
	// bitpacker class
	//  + read and write non-8 multiples of bits efficiently
	
	class BitPacker
	{
	public:
		
		enum Mode
		{
			Read,
			Write
		};
		
		BitPacker( Mode mode, void * buffer, unsigned int bytes )
		{
			assert( buffer );
			assert( bytes >= 4 );
			assert( ( bytes & 3 ) == 0 );
			this->mode = mode;
			this->buffer = (unsigned char*) buffer;
			this->ptr = (unsigned char*) buffer;
			this->bytes = bytes;
			bit_index = 0;
		}
		
		bool WriteBits( unsigned int value, int bits = 32 )
		{
			assert( bits > 0 );
			assert( bits <= 32 );
			assert( mode == Write );
			assert( ptr - buffer < bytes );
			if ( bits < 32 )
			{
				const unsigned int mask = ( 1 << bits ) - 1;
				value &= mask;
			}
			do
			{
				*ptr |= (unsigned char) ( value << bit_index );
				assert( bit_index < 8 );
				const int bits_written = std::min( bits, 8 - bit_index );
				assert( bits_written > 0 );
				assert( bits_written <= 8 );
				bit_index += bits_written;
				if ( bit_index >= 8 )
				{
					ptr++;
					bit_index = 0;
					value >>= bits_written;
					assert( ptr - buffer < bytes );
				}
				bits -= bits_written;
				assert( bits >= 0 );
				assert( bits <= 32 );
			}
			while ( bits > 0 );
			return true;
		}
		
		int GetBitsWritten() const
		{
			assert( mode == Write );
			return ( ptr - buffer ) * 8 + bit_index;
		}
		
		bool ReadBits( unsigned int & value, int bits = 32 )
		{
			assert( bits > 0 );
			assert( bits <= 32 );
			assert( mode == Read );
			assert( ptr - buffer < bytes );
			int original_bits = bits;
			int value_index = 0;
			value = 0;
			do
			{
				assert( bits >= 0 );
				assert( bits <= 32 );
				int bits_to_read = std::min( 8 - bit_index, bits );
				assert( bits_to_read > 0 );
				assert( bits_to_read <= 8 );
				value |= ( *ptr >> bit_index ) << value_index;
				bits -= bits_to_read;
				bit_index += bits_to_read;
				value_index += bits_to_read;
				assert( value_index >= 0 );
				assert( value_index <= 32 );
				if ( bit_index >= 8 )
				{
					ptr++;
					bit_index = 0;
					assert( ptr - buffer < bytes );
				}
			}
			while ( bits > 0 );
			if ( original_bits < 32 )
			{
				const unsigned int mask = ( 1 << original_bits ) - 1;
				value &= mask;
			}
			return true;
		}
		
		int GetBitsRead() const
		{
			assert( mode == Read );
			return ( ptr - buffer ) * 8 + bit_index;
		}
		
		Mode GetMode() const
		{
			return mode;
		}
		
	private:
		
		int bit_index;
		unsigned char * ptr;
		unsigned char * buffer;
		int bytes;
		Mode mode;
	};
	
	// arithmetic coder
	//  + todo: implement arithmetic coder based on jon blow's game developer article
	
	class ArithmeticCoder
	{
	public:
		
		enum Mode
		{
			Read,
			Write
		};
		
		ArithmeticCoder( Mode mode, void * buffer, unsigned int size )
		{
			assert( buffer );
			assert( size >= 4 );
			assert( size & 3 == 0 );
			this->mode = mode;
			this->buffer = (unsigned char*) buffer;
			this->size = size;
		}

		bool WriteInteger( unsigned int value, unsigned int minimum = 0, unsigned int maximum = 0xFFFFFFFF )
		{
			// ...
			return false;
		}
		
		bool ReadInteger( unsigned int value, unsigned int minimum = 0, unsigned int maximum = 0xFFFFFFFF )
		{
			// ...
			return false;
		}
		
	private:
		
		unsigned char * buffer;
		int size;
		Mode mode;
	};
	
	// stream class
	//  + unifies read and write into a serialize operation
	//  + provides attribution of stream for debugging purposes
	
	class Stream
	{
		// ...
	};
}

#endif
