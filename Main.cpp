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

ostream& operator<< (ostream &out, const Wrapper &w) {
	out << w.data;
	return out;
}

int main(){
	
	srand(time(NULL));
		for (int r = 0; r < 1; r++) {
		int add[100000]; 
		for (int i = 0; i < 100000; i++) {
			add[i] = rand() % 10000000;
		}
		BTree<Wrapper> *bt = new BTree<Wrapper>();
		for (int i = 0; i < 100000; i++) {
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
		//for (int i = 0; i < 100; i++) {
			//cout << i << endl << endl;
			//Wrapper *wr = new Wrapper(add[i]);
			//bt->remove(*wr);
			////bt->root->print();
			////cout << "member " << add << "?\n" << endl;
			////cout << r << endl;
			//if (!bt->member(*wr)){
				////cout << "yes" << endl;
			//} else {
				////cout << "no" << endl;
				//return -1;
			//}
		//}
		//delete[] add;
		for (int i = 0; i < 100000; i++) {
			if (!bt->member(add[i])) {
				bt->print(cerr);
				cout << "error" << endl;
				return -1;
			}
		}
		
		bt->print(cerr);
		delete bt;
		cout << endl << "############################################" << endl << endl;
		bt = 0;
		cout << r << endl;
	}
	//bt.add(54);
	//cout << "test" << endl;
	return 0;
}
