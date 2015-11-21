#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <map>
#include <string>

class MetaObject
{
public:
	const bool meta( const std::string& name, const bool def ) const;
	const int meta( const std::string& name, const int def ) const;
	const float meta( const std::string& name, const float def ) const;
	const std::string& meta( const std::string& name, const std::string& def = "" ) const;

	void setMeta( const std::string& name, const bool v );
	void setMeta( const std::string& name, const int v );
	void setMeta( const std::string& name, const float v );
	void setMeta( const std::string& name, const std::string& v );

private:
	std::map< std::string, std::string > mMeta;
};

#endif // METAOBJECT_H
