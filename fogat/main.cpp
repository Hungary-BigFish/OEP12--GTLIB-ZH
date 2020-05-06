#include <iostream>
#include "../library/summation.hpp"
#include "../library/maxsearch.hpp"
#include "../library/enumerator.hpp"
#include "../library/seqinfileenumerator.hpp"
#include "../library/stringstreamenumerator.hpp"

using namespace std;

struct Time {
	int h;
	int m;
	Time( int _h = 0, int _m = 0) : h(_h), m(_m) {}

	friend istream& operator >>( istream& is, Time &t ) {
		char colon;
		is >> t.h >> colon >> t.m;
		return is;
	}

	friend ostream& operator <<( ostream &os, const Time &t ) {
		os << t.h << ':' << t.m;
		return os;
	}
	friend Time operator+( const Time &a, const Time &b ) {
		int min = a.m + b.m;
		return Time( a.h + b.h + min / 60, min % 60 );
	}
	friend bool operator<( const Time &a, const Time &b ) {
		return (a.h == b.h)?(a.m < b.m):(a.h < b.h);
	}
};

struct Round{
	Time t;
	bool fell_out;
	Round( Time _t, bool c ) : t(_t), fell_out(c) {}

	Round() : t(Time()), fell_out(false) {}

	friend istream& operator >>( istream& is, Round &r ) {
		char c;
		is >> r.t >> c;
		r.fell_out = c == 'K';
		return is;
	}
	friend ostream& operator <<( ostream& os, const Round &r ) {
		os << r.t << (r.fell_out?" fell out":" stayed in");
		return os;
	}
	friend Round operator +( const Round &a, const Round &b ) {
		return Round( a.t + b.t, a.fell_out || b.fell_out );
	}
};

class RoundSum : public Summation<Round> {
    Round func(const Round& e) const override {
		return e;
	}
    Round neutral() const override {
		return Round();
	}
    Round add(const Round& a, const Round& b) const override {
		return a + b;
	}
};

struct Line {
	string name;
	string comp;
	Round total;
	Line( string _name = "", string _comp = "", Round _total = Round() ) : name(_name), comp(_comp), total(_total) {
		total.fell_out = true;
	}
	friend istream& operator >>( istream& is, Line &l ) {
		is >> l.name >> l.comp;
		is.get();
		string line;
		getline( is, line );
		stringstream ss( line );
		StringStreamEnumerator<Round> sse( ss );
		RoundSum rs;
		rs.addEnumerator( &sse );
		rs.run();
		l.total = rs.result();
		return is;
	}
	friend ostream& operator <<( ostream& os, const Line& l ) {
		os << l.name << " " << l.comp << " " << l.total;
		return os;
	}
	friend bool operator <( const Line& a, const Line& b ) {
		if( a.total.fell_out ) {
			return true;
		}
		if( b.total.fell_out ) {
			return false;
		}
		return b.total.t < a.total.t;
	}
};

class LineSum : public Summation<Line> {
private:
	string curr_name;
public:
	LineSum( const string& competition ) : curr_name( competition ) {}
	Line func(const Line& e) const override {
		return e;
	}
	Line neutral() const override {
		return Line();
	}
	Line add( const Line& a, const Line& b) const override {
		return (a < b)?b:a;
	}
	void first() override {}
    bool whileCond(const Line& current) const override {
		return curr_name == current.comp;
	}
};

class WinnerEnum : public Enumerator<string> {
private:
	string curr;
	bool the_end;
	SeqInFileEnumerator<Line> *x;
public:
	WinnerEnum( const string &file_name ) {
		x = new SeqInFileEnumerator<Line>(file_name);
	}
	void first() override {
		x -> first();
		next();
	}
	void next() override {
		if( x -> end() ){
			the_end = true;
		}
		else {
			the_end = false;
			LineSum ls((x->current()).comp);
			ls.addEnumerator(x);
			ls.run();
			curr = (ls.result().total.fell_out?"No winner":ls.result().name);
			curr += '\n';
		}
	}
	bool end() const override {
		return the_end;
	}
	string current() const override {
		return curr;
	}
};

class Final : public Summation<string> {
    string func(const string& e) const override {
		return e;
	}
    string neutral() const override {
		return "";
	}
    string add( const string& a, const string& b) const override {
		return a + b;
	}
};

void safe_run( const string &file_name ) {
	try{
		WinnerEnum we( file_name );
		Final f;
		f.addEnumerator( &we );
		f.run();
		cout << f.result();
	}
	catch ( SeqInFileEnumerator<Line>::Exceptions &e ) {
		cout << "Error: File not found" << endl;
	}
}

int main() {


	safe_run("fogathajtó.txt");
	return 0;
}

