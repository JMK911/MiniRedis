#ifndef MINIREDIS_NODE_H
#define MINIREDIS_NODE_H

#include <iostream>

template<typename K,typename V>
class Node{
public:
    Node(){};
    Node(K k, V v, int);
    ~Node();

    K getKey() const;
    V getValue() const;
    void setValue(V v);

    Node<K, V> **forward;
    //int node_level;

private:
    K key;
    V value;
    int node_level;
};

template<typename K,typename V>
Node<K,V>::Node(K k,V v,int level){
    this->key = k;
    this->value = v;
    this->node_level = level;
    this->forward = new Node<K,V>* [level + 1];
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
}

template<typename K,typename V>
Node<K,V>::~Node(){
    delete[] forward;
}

template<typename K,typename V>
K Node<K,V>::getKey() const{
    return this->key;
}

template<typename K,typename V>
V Node<K,V>::getValue() const{
    return this->value;
}

template<typename K,typename V>
void Node<K,V>::setValue(V v){
    this->value = v;
}
#endif