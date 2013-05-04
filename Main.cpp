#include "BTree.h"
#include <stdlib.h>
#include <time.h>

int main(){
	srand(time(NULL));
	for (int r = 0; r < 1000; r++) {
		BTree<int> *bt = new BTree<int>;
		int add = 0;
		for (int i = 0; i < 6; i++) {
			add = (rand() % 256);
			bt->add(add);
			bt->root->print();
			cout << "member " << add << "?\n" << endl;
			if (bt->member(add)){
				cout << "yes" << endl;
			} else {
				cout << "no" << endl;
				return -1;
			}
		}
		cout << "############################################" << endl;
		cout << endl << endl << endl << endl;
		delete bt;
		bt = 0;
	}
	//bt.add(54);
	//cout << "test" << endl;
	return 0;
}
