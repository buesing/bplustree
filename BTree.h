#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "Container.h"
#include <stdlib.h>

using namespace std;

class BTreeEmptyException;
class BTreeInternalErrorException;

template <typename E> 
class BTree : public Container<E> {

	class InnerNode;
	class LeafNode;
	// SUBCLASS NODE
	class Node {
		public:
			Node* parent;
			int nodeOrder;
			Node(Node* p, int o): parent(p), nodeOrder(o) {
			}
			virtual void init() {
			}
			virtual ~Node() {}
			virtual bool isLeafNode() = 0;
			virtual void print(ostream&o = cerr, int depth = 0) = 0;
	};
	class InnerNode : public Node {
		public:
			E *keys;
			Node **children;
			int sizeChildren;
			int sizeKeys;
			bool isLeafNode() {
				return false;
			}
			InnerNode(Node* pt, int o): Node(pt, o) { 
				sizeChildren = 0;
				sizeKeys = 0;
				keys = new E[o*2];
				children = new Node*[o*2+1];
			}
			~InnerNode(){
				cerr << "[InnerNode::~] destructing Inner node..." << endl;
				for (int i = 0; i < sizeChildren; i++) {
					delete children[i];
				}
				sizeChildren = 0;
				sizeKeys = 0;
				delete [] keys;
				delete [] children;
				children = 0;
				keys = 0;
			}
			void init() {
				sizeChildren = 0;
				sizeKeys = 0;
			}
			bool insertLeafNode(LeafNode* n) {
				if (sizeChildren < this->nodeOrder*2+1) {
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
				if (sizeChildren < this->nodeOrder*2+1) {
					n->parent = this;
					cerr << "[InnerNode::insertInnerNode] inserting..." << endl;
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
			void print(ostream &o = cerr, int depth = 0) {
				o << std::string(depth*4, '-') << "[print] Inner Node" << endl;
				o << std::string(depth*4, '-') << "[print] sizeKeys: " << sizeKeys << endl;
				o << std::string(depth*4, '-') << "[print] sizeChildren: " << sizeChildren << endl;
				o << std::string(depth*4, '-') << "[print] children: " << endl;
				for (int i = 0; i < sizeKeys; i++) {
					children[i]->print(o, depth + 1);
					o << string(depth*6, '-') << keys[i] << endl;
				}
				children[sizeKeys]->print(o, depth+1);
			}
	};

	class LeafNode : public Node {
		public:
			E *data;
			Node *next;
			int size;
			LeafNode(Node *pt, int o): Node(pt, o) {
				size = 0;
				data = new E[o*2+1];
			}
			~LeafNode() {
				cerr << "[~] destructing Leaf node..." << endl;
			}	
			bool addData(const E &e) {
				int i = 0;
				while (i < size and !(data[i] > e)) {
					if (e == data[i]) {
						cerr << "[BTree::add] already in node" << endl;
						return true;
					}
					i++;
				}
				if (size == this->nodeOrder*2+1)
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
			void remove(const E& key) {
				int i = 0;
				while (i < size and !(data[i] == key)) {
					i++;
				}
				if (i == size)
					return;
				while (i < size) {
					data[i] = data[i+1];
					i++;
				}
				size--;
				return;
			}
			void print(ostream& o = cerr, int depth = 0) {
				o << string(depth*4, '-') << "[print] Leaf Node" << endl;
				o << string(depth*4, '-') << "[print] size: " << size << endl;
				o << string(depth*4, '-') << "[print] data: ";
				for (int i = 0; i < size; i++) {
					o << data[i] << " ";
				}
				o << endl;
			}
	};
	// END SUBCLASS NODE
	
	public:
		BTree(int);
		virtual ~BTree();

		virtual void add(const E& e);
		virtual void add(const E e[], size_t s);
		virtual void remove(const E& e);
		virtual void remove(const E e[], size_t s);
		virtual bool member(const E& e ) const;
		virtual size_t size() const;
		virtual void size_(Node* current, size_t &n) const;
		virtual bool empty() const;
		virtual size_t apply( const Functor<E>& f, Order order) const;
		virtual bool apply_( const Functor<E>& f, Order order, Node* current, size_t &n) const;
		virtual E min() const;
		virtual E max() const;
		virtual E min_(Node* current) const;
		virtual E max_(Node* current) const;
		virtual std::ostream& print(std::ostream &o) const;
		bool validate(Node* current) const;
		E& findInsertKey(Node* current) const;
		/*private:*/
		int order;
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
BTree<E>::BTree(int o = 16): order(o){
	root = new LeafNode(0, o);

}

template <typename E> 
E& BTree<E>::findInsertKey(Node *current) const {
	if (!current->isLeafNode()) {
		return (findInsertKey(static_cast<InnerNode*>(current)->children[0]));
	} else {
		return (static_cast<LeafNode*>(current)->data[0]);
	}
}
template <typename E> 
bool BTree<E>::validate(Node *current) const {
	if (!current->isLeafNode()) {
		InnerNode* temp = static_cast<InnerNode*>(current);
		if (!temp->children[0]->isLeafNode()) {
			for (int i = 0; i < temp->sizeChildren; i++) {
				validate(temp->children[i]);
			}
		} else {
			for (int childIndex = 0; childIndex < temp->sizeKeys-1; childIndex++) {
				LeafNode* ln = static_cast<LeafNode*>(temp->children[childIndex]);
				for (int i = 0; i < ln->size; i++) {
					/*cerr << "and "<<ln->data[i]<<" <= " << temp->keys[childIndex] << "?" << endl;*/
					if (!(temp->keys[childIndex] > ln->data[i])) {
						cerr << temp->keys[childIndex] <<" > " << ln->data[i] << "?" << endl;
						this->print(cerr);
							/*!(ln->data[i] > temp->keys[childIndex])) {*/
						return false;
					}
					if (temp->parent and temp->keys[0] == static_cast<InnerNode*>(temp->parent)->keys[childIndex]) {
						root->print();
						cout << "INVALID" << endl;
						exit(EXIT_FAILURE);
						return false;
					}
				}
				
			}
			
		}
	}
	return true;
	
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
			LeafNode *right = new LeafNode(0, order);
			LeafNode *left = new LeafNode(0, order);
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
			InnerNode *newRoot = new InnerNode(0, order);
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
	LeafNode* insertNode = new LeafNode(temp->parent, order);

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

	LeafNode* overflow;
	if (insertNode->data[0] > static_cast<LeafNode*>(currentInner->children[order*2])->data[0]) {
		overflow = insertNode;
	} else {
		overflow = static_cast<LeafNode*>(currentInner->children[order*2]);
		currentInner->sizeChildren--;
		currentInner->sizeKeys--;
		currentInner->insertLeafNode(insertNode);
	}

	InnerNode* newRight = new InnerNode(currentInner->parent, order);

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
		} else {
			overflow = static_cast<InnerNode*>(currentInner->children[order*2]);
			currentInner->sizeChildren--;
			currentInner->sizeKeys--;
			cerr << "is the error here?" << endl;
			currentInner->insertInnerNode(newRight, insertKey);
			/*insertKey = overflow->keys[0];*/
			insertKey = findInsertKey(overflow);
		}

		newRight = new InnerNode(currentInner->parent, order);

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
	}

	cerr << "[BTree::add] root split, inserting with key = " << insertKey << "..." << endl;
	InnerNode* newRoot = new InnerNode(0, order);
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
	// TODO merge underflowing nodes
	Node *current = root;
	cerr << "[BTree::remove] looking up appropriate node..." << endl;
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
	temp->remove(e);
	if (temp->size < order) {
		// underflow, try to borrow element from neighbor
		InnerNode* parent = static_cast<InnerNode*>(temp->parent);
		if (parent_index != 0 && static_cast<LeafNode*>(parent->children[parent_index-1])->size > order) {
			LeafNode *left = static_cast<LeafNode*>(parent->children[parent_index-1]);
			temp->addData(left->data[--left->size]);
			return;
		}
		if (parent_index != parent->sizeKeys && static_cast<LeafNode*>(parent->children[parent_index-1])->size > order) {
			LeafNode *right = static_cast<LeafNode*>(parent->children[parent_index-1]);
		}
	}
	return;
}

template <typename E> 
void BTree<E>::remove(const E e[], size_t s){
	for (unsigned int i = 0; i < s; i++) {
		remove(e[i]);
	}
	return;
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
	size_t i = 0;
	size_(root, i);
	return i;

}

template <typename E> 
void BTree<E>::size_(Node *current, size_t &n) const{
	if (!current->isLeafNode()) {
		for (int i = 0; i < static_cast<InnerNode*>(current)->sizeChildren; i++) {
			size_(static_cast<InnerNode*>(current)->children[i], n);
		}
	} else {
		n += static_cast<LeafNode*>(current)->size;
	}
}

template <typename E> 
bool BTree<E>::empty() const{
	/*return (root->isLeafNode() && static_cast<LeafNode*>(root)->size == 0);*/
	return size() == 0;
}

template <typename E> 
size_t BTree<E>::apply( const Functor<E>& f, Order order = dontcare ) const{
	size_t i;
	apply_(f, order, root, i);
	return i;
}

template <typename E> 
bool BTree<E>::apply_( const Functor<E>& f, Order order, Node *current, size_t &n ) const{
	bool cont = true;
	if (current->isLeafNode()) {
		LeafNode *currentLeaf = static_cast<LeafNode*>(current);
		if (order == ascending) {
			for (int i = 0; i < currentLeaf->size; i++) {
				if (cont) {
					cont = f(currentLeaf->data[i]);
					n++;
				} else {
					return false;
				}
			}
		} else {
			for (int i = currentLeaf->size-1; i >= 0; i--) {
				if (cont) {
					cont = f(currentLeaf->data[i]);
					n++;
				} else {
					return false;
				}
			}

		}
		return true;
	} else {
		InnerNode *currentInner = static_cast<InnerNode*>(current);
		if (order == ascending) {
			for (int i = 0; i < currentInner->sizeChildren; i++) {
				if (!apply_(f, order, currentInner->children[i], n)) {
					return true;
				}
			}
		} else {
			for (int i = currentInner->sizeChildren-1; i >= 0; i--) {
				if (!apply_(f, order, currentInner->children[i], n)) {
					return true;
				}
			}

		}
	}
	return true;
}

template <typename E> 
E BTree<E>::min() const{
	return min_(root);
}

template <typename E> 
E BTree<E>::min_(Node *current) const{
	if (!current->isLeafNode()) {
		return (min_(static_cast<InnerNode*>(current)->children[0]));
	} else {
		return (static_cast<LeafNode*>(current)->data[0]);
	}
}

template <typename E>
E BTree<E>::max() const{
	return (max_(root));
}

template <typename E>
E BTree<E>::max_(Node* current) const{
	if (!current->isLeafNode()) {
		return (max_(static_cast<InnerNode*>(current)->children[static_cast<InnerNode*>(current)->sizeChildren-1]));
	} else {
		return (static_cast<LeafNode*>(current)->data[static_cast<LeafNode*>(current)->size-1]);
	}
}

template <typename E>
std::ostream& BTree<E>::print(std::ostream &o) const{
	root->print(o);
	return o;
}

#endif //BINTREE_H
