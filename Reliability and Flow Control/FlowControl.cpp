/*
	Flow Control Example
	From "Networking for Game Programmers" - http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Net.h"

using namespace std;
using namespace net;

class FlowControl
{
public:
	
	FlowControl()
	{
		printf( "flow control initialized\n" );
		Reset();
	}
	
	void Reset()
	{
		mode = Bad;
		penalty_time = 4.0f;
		good_conditions_time = 0.0f;
		penalty_reduction_accumulator = 0.0f;
	}
	
	void Update( float deltaTime, float rtt )
	{
		const float RTT_Threshold = 250.0f;

		if ( mode == Good )
		{
			if ( rtt > RTT_Threshold )
			{
				printf( "bad mode (rtt = %.2f)\n", rtt );
				mode = Bad;
				if ( good_conditions_time < 10.0f && penalty_time < 60.0f )
				{
					penalty_time *= 2.0f;
					printf( "penalty time increased to %.1f\n", penalty_time );
				}
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				return;
			}
			
			good_conditions_time += deltaTime;
			penalty_reduction_accumulator += deltaTime;
			
			if ( penalty_reduction_accumulator > 10.0f && penalty_time > 1.0f )
			{
				penalty_time /= 2.0f;
				printf( "penalty time reduced to %.1f\n", penalty_time );
				penalty_reduction_accumulator = 0.0f;
			}
		}
		
		if ( mode == Bad )
		{
			if ( rtt <= RTT_Threshold )
				good_conditions_time += deltaTime;
			else
				good_conditions_time = 0.0f;
				
			if ( good_conditions_time > penalty_time )
			{
				printf( "good mode (rtt = %.2f)\n", rtt );
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				mode = Good;
				return;
			}
		}
	}
	
	float GetSendRate()
	{
		return mode == Good ? 30.0f : 10.0f;
	}
	
private:

	enum Mode
	{
		Good,
		Bad
	};

	Mode mode;
	float penalty_time;
	float good_conditions_time;
	float penalty_reduction_accumulator;
};

// ----------------------------------------------

const int ServerPort = 30000;
const int ClientPort = 30001;
const int ProtocolId = 0x11223344;
const float DeltaTime = 1.0f / 30.0f;
const float SendRate = 1.0 / 30.0f;
const float TimeOut = 10.0f;
const int PacketSize = 256;

int main( int argc, char * argv[] )
{
	// parse command line

	enum Mode
	{
		Client,
		Server
	};

	Mode mode = Server;
	Address address;

	if ( argc >= 2 )
	{
		int a,b,c,d;
		if ( sscanf( argv[1], "%d.%d.%d.%d", &a, &b, &c, &d ) )
		{
			mode = Client;
			address = Address(a,b,c,d,ServerPort);
		}
	}

	// initialize

	if ( !InitializeSockets() )
	{
		printf( "failed to initialize sockets\n" );
		return 1;
	}

	ReliableConnection connection( ProtocolId, TimeOut );

	const int port = mode == Server ? ServerPort : ClientPort;
	
	if ( !connection.Start( port ) )
	{
		printf( "could not start connection on port %d\n", port );
		return 1;
	}
	
	if ( mode == Client )
		connection.Connect( Address(127,0,0,1,ServerPort ) );
	else
		connection.Listen();
	
	FlowControl flowControl;

	bool connected = false;
	float sendAccumulator = 0.0f;
	
	while ( true )
	{
		// update flow control
		
		if ( connection.IsConnected() )
			flowControl.Update( DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f );
		
		const float sendRate = flowControl.GetSendRate();
		
		// send and receive packets
		
		unsigned char packet[PacketSize];

		sendAccumulator += DeltaTime;		
		while ( sendAccumulator >= 1.0f / sendRate )
		{
			connection.SendPacket( packet, sizeof( packet ) );
			sendAccumulator -= 1.0f / sendRate;
		}
		
		while ( true )
		{
			unsigned char packet[256];
			int bytes_read = connection.ReceivePacket( packet, sizeof(packet) );
			if ( bytes_read == 0 )
				break;
		}

		// handle changes in connection state

		if ( mode == Server )
		{
			if ( connected && !connection.IsConnected() && mode == Server )
			{
				printf( "flow control reset\n" );
				flowControl.Reset();
				connected = false;
			}
		}
		else
		{
			if ( !connected && connection.IsConnected() )
			{
				printf( "client connected to server\n" );
				connected = true;
			}
		
			if ( !connected && connection.ConnectFailed() )
			{
				printf( "connection failed\n" );
				break;
			}
		}
		
		// update connection
		
		connection.Update( DeltaTime );

		wait( DeltaTime );
	}
	
	ShutdownSockets();

	return 0;
}
