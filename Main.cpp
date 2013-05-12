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
	for (int r = 0; r < 1000; r++) {
		BTree<Wrapper> *bt = new BTree<Wrapper>();
		for (int i = 0; i < 1000; i++) {
			int a = rand() % 100;
			cout << i << endl << endl;
			Wrapper *add = new Wrapper(a);
			bt->add(*add);
			//bt->root->print();
			//cout << "member " << add << "?\n" << endl;
			//cout << r << endl;
			if (bt->member(*add)){
				//cout << "yes" << endl;
			} else {
				//cout << "no" << endl;
				return -1;
			}
			delete add;
			add = 0;
		}
		delete bt;
		cout << endl << "############################################" << endl << endl;
		bt = 0;
		cout << r << endl;
	}
	//bt.add(54);
	//cout << "test" << endl;
	return 0;
}
