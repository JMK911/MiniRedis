#ifndef MINIREDIS_NODE_H
#define MINIREDIS_NODE_H

#include <iostream>

template<typename K,typename V>
class Node{
public:
    Node(){};
    Node(K k, V v, int);
    ~Node();

    //获取节点的key
    K getKey() const;

    //获取节点的value
    V getValue() const;

    //重设节点的value
    void setValue(V v);

    //forward数组存储每一层索引中，节点的下一个指向
    Node<K, V> **forward;

private:
    //节点的值不暴露，只允许通过接口获取、设置
    K key;
    V value;

    //节点索引的层数，控制节点最高出现在第几层索引，并用于forward数组的初始化
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