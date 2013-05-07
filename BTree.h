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

	class InnerNode;
	// SUBCLASS NODE
	class Node {
		public:
			Node* parent;
			Node(Node* p): parent(p) {
				
			}
			virtual ~Node() {}
			virtual bool isLeafNode() = 0;
			virtual void print(int depth = 0) = 0;
	};
	class InnerNode : public Node {
		public:
			E keys[maxKeys];
			Node *children[maxData];
			int sizeChildren;
			int sizeKeys;
			bool isLeafNode() {
				return false;
			}
			InnerNode(Node* pt = 0): Node(pt){ 
				sizeChildren = 0;
				sizeKeys = 0;
			}
			~InnerNode(){
				cerr << "[~] destructing Inner node..." << endl;
				for (int i = 0; i < sizeChildren; i++) {
					delete children[i];
				}
				sizeChildren = 0;
				sizeKeys = 0;
				/*children = 0;*/
			}
			void print(int depth = 0) {
				cerr << std::string(depth*2, '-') << "[print] Inner Node" << endl;
				cerr << std::string(depth*2, '-') << "[print] sizeKeys: " << sizeKeys << endl;
				cerr << std::string(depth*2, '-') << "[print] sizeChildren: " << sizeChildren << endl;
				cerr << std::string(depth*2, '-') << "[print] keys: ";
				for (int i = 0; i < sizeKeys; i++) {
					std::cerr << keys[i] << " ";
				}
				for (int i = 0; i < sizeChildren; i++) {
					children[i]->print(depth + 1);
				}
				cerr << endl;
			}
	};

	class LeafNode : public Node {
		public:
			E data[order*2+1];
			Node *next;
			int size;
			LeafNode(Node *pt = 0): Node(pt) {
			}
			~LeafNode() {
				cerr << "[~] destructing Leaf node..." << endl;
				size = 0;
			}	
			bool addData(const E &e) {
				if (size >= order*2+1) {
					return false;
				} else {
					int i = 0;
					while (e > data[i] && i < size) {
						i++;
					}
					if (e == data[i]) {
						cerr << "[BTree::add] already in node" << endl;
						return true;
					}
					for (int s = size; s > i; s--) {
						data[s] = data[s-1];
					}
					data[i] = e;
					size++;
					return true;
				}	
			}
			bool isLeafNode() {
				return true;
			}
			void print(int depth = 0) {
				cerr << endl;
				cerr << string(depth*2, '-') << "[print] Leaf Node" << endl;
				cerr << string(depth*2, '-') << "[print] size: " << size << endl;
				cerr << string(depth*2, '-') << "[print] data: ";
				for (int i = 0; i < size; i++) {
					cerr << data[i] << " ";
				}
				cerr << endl;
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
	root = new LeafNode(0);
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
	cerr << "[BTree::add] adding " << e << endl;;
	if (root->isLeafNode()) {
		cerr << "[BTree::add] into root (leaf) node..." << endl;;
		LeafNode *newRoot = static_cast<LeafNode*>(root);
		if (newRoot->addData(e)) {
			// Root Node is not full, easiest case
			root = newRoot;
			return;
		} else {
			// Root Node is full
			// check if element already in node
			for (int i = 0; i < newRoot->size; i++) {
				if (e == newRoot->data[i])
					return;
			}
			cerr << "[BTree::add] splitting nodes... " << endl;
			cerr << "[BTree::add] creating new nodes..." << endl;
			LeafNode *right = new LeafNode();
			LeafNode *left = new LeafNode();
			cerr << "[BTree::add] populating new nodes..." << endl;
			int stop = e > newRoot->data[order] ? order+1 : order;
			for (int i = 0; i < stop; i++) {
				left->data[i] = newRoot->data[i];
				left->size++;
			}
			for (int i = stop; i < order*2+1; i++) {
				right->data[i-stop] = newRoot->data[i];
				right->size++;
			}
			stop == order ? left->addData(e) : right->addData(e);
			cerr << "[BTree::add] connecting new nodes to new root..." << endl;
			InnerNode *newRoot = new InnerNode(0);
			newRoot->children[0] = left;
			newRoot->children[1] = right;
			newRoot->keys[0] = right->data[0];
			newRoot->sizeChildren = 2;
			newRoot->sizeKeys = 1;
			delete root;
			root = newRoot;
			left->parent = root;
			right->parent = root;
			cerr << "[BTree::add] done, returning" << endl;
			return;
		}
	}

	Node *current = root;
	cerr << "[BTree::add] looking up appropriate node..." << endl;
	int parent_index = 0;
	while (!current->isLeafNode()) {
		parent_index = 0;
		InnerNode* temp = static_cast<InnerNode*>(current);
		while ((e > temp->keys[parent_index] or 
				e == temp->keys[parent_index]) and 
				parent_index < temp->sizeKeys) {
			parent_index++;
		}
		current = temp->children[parent_index];
	}
	// found node, check if element already in there
	LeafNode* temp = static_cast<LeafNode*>(current);
	for (int i = 0; i < temp->size; i++) {
		if (temp->data[i] == e) {
			return;
		}
	}
	cerr << "[BTree::add] node found, attempting to insert..." << endl;
	if (temp->addData(e)) {
		cerr << "[BTree::add] inserted, returning" << endl;
		return;
	}
	cerr << "[BTree::add] node full, splitting nodes..." << endl;
	// TODO smth wrong here
	LeafNode *right = new LeafNode(temp->parent);
	LeafNode *left = new LeafNode(temp->parent);
	int stop = e > temp->data[order] ? order+1 : order;
	for (int i = 0; i < stop; i++) {
		left->data[i] = temp->data[i];
		left->size++;
	}
	for (int i = stop; i < order*2+1; i++) {
		right->data[i-stop] = temp->data[i];
		right->size++;
	}
	InnerNode *curr = static_cast<InnerNode*>(temp->parent);
	stop == order ? left->addData(e) : right->addData(e);
	cerr << "[BTree::add] nodes split, attempting to insert new key into parent..." << endl;

	// here starts the recursive part
	/*curr->children[parent_index] = left;*/
	if (curr->sizeKeys < maxKeys) {
		for (int i = curr->sizeKeys; i > parent_index; i--) {
			curr->keys[i] = curr->keys[i-1];
		}
		curr->keys[parent_index+1] = right->data[0];
		curr->keys[parent_index] = left->data[0];
		curr->sizeKeys++;

		for (int i = curr->sizeChildren; i > parent_index; i--) {
			curr->children[i] = curr->children[i-1];
		}
		curr->children[parent_index+1] = right;
		curr->children[parent_index] = left;
		curr->sizeChildren++;
	} else {
	cerr << "[BTree::add] parent node full, splitting inner node..." << endl;
	}
	// /TODO 
}

template <typename E>
void BTree<E>::add(const E e[], size_t s) {
	for (unsigned i = 0; i < s; i++) {
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
	cerr << "[member] " << e << "?\n";
	Node *current = root;
	cerr << endl;
	while (!current->isLeafNode()) {
		InnerNode* temp = static_cast<InnerNode*>(current);
		int next = 0;
		while ((e > temp->keys[next] or e == temp->keys[next]) and next < temp->sizeKeys) {
			next++;
		}
		current = temp->children[next];
	}
	LeafNode* temp = static_cast<LeafNode*>(current);
	temp->print();
	for (int i = 0; i < temp->size; i++) {
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
	return o;
}

#endif //BINTREE_H
