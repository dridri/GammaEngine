#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

int main( int ac, char** av )
{
	ifstream f( av[1] );
	string line;
	double x, y, z;

	double scale = ( ac > 2 ) ? atof(av[2]) : 1.0;

	while ( getline( f, line ) ) {
		if ( line[0] == 'v' and line[1] == ' ' ) {
			istringstream ss( line.substr(2) );
			ss >> setprecision(6) >> x;
			ss >> setprecision(6) >> z;
			ss >> setprecision(6) >> y;
			x *= scale;
			y *= scale;
			z *= scale;
			cout << fixed << setprecision(6) << "v " << x << " " << y << " " << z << "\n";
		} else if ( line[0] == 'v' and line[1] == 'n' and line[2] == ' ' ) {
			istringstream ss( line.substr(3) );
			ss >> setprecision(6) >> x;
			ss >> setprecision(6) >> z;
			ss >> setprecision(6) >> y;
			x *= scale;
			y *= scale;
			z *= scale;
			cout << fixed << setprecision(6) << "vn " << x << " " << y << " " << z << "\n";
		} else if ( line[0] == 'f' and line[1] == ' ' ) {
			istringstream iss( line.substr(2) );
			string p1, p2, p3;
			getline( iss, p1, ' ' );
			getline( iss, p2, ' ' );
			getline( iss, p3, ' ' );
			cout << "f " << p1 << " " << p3 << " " << p2 << "\n";
		} else {
			cout << line << "\n";
		}
	}

	return 0;
}
