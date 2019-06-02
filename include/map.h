#ifndef MAP_H
#define MAP_H



#include <iostream>

using namespace std;

// key structure that represents a node in the tree
template<class K, class D>
struct Node {
	K key; // holds the key
  D data;
	Node<K, D> *parent; // pointer to the parent
	Node<K, D> *left; // pointer to left child
	Node<K, D> *right; // pointer to right child
	bool color; // 1 -> Red, 0 -> Black
};

// typedef Node<K, D> *Node<K, D> *;

// class RBTree implements the operations in Red Black Tree
template<class K, class D>
class RBTree {
public:
	Node<K, D> * root;
	Node<K, D> * TNULL;
	int size = 0;

	// initializes the nodes with appropirate values
	// all the pointers are set to point to the null pointer
	__host__ __device__ void initializeNULLNode(Node<K, D> * node, Node<K, D> * parent) {
		node->key = 0;
    node->data = 0;
		node->parent = parent;
		node->left = nullptr;
		node->right = nullptr;
		node->color = false;
	}

	void preOrderHelper(Node<K, D> * node) {
		if (node != TNULL) {
			cout<<node->key<<" ";
			preOrderHelper(node->left);
			preOrderHelper(node->right);
		}
	}

	void inOrderHelper(Node<K, D> * node) {
		if (node != TNULL) {
			inOrderHelper(node->left);
			cout<<node->key<<" ";
			inOrderHelper(node->right);
		}
	}

	void postOrderHelper(Node<K, D> * node) {
		if (node != TNULL) {
			postOrderHelper(node->left);
			postOrderHelper(node->right);
			cout<<node->key<<" ";
		}
	}

	__host__ __device__ Node<K, D> * searchTreeHelper(Node<K, D> * node, K key) {
		if (node == TNULL || key == node->key) {
			return node;
		}

		if (key < node->key) {
			return searchTreeHelper(node->left, key);
		}
		return searchTreeHelper(node->right, key);
	}

	// fix the rb tree modified by the delete operation
	__host__ __device__ void fixDelete(Node<K, D> * x) {
		Node<K, D> * s;
		while (x != root && x->color == false) {
			if (x == x->parent->left) {
				s = x->parent->right;
				if (s->color == true) {
					// case 3.1
					s->color = false;
					x->parent->color = true;
					leftRotate(x->parent);
					s = x->parent->right;
				}

				if (s->left->color == false && s->right->color == false) {
					// case 3.2
					s->color = true;
					x = x->parent;
				} else {
					if (s->right->color == false) {
						// case 3.3
						s->left->color = false;
						s->color = true;
						rightRotate(s);
						s = x->parent->right;
					}

					// case 3.4
					s->color = x->parent->color;
					x->parent->color = false;
					s->right->color = false;
					leftRotate(x->parent);
					x = root;
				}
			} else {
				s = x->parent->left;
				if (s->color == true) {
					// case 3.1
					s->color = false;
					x->parent->color = true;
					rightRotate(x->parent);
					s = x->parent->left;
				}

				if (s->right->color == false && s->right->color == false) {
					// case 3.2
					s->color = true;
					x = x->parent;
				} else {
					if (s->left->color == false) {
						// case 3.3
						s->right->color = false;
						s->color = true;
						leftRotate(s);
						s = x->parent->left;
					}

					// case 3.4
					s->color = x->parent->color;
					x->parent->color = false;
					s->left->color = false;
					rightRotate(x->parent);
					x = root;
				}
			}
		}
		x->color = false;
	}


	__host__ __device__ void rbTransplant(Node<K, D> * u, Node<K, D> * v){
		if (u->parent == nullptr) {
			root = v;
		} else if (u == u->parent->left){
			u->parent->left = v;
		} else {
			u->parent->right = v;
		}
		v->parent = u->parent;
	}

	__host__ __device__ void deleteNodeHelper(Node<K, D> * node, K key) {
		// find the node containing key
		Node<K, D> * z = TNULL;
		Node<K, D> * x, * y;
		while (node != TNULL){
			if (node->key == key) {
				z = node;
			}

			if (node->key <= key) {
				node = node->right;
			} else {
				node = node->left;
			}
		}

		if (z == TNULL) {
			cout<<"Couldn't find key in the tree"<<endl;
			return;
		}

		y = z;
		bool y_original_color = y->color;
		if (z->left == TNULL) {
			x = z->right;
			rbTransplant(z, z->right);
		} else if (z->right == TNULL) {
			x = z->left;
			rbTransplant(z, z->left);
		} else {
			y = minimum(z->right);
			y_original_color = y->color;
			x = y->right;
			if (y->parent == z) {
				x->parent = y;
			} else {
				rbTransplant(y, y->right);
				y->right = z->right;
				y->right->parent = y;
			}

			rbTransplant(z, y);
			y->left = z->left;
			y->left->parent = y;
			y->color = z->color;
		}
		delete z;
		if (y_original_color == false){
			fixDelete(x);
		}
	}

	// fix the red-black tree
	__host__ __device__ void fixInsert(Node<K, D> * k){
		Node<K, D> * u;
		while (k->parent->color == true) {
			if (k->parent == k->parent->parent->right) {
				u = k->parent->parent->left; // uncle
				if (u->color == true) {
					// case 3.1
					u->color = false;
					k->parent->color = false;
					k->parent->parent->color = true;
					k = k->parent->parent;
				} else {
					if (k == k->parent->left) {
						// case 3.2.2
						k = k->parent;
						rightRotate(k);
					}
					// case 3.2.1
					k->parent->color = false;
					k->parent->parent->color = true;
					leftRotate(k->parent->parent);
				}
			} else {
				u = k->parent->parent->right; // uncle

				if (u->color == true) {
					// mirror case 3.1
					u->color = false;
					k->parent->color = false;
					k->parent->parent->color = true;
					k = k->parent->parent;
				} else {
					if (k == k->parent->right) {
						// mirror case 3.2.2
						k = k->parent;
						leftRotate(k);
					}
					// mirror case 3.2.1
					k->parent->color = false;
					k->parent->parent->color = true;
					rightRotate(k->parent->parent);
				}
			}
			if (k == root) {
				break;
			}
		}
		root->color = false;
	}

	__host__ __device__ void printHelper(Node<K, D> * root, string indent, bool last) {
		// print the tree structure on the screen
	   	if (root != TNULL) {
		   cout<<indent;
		   if (last) {
		      cout<<"R----";
		      indent += "     ";
		   } else {
		      cout<<"L----";
		      indent += "|    ";
		   }

           string sColor = root->color?"RED":"BLACK";
		   cout<<root->key<<"("<<sColor<<")"<<endl;
		   printHelper(root->left, indent, false);
		   printHelper(root->right, indent, true);
		}
		// cout<<root->left->key<<endl;
	}

public:
	__host__ __device__ RBTree() {
		TNULL = new Node<K, D>;
		TNULL->color = false;
		TNULL->left = nullptr;
		TNULL->right = nullptr;
		root = TNULL;
	}

	// Pre-Order traversal
	// Node->Left Subtree->Right Subtree
	__host__ __device__ void preorder() {
		preOrderHelper(this->root);
	}

	// In-Order traversal
	// Left Subtree -> Node -> Right Subtree
	__host__ __device__ void inorder() {
		inOrderHelper(this->root);
	}

	// Post-Order traversal
	// Left Subtree -> Right Subtree -> Node
	__host__ __device__ void postorder() {
		postOrderHelper(this->root);
	}

	// search the tree for the key k
	// and return the corresponding node
	__host__ __device__ Node<K, D> * searchTree(int k) {
		return searchTreeHelper(this->root, k);
	}

	// find the node with the minimum key
	__host__ __device__ Node<K, D> * minimum(Node<K, D> * node) {
		while (node->left != TNULL) {
			node = node->left;
		}
		return node;
	}

	// find the node with the maximum key
	__host__ __device__ Node<K, D> * maximum(Node<K, D> * node) {
		while (node->right != TNULL) {
			node = node->right;
		}
		return node;
	}

	// find the successor of a given node
	__host__ __device__ Node<K, D> * successor(Node<K, D> * x) {
		// if the right subtree is not null,
		// the successor is the leftmost node in the
		// right subtree
		if (x->right != TNULL) {
			return minimum(x->right);
		}

		// else it is the lowest ancestor of x whose
		// left child is also an ancestor of x.
		Node<K, D> * y = x->parent;
		while (y != TNULL && x == y->right) {
			x = y;
			y = y->parent;
		}
		return y;
	}

	// find the predecessor of a given node
	__host__ __device__ Node<K, D> * predecessor(Node<K, D> * x) {
		// if the left subtree is not null,
		// the predecessor is the rightmost node in the
		// left subtree
		if (x->left != TNULL) {
			return maximum(x->left);
		}

		Node<K, D> * y = x->parent;
		while (y != TNULL && x == y->left) {
			x = y;
			y = y->parent;
		}

		return y;
	}

	// rotate left at node x
	__host__ __device__ void leftRotate(Node<K, D> * x) {
		Node<K, D> * y = x->right;
		x->right = y->left;
		if (y->left != TNULL) {
			y->left->parent = x;
		}
		y->parent = x->parent;
		if (x->parent == nullptr) {
			this->root = y;
		} else if (x == x->parent->left) {
			x->parent->left = y;
		} else {
			x->parent->right = y;
		}
		y->left = x;
		x->parent = y;
	}

	// rotate right at node x
	__host__ __device__ void rightRotate(Node<K, D> * x) {
		Node<K, D> * y = x->left;
		x->left = y->right;
		if (y->right != TNULL) {
			y->right->parent = x;
		}
		y->parent = x->parent;
		if (x->parent == nullptr) {
			this->root = y;
		} else if (x == x->parent->right) {
			x->parent->right = y;
		} else {
			x->parent->left = y;
		}
		y->right = x;
		x->parent = y;
	}

	// insert the key to the tree in its appropriate position
	// and fix the tree
	 __host__ __device__ void insert(K key, D data) {
		// Ordinary Binary Search Insertion
		Node<K, D> * node = new Node<K, D>;
		node->parent = nullptr;
		node->key = key;
    node->data = data;
		node->left = TNULL;
		node->right = TNULL;
		node->color = true; // new Node<K, D> must be red

		size++;

		Node<K, D> * y = nullptr;
		Node<K, D> * x = this->root;

		while (x != TNULL) {
			y = x;
			if (node->key < x->key) {
				x = x->left;
			} else {
				x = x->right;
			}
		}

		// y is parent of x
		node->parent = y;
		if (y == nullptr) {
			root = node;
		} else if (node->key < y->key) {
			y->left = node;
		} else {
			y->right = node;
		}

		// if new Node<K, D> is a root node, simply return
		if (node->parent == nullptr){
			node->color = false;
			return;
		}

		// if the grandparent is null, simply return
		if (node->parent->parent == nullptr) {
			return;
		}

		// Fix the tree
		fixInsert(node);
	}

	__host__ __device__ Node<K, D> * getRoot(){
		return this->root;
	}

	// delete the node from the tree
	__host__ __device__ void deleteNode(K key) {
		deleteNodeHelper(this->root, key);
		size--; // need to verify if node exists
	}

	// print the tree structure on the screen
	__host__ __device__ void prettyPrint() {
	    if (root) {
    		printHelper(this->root, "", true);
	    }
	}

};


#endif
