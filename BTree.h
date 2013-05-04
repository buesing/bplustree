#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "Container.h"

using namespace std;

class BTreeEmptyException;
class BTreeInternalErrorException;

const int order = 2;
const int maxData = order*2+1;
const int maxKeys = order*2;

template <typename E> 
class BTree : public Container<E> {

	// SUBCLASS NODE
	class Node {
		public:
			virtual ~Node() {}
			inline virtual bool isLeafNode() = 0;
			virtual void print(int depth = 0) = 0;
	};
	class InnerNode : public Node {
		public:
			E keys[maxKeys];
			Node *children[maxData];
			int sizeChildren;
			int sizeKeys;
			inline bool isLeafNode() {
				return false;
			}
			InnerNode(){ 
				sizeChildren = 0;
				sizeKeys = 0;
			}
			~InnerNode(){
				cerr << "[~] destructing Inner node..." << endl;
				delete[] children;
			}
			void print(int depth = 0) {
				cout << endl;
				cout << std::string(depth, '\t') << "[print] Inner Node" << endl;
				cout << std::string(depth, '\t') << "[print] sizeKeys: " << sizeKeys << endl;
				cout << std::string(depth, '\t') << "[print] sizeChildren: " << sizeChildren << endl;
				cout << std::string(depth, '\t') << "[print] keys: ";
				for (int i = 0; i < sizeKeys; i++) {
					std::cout << keys[i] << " ";
				}
				cout << endl << endl << std::string(depth, '\t') << "--------- children: ---------" << endl;
				for (int i = 0; i < sizeChildren; i++) {
					children[i]->print(depth + 1);
				}
				cout << endl;
			}
	};

	class LeafNode : public Node {
		public:
			E data[order*2+1];
			Node *next;
			int size;
			LeafNode() {
			}
			~LeafNode() {
				cerr << "[~] destructing Leaf node..." << endl;
			}	
			bool addData(const E &e) {
				if (size >= order*2+1) {
					return false;
				} else {
					int i = 0;
					while (e > data[i] && i < size) {
						i++;
					}
					for (int s = size; s > i; s--) {
						data[s] = data[s-1];
					}
					data[i] = e;
					size++;
					return true;
				}	
			}
			inline bool isLeafNode() {
				return true;
			}
			void print(int depth = 0) {
				cout << endl;
				cout << std::string(depth, '\t') << "[print] Leaf Node" << endl;
				cout << std::string(depth, '\t') << "[print] size: " << size << endl;
				cout << std::string(depth, '\t') << "[print] data: ";
				for (int i = 0; i < size; i++) {
					std::cout << data[i] << " ";
				}
				cout << endl << endl;
			}
	};
	// END SUBCLASS NODE
	
	public:
		BTree();
		virtual ~BTree();

		virtual void add(const E& e);
		virtual void add(const E e[], size_t s);
		virtual void remove(const E& e);
		virtual void remove(const E e[], size_t s);
		virtual bool member(const E& e ) const;
		virtual size_t size() const;
		virtual bool empty() const;
		virtual size_t apply( const Functor<E>& f, Order order) const;
		virtual E min() const;
		virtual E max() const;
		virtual std::ostream& print(std::ostream &o) const;
		/*private:*/
		Node *root;
};

class BTreeEmptyException : public ContainerException {
	public:
		virtual const char * what() const throw() { return "Empty BTree"; }
};

class BTreeInternalErrorException : public ContainerException {
	const char * w;
	public:
	explicit BTreeInternalErrorException( const char * w ) throw() : w(w) {}
	virtual const char * what() const throw() { return w; }
};

template <typename E> 
BTree<E>::BTree(){
	root = new LeafNode;
}

/*template <typename E> */
/*BTree<E>::BTree(int k = 2){*/
/*root = new Node;*/
/*}*/

template <typename E> 
BTree<E>::~BTree(){
	cerr << "[~] calling destructor..." << endl;
	delete root;
}

template <typename E> 
void BTree<E>::add(const E& e) {
	cerr << "[add] adding " << e;
	if (root->isLeafNode()) {
		LeafNode *newRoot = static_cast<LeafNode*>(root);
		if (newRoot->addData(e)) {
			cerr << " into root (leaf) node..." << endl;;
			// Root Node is not full, easiest case
			root = newRoot;
			return;
		} else {
			cerr << ", splitting nodes... " << endl;
			// Root Node is full
			cerr << "[add] creating new nodes..." << endl;
			LeafNode *right = new LeafNode;
			LeafNode *left = new LeafNode;
			cerr << "[add] populating new nodes..." << endl;
			int stop = e > newRoot->data[order+1] ? order+1 : order;
			for (int i = 0; i < stop; i++) {
				left->data[i] = newRoot->data[i];
				left->size++;
			}
			for (int i = stop; i < order*2+1; i++) {
				right->data[i-stop] = newRoot->data[i];
				right->size++;
			}
			if (right->data[0] > e) {
				left->addData(e);
			} else {
				right->addData(e);
			}
			cerr << "[add] connecting new nodes to new root..." << endl;
			InnerNode *newRoot = new InnerNode;
			newRoot->children[0] = left;
			newRoot->children[1] = right;
			newRoot->keys[0] = right->data[0];
			newRoot->sizeChildren = 2;
			newRoot->sizeKeys = 1;
			cerr << "[add] deleting old root..." << endl;
			delete root;
			root = newRoot;
			cerr << "[add] done, returning" << endl;
			return;
		}
	}
	// look if value already exists
	cerr << " into tree, searching for value..." << endl;
	cerr << "ERROR: no case defined yet, undefined behaviour" << endl;
}

template <typename E>
void BTree<E>::add(const E e[], size_t s) {
	for (int i = 0; i < s; i++) {
		add(e[i]);
	}
	
}

template <typename E> 
void BTree<E>::remove(const E& e){
	return;
}

template <typename E> 
void BTree<E>::remove(const E e[], size_t s){

}

template <typename E> 
bool BTree<E>::member(const E& e ) const{
	Node *current = root;
	cerr << endl;
	while (!current->isLeafNode()) {
		InnerNode* temp = static_cast<InnerNode*>(current);
		int next = 0;
		while (e > temp->keys[next] and next < temp->sizeKeys) {
			next++;
		}
		current = temp->children[next];
	}
	LeafNode* temp = static_cast<LeafNode*>(current);
	for (int i = 0; i < temp->size; i++) {
		cerr << "[member] " << temp->data[i] << "==" << e << "?\n";
		if (temp->data[i] == e) {
			return true;
		}
	}
	return false;
}

template <typename E> 
size_t BTree<E>::size() const{
	return 0;
}

template <typename E> 
bool BTree<E>::empty() const{
	return false;
}

template <typename E> 
size_t BTree<E>::apply( const Functor<E>& f, Order order = dontcare ) const{
	return 0;
}

template <typename E> 
E BTree<E>::min() const{
	return 0;
}

template <typename E>
E BTree<E>::max() const{
	return 0;
}

template <typename E>
std::ostream& BTree<E>::print(std::ostream &o) const{
}

#endif //BINTREE_H
