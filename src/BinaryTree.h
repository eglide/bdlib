/*
 * Copyright (c) 2006-2014, Bryan Drewery <bryan@shatow.net>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* BinaryTree.h
 */
#ifndef _BD_BINARYTREE_H
#define _BD_BINARYTREE_H 1

#include "bdlib.h"
#include "Iterator.h"

BDLIB_NS_BEGIN


template <class Key, class Value>
/**
  * @class BinaryTree
  * @brief Binary tree data structure
  */
class BinaryTree {
  class BinaryTreeIterator;

  public:
    typedef BinaryTreeIterator iterator;
    typedef KeyValue<Key, Value> iterator_type;

  private:
    class Node {
      public:
        Node *left;
        Node *right;
        iterator_type kv;

        Node(const Key k, const Value v) : left(nullptr), right(nullptr), kv(k, v) { };
        Node(const Node& n) : left(n.left ? new Node(*(n.left)) : nullptr),
                              right(n.right ? new Node(*(n.right)) : nullptr),
                              kv(n.kv) {};
    };

    size_t my_size;
    Node *root;

    /** 
      * @brief Returns a reference to the node with a matching key
      * @param search Where to start the search in the tree
      * @param key The key to search for
      * @return The node searched for, if found, otherwise NULL
      */
    Node*& fetchNode(Node* const* search, const Key& key) const {
      while ((*search) != nullptr) {
        if (key < (*search)->kv.key())
          search = &(*search)->left;
        else if (key > (*search)->kv.key())
          search = &(*search)->right;
        else {
          return const_cast<Node*&>(*search);
        }
      }
      return const_cast<Node*&>(*search);
    }

    /** 
      * @brief Insert a node into the given tree
      * @param search The tree to insert the node into
      * @param node The node to insert
      * @sa fetchNode
      * This first looks for the where the node SHOULD be in the tree.
      * It will of course fail, but this is the location to place the node.
      */
    inline void insertNode(Node** search, Node* const node) {
      Node*& insertAt = fetchNode(search, node->kv.key());
      insertNode(insertAt);
    }

    inline void insertNode(Node*& insertAt, Node* const node) {
      insertAt = node;
      ++my_size;
    }

    /**
      * @brief Delete a node from the tree. (possibly recursive)
      * @param node A reference to the node to delete.
      */
    void deleteNode(Node*& node) {
      if (node == nullptr)
        return;
      if (node->left == nullptr) {
        Node* temp = node->right;
        delete node;
        node = temp;
        --my_size;
      } else if (node->right == nullptr) {
        Node* temp = node->left;
        delete node;
        node = temp;
        --my_size;
      } else {
        //Two children, find max of left subtree and swap
        Node** temp = &(node->left);

        while ((*temp)->right != nullptr)
          temp = &(*temp)->right;
        node->kv = (*temp)->kv;
        deleteNode(*temp);
      }
    }
  public:
    /** 
      * @brief Checks whether the given key is in the tree
      * @param key The key to search for
      * @return true/false depending on found/not-found
      */
    bool contains(const Key& key) const {
      if (isEmpty())
        return false;
      if (fetchNode(&root, key))
        return true;
      return false;
    }

    /**
      * @brief Returns the current size of the tree
      * @return The current size of the tree
      */
    inline size_t size() const { return my_size; };
    inline bool isEmpty() const { return size() == 0; };
    inline explicit operator bool() const { return !isEmpty(); };

  public:
    BinaryTree() : my_size(0), root(nullptr) {};
    BinaryTree(const BinaryTree& tree) : my_size(tree.my_size), root(new Node(*(tree.root))) {};
    BinaryTree(BinaryTree&& tree) : my_size(std::move(tree.my_size)), root(std::move(tree.root)) {
      tree.root = nullptr;
      tree.my_size = 0;
    };

    friend void swap(BinaryTree& a, BinaryTree& b) {
      using std::swap;

      swap(a.my_size, b.my_size);
      swap(a.root, b.root);
    }

    BinaryTree& operator=(BinaryTree tree) {
      swap(*this, tree);
      return *this;
    }

    /**
      * @brief Destructor
      * @sa deleteNode
      * This simply deletes the root node over and over until the root is NULL.
      * There may possibly be a quicker way that does not use recursion (deleteNode does) but this seems the simplest.
      */
    virtual ~BinaryTree() { clear(); };

    /**
      * @brief insert Key/Value pair into tree
      * @param key The key to be inserted (or replaced)
      * @param value The value to be inserted (or replaced)
      * @post The tree's size is increased by 1 if the element was not already in the tree
      */
    bool insert(const Key& key, const Value& value) {
      Node*& rNode = fetchNode(&root, key);
      if (rNode)
        rNode->kv = iterator_type(key, value);
      else {
        Node *newNode = new Node(key, value);
        insertNode(rNode, newNode);
      }
      return true;
    }

    /**
      * @brief Associate array type accessor (rvalue)
      * @sa getValue
      * @param key The key to search for
      */
    inline const Value operator[](const Key& key) const { return getValue(key); }

    /**
      * @brief Associate array type accessor (lvalue)
      * @param key The key to search for
      * @sa insert
      * If the key is not in the tree, it is inserted, and the value set to the rvalue given.
      */
    Value& operator[](const Key& key) {
      Node*& node = fetchNode(&root, key);
      if (!node)
        insertNode(node, new Node(key, Value()));
      //The key was inserted at the return node's address! node is now guranteed to be NON-NULL ! */
      return node->kv.v;
    }

    /** 
      * @brief Remove the given key (and its value) from the tree
      * @param key The key to be searched/removed
      * @return Whether or not the key was removed
      */
    bool remove(const Key& key) {
      Node*& node = fetchNode(&root, key);

      if (node != nullptr) {
        deleteNode(node);
        return true;
      }
      return false;
    }

    inline void clear() {
      while (root != nullptr)
        deleteNode(root);
    }

    /**
      * @brief Find a value in the tree from the key given
      * @param key The key to search for
      * @return The value of the key searched for, or NULL if not found
      */
    Value getValue(const Key& key) const {
      const Value empty;
      if (isEmpty()) return empty;

      Node* node = fetchNode(&root, key);
      if (node)
        return node->kv.value();
      return empty;
    }

#ifdef DISABLED
    /**
      * @brief Find the first key from the given value in the tree
      * @param key The value to search for
      * @return The key of the value searched for, or NULL if not found
      */
    Value getKey(const Value& value) const {
      const Key empty;
      if (isEmpty()) return empty;

      for (iterator iter = begin(); iter; ++iter) {
        if (iter->value() == value)
          return iter->key();
      }
      return empty;
    }
#endif

  private:
    class BinaryTreeIterator : public Iterator<iterator_type> {
      friend class BinaryTree;
      typedef BinaryTree<Key, Value> Tree;
      private:
        Tree* tree;
        size_t index;
        size_t my_size;
        bool reverse;
        iterator_type* storage;

        void fillArray(int& i, const Node* node) {
          if (node == nullptr) 
            return;
          fillArray(i, node->left);
          storage[i++] = iterator_type(node->kv);
          fillArray(i, node->right);
        }

        BinaryTreeIterator(Tree* t, Node* node, size_t size, bool _reverse = 0) : Iterator<iterator_type>(),
                                                                          tree(t),
                                                                          index(_reverse ? size - 1 : 0),
                                                                          my_size(size), 
                                                                          reverse(_reverse),
                                                                          storage(new iterator_type[size]) {
          int i = 0;
          fillArray(i, node);
        }
        //BinaryTreeIterator& operator=(const BinaryTreeIterator&); ///<Block implicit copy constructor
      public:
        BinaryTreeIterator(const iterator& iter) : Iterator<iterator_type>(), 
                                                   tree(iter.tree),
                                                   index(iter.index), 
                                                   my_size(iter.my_size), 
                                                   reverse(iter.reverse),
                                                   storage(new iterator_type[iter.my_size]) {
          for (size_t i = 0; i < my_size; ++i)
            storage[i] = iter.storage[i];
        }

        BinaryTreeIterator() : Iterator<iterator_type>(),
                               tree(nullptr),
                               index(0),
                               my_size(0),
                               reverse(0),
                               storage(nullptr) {
        };

        iterator& operator=(const iterator& iter) {
          if (&iter != this) {
            if (my_size != iter.my_size) {
              delete[] storage;
              storage = new iterator_type[iter.my_size];
            }

            tree = iter.tree;
            index = iter.index;
            my_size = iter.my_size;

            for (size_t i = 0; i < my_size; ++i)
              storage[i] = iter.storage[i];
          }
          return *this;
        }

        virtual ~BinaryTreeIterator() { delete[] storage; }

        virtual void remove() {
          tree->remove(storage[index].key());
/*
          --my_size;
          for (int i = index; i < my_size; ++i)
            storage[i] = storage[i + 1];
          //The iterator is 'invalid' now, but the index is shifter down in case iter++ is called
          --index;
*/
        }

        virtual explicit operator bool() const {
          return ((reverse && index > 0) || (!reverse && index < (my_size)));
        };

        virtual operator iterator_type() { return operator*(); };
        virtual iterator_type& operator*(){ return storage[index]; }

        /**
         * @brief Postfix increment
         */
        virtual iterator operator++(int) {
          iterator tmp(*this);
          ++index;
          return tmp;
        }

        /**
         * @brief Prefix increment
         */
        virtual iterator& operator++() {
          ++index;
          return *this;
        }

        /**
         * @brief Postfix decrement
         */
        virtual iterator operator--(int) {
          iterator tmp(*this);
          --index;
          return tmp;
        }

        /**
         * @brief Prefix decrement
         */
        virtual iterator& operator--() {
          --index;
          return *this;
        }
    };
  public:
    inline iterator begin() { return iterator(this, root, my_size); };
    inline iterator end() { return iterator(this, root, my_size, 1); };

};

BDLIB_NS_END
#endif /* _BD_BINARYTREE_H */ 
/* vim: set sts=2 sw=2 ts=8 et: */
