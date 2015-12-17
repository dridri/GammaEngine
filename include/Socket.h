#ifndef GE_SOCKET_H
#define GE_SOCKET_H

#include <list>
#include <map>
#include "MetaObject.h"

namespace GE
{

class Socket : public MetaObject
{
public:
	typedef enum {
		TCP,
		UDP
	} PortType;

	Socket( const std::string& server = "", unsigned short port = 0, PortType type = TCP, int timeout = 0 );
	~Socket();

	bool isConnected() const { return mSocket > 0; }
	int Connect( const std::string& server, unsigned short port, PortType type = TCP, int timeout = 0 );
	int Send( const void* data, size_t size, int timeout = 0 );
	int Receive( void* data, size_t size, bool fixed_size = false, int timeout = 0 );

	int Send( const std::string& s, int timeout = 0 );
	int HTTPGet( const std::string& url, int timeout = 0 );
	int HTTPGet( const std::string& url, const std::map< std::string, std::string >& args = std::map< std::string, std::string >(), int timeout = 0 );
	std::string Receive( int timeout = 0 );
	std::string HTTPResponse( std::string* header = nullptr, int timeout = 0 );

	int rawSocket() const { return mSocket; }

private:
	typedef struct {
		uint8_t* p;
		size_t s;
	} Buffer;

	int mSocket;
	void* mSin;
	std::list< Buffer > mRecvQueue;
};

}

#endif // GE_SOCKET_H
