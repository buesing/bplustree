#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "Container.h"

using namespace std;

class BTreeEmptyException;
class BTreeInternalErrorException;

const int order = 1;
const int maxData = order*2+1;
const int maxKeys = order*2;

template <typename E> 
class BTree : public Container<E> {

	class InnerNode;
	class LeafNode;
	// SUBCLASS NODE
	class Node {
		public:
			Node* parent;
			Node(Node* p): parent(p) {
			}
			virtual void init() {
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
				cerr << "[InnerNode::~] destructing Inner node..." << endl;
				for (int i = 0; i < sizeChildren; i++) {
					delete children[i];
				}
				sizeChildren = 0;
				sizeKeys = 0;
				/*children = 0;*/
			}
			void init() {
				sizeChildren = 0;
				sizeKeys = 0;
			}
			bool insertLeafNode(LeafNode* n) {
				if (sizeChildren < maxData) {
					cerr << "[InnerNode::insertLeafNode] inserting... with key "<<  n->data[0] << endl;
					int at = 0;
					while (sizeKeys != at and n->data[0] > keys[at]) {
						at++;
					}
					for (int i = sizeKeys; i > at ; i--) {
						keys[i] = keys[i-1];
					}
					keys[at] = n->data[0];
					at++;
					for (int i = sizeChildren; i > at; i--) {
						children[i] = children[i-1];
					}
					children[at] = n;
					sizeKeys++;
					sizeChildren++;
					return true;
				} else {
					return false;
				}
			}
			bool insertInnerNode(InnerNode* n, E& key) {
				if (sizeChildren < maxData) {
					n->parent = this;
					cerr << "[InnerNode::insertInnerNode] inserting... with key" << key << endl;
					int at = 0;
					while (sizeKeys != at and !(keys[at] > key)) {
						at++;
					}
					for (int i = sizeKeys; i > at ; i--) {
						keys[i] = keys[i-1];
					}
					keys[at] = key;
					at++;
					for (int i = sizeChildren; i > at; i--) {
						children[i] = children[i-1];
					}
					children[at] = n;
					sizeKeys++;
					sizeChildren++;
					return true;
				} else {
					return false;
				}
			}
			void print(int depth = 0) {
				cerr << std::string(depth*6, '-') << "[print] Inner Node" << endl;
				cerr << std::string(depth*6, '-') << "[print] sizeKeys: " << sizeKeys << endl;
				cerr << std::string(depth*6, '-') << "[print] sizeChildren: " << sizeChildren << endl;
				cerr << std::string(depth*6, '-') << "[print] children: " << endl;
				for (int i = 0; i < sizeKeys; i++) {
					children[i]->print(depth + 1);
					std::cerr << string(depth*8, '-') << keys[i] << endl;
				}
				children[sizeKeys]->print(depth+1);
			}
	};

	class LeafNode : public Node {
		public:
			E data[order*2+1];
			Node *next;
			int size;
			LeafNode(Node *pt = 0): Node(pt) {
				size = 0;
			}
			~LeafNode() {
				cerr << "[~] destructing Leaf node..." << endl;
			}	
			bool addData(const E &e) {
				int i = 0;
				while (i < size and !(data[i] > e)) {
					cerr << e << " == " << data[i] << endl;
					if (e == data[i]) {
						cerr << "[BTree::add] already in node" << endl;
						return true;
					}
					i++;
				}
				if (size == order*2+1)
					return false;
				for (int s = size; s > i; s--) {
					data[s] = data[s-1];
				}
				data[i] = e;
				size++;
				return true;
			}
			bool isLeafNode() {
				return true;
			}
			void init() {
				size = 0;
			}
			void print(int depth = 0) {
				cerr << string(depth*6, '-') << "[print] Leaf Node" << endl;
				cerr << string(depth*6, '-') << "[print] size: " << size << endl;
				cerr << string(depth*6, '-') << "[print] data: ";
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
				cerr << e << " == " << newRoot->data[i] << endl;
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
		while (parent_index < temp->sizeKeys and 
			!((temp->keys[parent_index] > e))) {
			parent_index++;
		}
		current = temp->children[parent_index];
	}
	// found node, check if element already in there
	LeafNode* temp = static_cast<LeafNode*>(current);
	cerr << "[BTree::add] node found, attempting to insert..." << endl;
	if (temp->addData(e)) {
		cerr << "[BTree::add] inserted, returning" << endl;
		return;
	}

	cerr << "[BTree::add] node full, splitting nodes..." << endl;
	LeafNode* insertNode = new LeafNode(temp->parent);

	E dataOverflow;
	if (e > temp->data[order*2]) {
		dataOverflow = e;
	} else {
		dataOverflow = temp->data[order*2];
		temp->size--;
		temp->addData(e);
	}

	for (int i = 0; i < order; i++) {
		insertNode->data[i] = temp->data[i+order+1];
	}
	insertNode->data[order] = dataOverflow;
	insertNode->size = order+1;
	temp->size = order+1;

	InnerNode* currentInner = static_cast<InnerNode*>(temp->parent);

	if (currentInner->insertLeafNode(insertNode)) {
		cerr << "[BTree::add] split leaf nodes, no further splits neccessary." << endl;
		cerr << "[BTree::add] insert successful, returning" << endl;
		return;
	}

	cerr << "[BTree::add] parent node full, splitting inner node..." << endl;
	// strange technique: add element into node, save overflow ( lastt element)
	// then split nodes in middle

	cerr << "currentInner: ( node to split)" << endl;
	currentInner->print();

	E& insertLeafKey = insertNode->data[0];

	LeafNode* overflow;
	if (insertNode->data[0] > static_cast<LeafNode*>(currentInner->children[order*2])->data[0]) {
		overflow = insertNode;
	} else {
		overflow = static_cast<LeafNode*>(currentInner->children[order*2]);
		currentInner->sizeChildren--;
		currentInner->sizeKeys--;
		//TODO!!
		cerr << "insertLeafNode with key " << insertNode->data[0] << endl;
		currentInner->insertLeafNode(insertNode);
	}

	InnerNode* newRight = new InnerNode(currentInner->parent);

	for (int i = order+1; i < order*2; i++) {
		newRight->keys[i-(order+1)] = currentInner->keys[i];
		newRight->children[i-(order+1)] = currentInner->children[i];
		newRight->children[i-(order+1)]->parent = newRight;
	}
	newRight->children[order-1] = currentInner->children[order*2];
	newRight->children[order-1]->parent = newRight;
	newRight->children[order] = overflow;
	newRight->children[order]->parent = newRight;
	newRight->keys[order-1] = overflow->data[0];

	newRight->sizeKeys = order;
	newRight->sizeChildren = order+1;

	currentInner->sizeKeys = order;
	currentInner->sizeChildren = order+1;

	E& insertKey = currentInner->keys[order];

	// now insert insertKey and newRight into parent node...
	while (currentInner->parent) {
		cerr << "newRight:" << endl;
		newRight->print();
		cerr << endl << endl;
		cerr << "insertKey " << insertKey << endl;
		currentInner = static_cast<InnerNode*>(currentInner->parent);
		if (currentInner->insertInnerNode(newRight, insertKey)) {
			cerr << "[BTee::add] inserted into node, done" << endl;
			return;
		}
		cerr << "[BTree::add] next parent node full, splitting..." << endl;
		// else split this node, this time with inner nodes
		InnerNode* overflow;
		if (newRight->keys[0] > currentInner->keys[order*2-1]) {
			overflow = newRight;
			cerr << "!!!!!OVERFLOW = NEWRIGHT" << endl;
		} else {
			overflow = static_cast<InnerNode*>(currentInner->children[order*2]);
			currentInner->sizeChildren--;
			currentInner->sizeKeys--;
			currentInner->insertInnerNode(newRight, insertKey);
			insertKey = overflow->keys[0];
		}

		newRight = new InnerNode(currentInner->parent);

		for (int i = order+1; i < order*2; i++) {
			newRight->keys[i-(order+1)] = currentInner->keys[i];
			newRight->children[i-(order+1)] = currentInner->children[i];
			newRight->children[i-(order+1)]->parent = newRight;
		}
		newRight->children[order-1] = currentInner->children[order*2];
		newRight->children[order-1]->parent = newRight;
		newRight->children[order] = overflow;
		newRight->children[order]->parent = newRight;
		newRight->keys[order-1] = insertKey;


		newRight->sizeKeys = order;
		newRight->sizeChildren = order+1;

		currentInner->sizeKeys = order;
		currentInner->sizeChildren = order+1;
		insertKey = currentInner->keys[order];

		cerr << "currentInner: ( nodes after split)" << endl;
		currentInner->print();
		cerr << "newright: ( nodes after split)" << endl;
		newRight->print();
		cerr << "chenge insertkey to " << insertKey << endl;
	}

	/*cerr << "----------------------------------------------insertKey: " << insertKey << endl;*/
	/*cerr << "currentInner: " << endl;*/
	/*currentInner->print();*/
	/*cerr << "newRight: " << endl;*/
	/*newRight->print();*/
	cerr << "[BTree::add] root split, inserting with key = " << insertKey << "..." << endl;
	InnerNode* newRoot = new InnerNode(0);
	newRoot->keys[0] = insertKey;
	newRoot->children[0] = currentInner;
	newRoot->children[1] = newRight;
	newRoot->sizeChildren = 2;
	newRoot->sizeKeys = 1;
	currentInner->parent = newRoot;
	newRight->parent = newRoot;
	root = newRoot;
	cerr << "[BTree::add] created new root node, returning" << endl;
	return;
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
	cerr << "[member] " << e << "?";
	Node *current = root;
	while (!current->isLeafNode()) {
		InnerNode* temp = static_cast<InnerNode*>(current);
		int next = 0;
		while (next < temp->sizeKeys and (e > temp->keys[next] or e == temp->keys[next])) {
			next++;
		}
		current = temp->children[next];
	}
	LeafNode* temp = static_cast<LeafNode*>(current);
	for (int i = 0; i < temp->size; i++) {
		if (temp->data[i] == e) {
			cerr << " yes" << endl;
			return true;
		}
	}
	cerr << " no!" << endl;
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
	return E();
}

template <typename E>
E BTree<E>::max() const{
	return E();
}

template <typename E>
std::ostream& BTree<E>::print(std::ostream &o) const{
	return o;
}

#endif //BINTREE_H
