#include "BTree.h"
#include <stdlib.h>
#include <time.h>

class Wrapper{
	public:
		int data;
		Wrapper(int d = -1){
			data = d;
		}
		~Wrapper(){
			data = 0;
		}
		bool operator>(const Wrapper t) const {
			return (data > t.data);
		}
		bool operator==(const Wrapper t) const {
			return (data == t.data);
		}
};

template <typename E>
class PrintN : public Functor<E> {
	std::ostream& o;
	mutable int n;
	public:
	explicit PrintN( int n = 0, std::ostream& o = std::cout ) : o( o ), n( n ) { }
	explicit PrintN( std::ostream& o ) : o( o ), n( 0 ) { }
	bool operator()( const E& e ) const {
		return n <= 0 || --n;
	}
};

ostream& operator<< (ostream &out, const Wrapper &w) {
	out << w.data;
	return out;
}

int main(){
	
	srand(time(NULL));
		for (int r = 0; r < 1; r++) {
		int add[100000]; 
		for (int i = 0; i < 100000; i++) {
			add[i] = rand() % 1000;
		}
		BTree<Wrapper> *bt = new BTree<Wrapper>(2);
		for (int i = 0; i < 40; i++) {
			cout << i << endl << endl;
			Wrapper *wr = new Wrapper(add[i]);
			bt->add(*wr);
			//bt->root->print();
			//cout << "member " << add << "?\n" << endl;
			//cout << r << endl;
			//if (bt->member(*wr)){
				////cout << "yes" << endl;
			//} else {
				//bt->print(cerr);
				////cout << "no" << endl;
				//return -1;
			//}
			//if (!bt->validate(bt->root)) {
				//cerr << "validation failed" << endl;
				//bt->print(cerr);
				//return -1;
			//} else {
				//cerr << "valid." << endl;
			//}
			//cerr << "min: " << bt->min() << endl;
			//cerr << "max: " << bt->max() << endl;
			//cerr << "size: " << bt->size() << endl;
		}
		//bt->print(cerr);
		for (int i = 0; i < 40; i++) {
			cout << i << endl << endl;
			cerr << "############### removing " << add[i] << endl;
			Wrapper *wr = new Wrapper(add[i]);
			bt->remove(*wr);
			bt->root->print();
			//cout << "member " << add << "?\n" << endl;
			//cout << r << endl;
			if (!bt->member(*wr)){
				//cout << "yes" << endl;
			} else {
				//cout << "no" << endl;
				return -1;
			}
		}
		//delete[] add;

		bt->print(cerr);
		cout << "size: " << bt->size() << endl;
		cout << "descending:" << endl;
		size_t rc = bt->apply( PrintN<Wrapper>( 600 ), descending );
		cout << rc << " == 100?" << endl;
		cout << "ascending:" << endl;
		rc = bt->apply( PrintN<Wrapper>( 10 ), ascending );
		
		//bt->print(cerr);
		//cout << bt->size() << endl;
		//delete bt;
		cout << endl << "############################################" << endl << endl;
		bt = 0;
		cout << r << endl;
	}
	//bt.add(54);
	//cout << "test" << endl;
	return 0;
}
