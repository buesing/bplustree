// TODO:
// implement size variable
// quicker member?
// quicker add?

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "Container.h"
#include <stdlib.h>

using namespace std;

class BTreeEmptyException;
class BTreeInternalException;

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
			~InnerNode() {
				sizeChildren = 0;
				sizeKeys = 0;
				delete [] keys;
				delete [] children;
				children = 0;
				keys = 0;
			}
			void init() {
				this->parent = 0;
				sizeChildren = 0;
				sizeKeys = 0;
			}
			bool insertLeafNode(LeafNode* n) {
				if (sizeChildren < this->nodeOrder*2+1) {
					int at = 0;
					while (sizeKeys != at and n->data[0] > keys[at]) at++;
					for (int i = sizeKeys; i > at ; i--) keys[i] = keys[i-1];
					keys[at] = n->data[0];
					at++;
					for (int i = sizeChildren; i > at; i--) children[i] = children[i-1];
					children[at] = n;
					sizeKeys++;
					sizeChildren++;
					return true;
				} else return false;
			}
			bool insertInnerNode(InnerNode* n, E& key) {
				if (sizeChildren < this->nodeOrder*2+1) {
					n->parent = this;
					int at = 0;
					while (sizeKeys != at and !(keys[at] > key)) at++;
					for (int i = sizeKeys; i > at ; i--) keys[i] = keys[i-1];
					keys[at] = key;
					at++;
					for (int i = sizeChildren; i > at; i--) children[i] = children[i-1];
					children[at] = n;
					sizeKeys++;
					sizeChildren++;
					return true;
				} else return false;
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
			LeafNode *next;
			LeafNode *prev;
			E *data;
			int size;
			LeafNode(Node *pt, int o, LeafNode* n, LeafNode* p): Node(pt, o), next(n), prev(p) {
				size = 0;
				data = new E[o*2+1];
			}
			~LeafNode() {
				delete [] data;
				data = 0;
			}	
			bool addData(const E &e) {
				int i = 0;
				while (i < size and !(data[i] > e)) {
					if (e == data[i]) return true;
					i++;
				}
				if (size == this->nodeOrder*2+1) return false;
				for (int s = size; s > i; s--) data[s] = data[s-1];
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
				while (i < size and !(data[i] == key)) i++;
				if (i == size) return;
				if (i == size-1) {
					size--;
					return;
				}
				while (i < size-1) {
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
	}; // END SUBCLASS NODE
	
	public:
		BTree(int);
		virtual ~BTree();
		virtual void add(const E& e);
		virtual void add(const E e[], size_t s);
		virtual void remove(const E& e);
		virtual void remove(const E e[], size_t s);
		virtual bool member(const E& e) const;
		virtual size_t size() const;
		virtual bool empty() const;
		virtual size_t apply(const Functor<E>& f, Order order) const;
		virtual E min() const;
		virtual E max() const;
		virtual std::ostream& print(std::ostream &o) const;
	private:
		virtual void delete_(Node* current);
		bool validate(Node* current) const;
		E& findInsertKey(Node* current) const;
		int order;
		size_t items;
		E minimum;
		E maximum;
		Node *root;
};

class BTreeEmptyException : public ContainerException {
	public:
		virtual const char * what() const throw() { return "BTree is empty"; }
};

class BTreeInternalException : public ContainerException {
	const char * w;
	public:
	explicit BTreeInternalException(const char * w) throw() : w(w) {}
	virtual const char * what() const throw() { return w; }
};

template <typename E> 
BTree<E>::BTree(int o = 8): order(o){
	// actual allocations begins in add()
	root = 0;
	items = 0;
	minimum = E();
	maximum = E();
}

template <typename E> 
BTree<E>::~BTree(){
	delete_(root);
}

template <typename E> 
void BTree<E>::delete_(Node* current){
	if (current) {
		if (current->isLeafNode()) {
			delete current;
			return;
		} else {
			for (int i = 0; i < static_cast<InnerNode*>(current)->sizeChildren; i++) {
				delete_(static_cast<InnerNode*>(current)->children[i]);
			}
			delete current;
		}
	}
}

template <typename E> 
E& BTree<E>::findInsertKey(Node *current) const {
	if (!current->isLeafNode()) return (findInsertKey(static_cast<InnerNode*>(current)->children[0]));
	else return (static_cast<LeafNode*>(current)->data[0]);
}

template <typename E> 
bool BTree<E>::validate(Node *current) const {
	if (!current->isLeafNode()) {
		InnerNode* temp = static_cast<InnerNode*>(current);
		if (temp->parent && temp->sizeKeys < order) return false;
		if (temp->sizeKeys != temp->sizeChildren-1) return false;
		if (!temp->children[0]->isLeafNode()) {
			for (int i = 0; i < temp->sizeChildren; i++) {
				if (!(static_cast<InnerNode*>(static_cast<InnerNode*>(temp->children[i])->parent)->keys[0] == temp->keys[0])) {
					return false;
				}
				return (validate(temp->children[i]));
			}
		} else {
			for (int childIndex = 0; childIndex < temp->sizeKeys-1; childIndex++) {
				LeafNode* ln = static_cast<LeafNode*>(temp->children[childIndex]);
				for (int i = 0; i < ln->size; i++) {
					if (!(temp->keys[childIndex] > ln->data[i])) return false;
					if (temp->parent and temp->keys[0] == static_cast<InnerNode*>(temp->parent)->keys[childIndex])
						return false;
				}
			}
		}
	}
	return true;
}

template <typename E> 
void BTree<E>::add(const E& e) {
	if (!root) {
		root = new LeafNode(0,order, 0, 0);
		minimum = e;
		maximum = e;
	}
	if (minimum > e)
		minimum = e;
	if (e > maximum)
		maximum = e;
	if (root->isLeafNode()) {
		if (static_cast<LeafNode*>(root)->addData(e)) {
			// Root Node is not full, easiest case
			return;
		} else {
			// Root Node is full
			LeafNode *right = new LeafNode(0, order, 0, 0);
			LeafNode *left = new LeafNode(0, order, 0, 0);
			int stop = e > static_cast<LeafNode*>(root)->data[order] ? order+1 : order;
			for (int i = 0; i < stop; i++) {
				left->data[i] = static_cast<LeafNode*>(root)->data[i];
				left->size++;
			}
			for (int i = stop; i < order*2+1; i++) {
				right->data[i-stop] = static_cast<LeafNode*>(root)->data[i];
				right->size++;
			}
			delete root;
			root = 0;
			stop == order ? left->addData(e) : right->addData(e);
			root = new InnerNode(0, order);
			static_cast<InnerNode*>(root)->children[0] = left;
			static_cast<InnerNode*>(root)->children[1] = right;
			static_cast<InnerNode*>(root)->keys[0] = right->data[0];
			static_cast<InnerNode*>(root)->sizeChildren = 2;
			static_cast<InnerNode*>(root)->sizeKeys = 1;
			left->parent = root;
			right->parent = root;
			left->next = right;
			right->prev = left;
			return;
		}
	}

	Node *current = root;
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
	if (temp->addData(e)) return;

	LeafNode* insertNode = new LeafNode(temp->parent, order, 0, 0);

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

	// rebuild linked list
	insertNode->next = temp->next;
	insertNode->prev = temp;
	if (temp->next) temp->next->prev = insertNode;
	temp->next = insertNode;

	InnerNode* currentInner = static_cast<InnerNode*>(temp->parent);

	if (currentInner->insertLeafNode(insertNode)) {
		return;
	}

	// strange technique: add element into node, save overflow (last element)
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
			return;
		}
		// else split this node, this time with inner nodes
		InnerNode* overflow;
		if (newRight->keys[0] > currentInner->keys[order*2-1]) {
			overflow = newRight;
		} else {
			overflow = static_cast<InnerNode*>(currentInner->children[order*2]);
			currentInner->sizeChildren--;
			currentInner->sizeKeys--;
			currentInner->insertInnerNode(newRight, insertKey);
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

	InnerNode* newRoot = new InnerNode(0, order);
	newRoot->keys[0] = insertKey;
	newRoot->children[0] = currentInner;
	newRoot->children[1] = newRight;
	newRoot->sizeChildren = 2;
	newRoot->sizeKeys = 1;
	currentInner->parent = newRoot;
	newRight->parent = newRoot;
	root = newRoot;
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
	if (!root) return;
	Node *current = root;
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
	// found node, check if element in there
	LeafNode* temp = static_cast<LeafNode*>(current);
	temp->remove(e);
	
	// if tree is empty now delete root
	if (root->isLeafNode() && static_cast<LeafNode*>(root)->size == 0) {
		delete root;
		root = 0;
		minimum = E();
		maximum = E();
		return;
	}

	// update min/max if neccessary
	if (minimum == e) {
		minimum = temp->data[0];
	}
	if (maximum == e) {
		maximum = temp->data[temp->size-1];
	}
	
	// if no underflows, return
	if (temp->size >= order || !temp->parent) {
		return;
	}
	
	// now we have an underflow and we are not working on the root node, so a rebuild is neccessary
	// try borrowing element from left sibling
	InnerNode* parent = static_cast<InnerNode*>(temp->parent);
	if (parent_index != 0 && static_cast<LeafNode*>(parent->children[parent_index-1])->size > order) {
		LeafNode *leftSibling = static_cast<LeafNode*>(parent->children[parent_index-1]);
		temp->addData(leftSibling->data[--leftSibling->size]);
		parent->keys[parent_index-1] = temp->data[0];
		return;
	}

	// try borrowing element from right sibling
	if (parent_index < parent->sizeKeys && static_cast<LeafNode*>(parent->children[parent_index+1])->size > order) {
		LeafNode *rightSibling = static_cast<LeafNode*>(parent->children[parent_index+1]);
		temp->addData(rightSibling->data[0]);
		rightSibling->remove(rightSibling->data[0]);
		parent->keys[parent_index] = rightSibling->data[0];
		return;
	}

	// else delete node and insert its elements
	// remove key from parent node
	int i = parent_index;
	while (i < parent->sizeKeys && i+1 < order*2) {
		parent->children[i] = parent->children[i+1];
		parent->keys[i] = parent->keys[i+1];
		i++;
	}
	parent->children[i] = parent->children[i+1];
	parent->sizeChildren--;
	parent->sizeKeys--;

	// node is now deleted, now insert remaining elements
	for (i = 0; i < temp->size; i++) {
		add(temp->data[i]);
	}
	if (temp->prev) temp->prev->next = temp->next;
	if (temp->next) temp->next->prev = temp->prev;
	delete temp;
	temp = 0;

	// done, now check if the parent node is underflowing
	InnerNode *curr = parent;
	parent = static_cast<InnerNode*>(curr->parent);
	while (parent && curr->sizeKeys < order) {
		parent_index = 0;
		while (parent_index < parent->sizeKeys && !(parent->keys[parent_index] > curr->keys[0]))
			parent_index++;

		if (parent_index != 0 && static_cast<InnerNode*>(parent->children[parent_index-1])->sizeKeys > order) {
			InnerNode *leftSibling = static_cast<InnerNode*>(parent->children[parent_index-1]);

			// insert rightmost node of left sibling at beginning
			for (int i = curr->sizeKeys; i > 0; i--) {
				curr->keys[i] = curr->keys[i-1];
				curr->children[i+1] = curr->children[i];
			}
			curr->children[1] = curr->children[0];
			curr->children[0] = leftSibling->children[leftSibling->sizeChildren-1];
			curr->children[0]->parent = curr;
			curr->keys[0] = parent->keys[parent_index-1];
			curr->sizeChildren++;
			curr->sizeKeys++;
			leftSibling->sizeChildren--;
			leftSibling->sizeKeys--;

			parent->keys[parent_index-1] = leftSibling->keys[leftSibling->sizeKeys];
			return;
		}
		if (parent_index < parent->sizeKeys && static_cast<InnerNode*>(parent->children[parent_index+1])->sizeKeys > order) {
			InnerNode *rightSibling = static_cast<InnerNode*>(parent->children[parent_index+1]);
			curr->keys[curr->sizeKeys++] = parent->keys[parent_index];
			curr->children[curr->sizeChildren++] = rightSibling->children[0];
			curr->children[curr->sizeChildren-1]->parent = curr;
			parent->keys[parent_index] = rightSibling->keys[0];

			// delete old node out of right sibling
			for (int i = 0; i < rightSibling->sizeKeys-1; i++) {
				rightSibling->children[i] = rightSibling->children[i+1];
				rightSibling->keys[i] = rightSibling->keys[i+1];
			}
			rightSibling->children[rightSibling->sizeKeys-1] = rightSibling->children[rightSibling->sizeKeys];
			rightSibling->sizeChildren--;
			rightSibling->sizeKeys--;
			return;
		}


		// if right sibling, put all their elements into curr
		if (parent_index < parent->sizeKeys) {
			InnerNode *rightSibling = static_cast<InnerNode*>(parent->children[parent_index+1]);
			curr->insertInnerNode(static_cast<InnerNode*>(rightSibling->children[0]),parent->keys[parent_index]);
			
			// insert elements of right sibling into this node
			int i = 1;
			while (i < rightSibling->sizeKeys) {
				curr->keys[curr->sizeKeys++] = rightSibling->keys[i-1];
				curr->children[curr->sizeChildren++] = rightSibling->children[i];
				curr->children[curr->sizeChildren-1]->parent = curr;
				i++;
			}
			curr->keys[curr->sizeKeys++] = rightSibling->keys[i-1];
			curr->children[curr->sizeChildren++] = rightSibling->children[rightSibling->sizeKeys];
			curr->children[curr->sizeChildren-1]->parent = curr;
			// delete old key from parent node
			i = parent_index;
			while (i < parent->sizeKeys-1) {
				parent->children[i+1] = parent->children[i+2];
				parent->keys[i] = parent->keys[i+1];
				i++;
			}
			parent->sizeKeys--;
			parent->sizeChildren--;
			delete rightSibling;
			rightSibling = 0;
		}
		// if left sibling, put all currs elements there
		else if (parent_index > 0) {

			// put as many nodes as possible in left sibling
			InnerNode *leftSibling = static_cast<InnerNode*>(parent->children[parent_index-1]);
			leftSibling->insertInnerNode(static_cast<InnerNode*>(curr->children[0]), parent->keys[parent_index-1]);
			for (int i = 0; i < curr->sizeKeys; i++) {
				leftSibling->keys[leftSibling->sizeKeys++] = curr->keys[i];
				leftSibling->children[leftSibling->sizeChildren++] = curr->children[i+1];
				leftSibling->children[leftSibling->sizeChildren-1]->parent = leftSibling;
			}
			
			// delete old key from parent node
			int i = parent_index;
			while (i < parent->sizeKeys) {
				parent->children[i] = parent->children[i+1];
				parent->keys[i-1] = parent->keys[i];
				i++;
			}
			parent->sizeKeys--;
			parent->sizeChildren--;
			delete curr;
			curr = 0;
		} 
		// done, continue with parent of parent
		curr = parent;
		parent = static_cast<InnerNode*>(curr->parent);
	}
	if (!parent and curr->sizeKeys == 0) {
		root = curr->children[0];
		delete root->parent;
		root->parent = 0;
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
bool BTree<E>::member(const E& e) const{
	if (!root) return false;
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
			return true;
		}
	}
	return false;
}

template <typename E> 
size_t BTree<E>::size() const{
	if (!root) return 0;
	size_t n = 0;
	Node *current = root;
	while (!current->isLeafNode())
		current = static_cast<InnerNode*>(current)->children[0];
	LeafNode *c = static_cast<LeafNode*>(current);
	while (c) {
		n += c->size;
		c = c->next;
	}
	return n;
}

template <typename E> 
bool BTree<E>::empty() const{
	return (size() == 0);
}

template <typename E> 
size_t BTree<E>::apply(const Functor<E>& f, Order o = dontcare) const {
	if (!root) return 0;
	size_t n = 0;
	if (o == ascending) {
		// look for leftmost node
		Node *current = root;
		while (!current->isLeafNode())
			current = static_cast<InnerNode*>(current)->children[0];
		LeafNode *c = static_cast<LeafNode*>(current);
		while (c) {
			for (int i = 0; i < c->size; i++) {
				if (f(c->data[i]))
					n++;
				else
					return n+1;
			}
			c = c->next;
		}
		return n;
	} else {
		Node *current = root;
		while (!current->isLeafNode())
			current = static_cast<InnerNode*>(current)->children[static_cast<InnerNode*>(current)->sizeKeys];
		LeafNode *c = static_cast<LeafNode*>(current);
		while (c) {
			for (int i = c->size-1; i >= 0; i--) {
				if (f(c->data[i]))
					n++;
				else
					return n+1;
			}
			c = c->prev;
		}
		return n;
	}
}

template <typename E> 
E BTree<E>::min() const {
	if (!root) throw BTreeEmptyException();
	return minimum;
}

template <typename E>
E BTree<E>::max() const {
	if (!root) throw BTreeEmptyException();
	return maximum;
}

template <typename E>
std::ostream& BTree<E>::print(std::ostream &o) const {
	if (root)
		root->print(o);
	return o;
}

#endif //BINTREE_H
