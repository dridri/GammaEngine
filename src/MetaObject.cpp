#include <stdlib.h>
#include <sstream>
#include "MetaObject.h"

const bool MetaObject::meta( const std::string& name, const bool def ) const
{
	if ( mMeta.count( name ) > 0 ) {
		return ( mMeta.at( name ) == "true" );
	}
	return def;
}


const int MetaObject::meta( const std::string& name, const int def ) const
{
	if ( mMeta.count( name ) > 0 ) {
		return atoi( mMeta.at( name ).c_str() );
	}
	return def;
}


const float MetaObject::meta( const std::string& name, const float def ) const
{
	if ( mMeta.count( name ) > 0 ) {
		return atof( mMeta.at( name ).c_str() );
	}
	return def;
}


const std::string& MetaObject::meta( const std::string& name, const std::string& def ) const
{
	if ( mMeta.count( name ) > 0 ) {
		return mMeta.at( name );
	}
	return def;
}


void MetaObject::setMeta( const std::string& name, const bool v )
{
	mMeta[ name ] = ( v ? "true" : "false" );
}


void MetaObject::setMeta( const std::string& name, const int v )
{
	std::stringstream ss;
	ss << v;
	mMeta[ name ] = ss.str();
}


void MetaObject::setMeta( const std::string& name, const float v )
{
	std::stringstream ss;
	ss << v;
	mMeta[ name ] = ss.str();
}


void MetaObject::setMeta( const std::string& name, const std::string& v )
{
	mMeta[ name ] = v;
}
