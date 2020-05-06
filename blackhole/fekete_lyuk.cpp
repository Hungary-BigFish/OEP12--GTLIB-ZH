#include "../library/summation.hpp"
#include "../library/maxsearch.hpp"
#include "../library/stringstreamenumerator.hpp"
#include "../library/enumerator.hpp"
#include "../library/seqinfileenumerator.hpp"
#include <iostream>

using namespace std;

struct Date {
	int y;
	int m;
	int d;
	Date( int _y = 0, int _m = 0, int _d = 0 ) : y(_y), m(_m), d(_d) {} 
	friend bool operator<( const Date& b, const Date& a ) {
		if ( b.y == a.y ) {
			return (b.m == a.m)?(b.d < a.d):(b.m < a.m);
		}
		else { return b.y < a.y; }
	}
	friend istream& operator>>( istream& is, Date& a ) {
		char dot;
		is >> a.y >> dot >> a.m >> dot >> a.d;
		return is;
	}
};

struct Mesurement {
	Date dt;
	int mass;
	int distance;
	Mesurement( Date _date, int _mass, int _distance ) : dt(_date), mass(_mass), distance(_distance){}
	Mesurement() {
		Date d;
		dt = d;
		mass = 0;
		distance = 0;
	}
	friend istream& operator >>( istream& is, Mesurement &a ) {
		is >> a.dt >> a.mass >> a.distance;
		return is;
	}
};

class MesurementSum : public Summation<Mesurement, pair<Mesurement, bool>> {
	pair<Mesurement, bool> func(const Mesurement& e) const override {
		return make_pair(e, e.distance < 3);
	}
	pair<Mesurement, bool> neutral() const override {
		Mesurement ret;
		return make_pair(ret, false);
	}
	pair<Mesurement, bool> add( const pair<Mesurement, bool>& a, const pair<Mesurement, bool>& b) const override {
		return make_pair(((a.first.dt < b.first.dt)?b:a).first, a.second || b.second );
	}
};

struct Line {
	string id;
	string station;
	bool l;
	Mesurement last;
	friend istream& operator >>( istream& is, Line &a ) {
		is >> a.id >> a.station;
		string line;
		getline( is, line );
		stringstream ss( line );
		StringStreamEnumerator<Mesurement> sse = { ss };
		MesurementSum ms;
		ms.addEnumerator( &sse );
		ms.run();
		a.last = ms.result().first;
		a.l = ms.result().second;
		return is;
	}
};

struct BlackHole {
	string id;
	int mass_sum;
	int count;
	bool l;
	BlackHole( string _id = "", int mass = 0, int cnt = 0, bool _l = true ) : 
		id(_id), mass_sum(mass), count(cnt), l(_l) {}
};

class LineSum : public Summation<Line, BlackHole> {
private:
	string curr_id;
public:
	LineSum( string id ) : curr_id( id ) {}
	BlackHole func(const Line& e) const override {
		BlackHole ret = {e.id, e.last.mass, 1, e.l};
		return ret;
	}
    BlackHole neutral() const override {
		BlackHole ret;
		return ret;
	}
    BlackHole add( const BlackHole& a, const BlackHole& b) const override {
		BlackHole ret = { curr_id, a.mass_sum+b.mass_sum, a.count + b.count, a.l && b.l };
		return ret;
	}
	bool whileCond(const Line& current) const override { 
		return current.id == curr_id;
	}
	void first() override {}
};

class BlackHoleEnum : public Enumerator<BlackHole> {
private:
	SeqInFileEnumerator<Line> *x;
	BlackHole curr;
	bool the_end;
public:
	BlackHoleEnum( const string &fn ) {
		x = new SeqInFileEnumerator<Line>( fn );
	}
	~BlackHoleEnum(){
		delete x;
	}
	void first() override {
		x->first();
		next();
	}
	void next() override {
		if ( x -> end() ) {
			the_end = true;
		}
		else {
			the_end = false;
			LineSum ls = { x -> current().id };
			ls.addEnumerator(x);
			ls.run();
			curr = ls.result();
		}
	}
	bool end() const override {
		return the_end;
	}
	BlackHole current() const override {
		return curr;
	}
};

class Final : public MaxSearch<BlackHole, double> {
	double func(const BlackHole& e) const override {
		return 1.0 * e.mass_sum / e.count;
	}
	bool cond(const BlackHole& e) const override {
		return e.l;
	}
};

void safe_run( const string &file_name ) {
	try{
		Final f;
		BlackHoleEnum  bhe(file_name);
		f.addEnumerator( &bhe );
		f.run();
		cout << f.optElem().id << " " << f.opt() << endl;
	}
	catch ( SeqInFileEnumerator<Line>::Exceptions &e ) {
		cout << "Error: File not found" << endl;
	}
}

int main() {
	safe_run("fekete_lyuk.txt");
	return 0;
}
