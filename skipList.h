#ifndef MINIREDIS_SKIPILIST_H
#define MINIREDIS_SKIPILIST_H

#include <iostream>
#include <mutex>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "node.h"

std::mutex mtx;
std::string delimiter = ":";
#define STORE_FILE "store/dumpFile.txt"

template<typename K,typename V>
class skipList{
public:
    skipList(int);
    ~skipList();
    int insert_element(K k, V v);
    bool search_element(K k);
    void delete_element(K k);
    void display_list();
    void dump_file();
    void load_file();
    int size();

private:
    Node<K, V> *create_node(K, V, int);
    int get_random_level();
    void get_key_value_from_string(const std::string& str,std::string* key,std::string* value);
    bool is_valid_stirng(const std::string& str);
    void destroy(Node<K,V>*);

private:
    Node<K, V>* _header;
    int _max_level;
    
    //当前的所有节点中的最高层，随着插入动态变化
    int _cur_level;
    int _element_count;
    std::ifstream _file_reader;
    std::ofstream _file_writer;
};

template<typename K,typename V>
skipList<K,V>::skipList(int maxlevel){
    this->_max_level = maxlevel;
    this->_cur_level = 0;
    this->_element_count = 0;

    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template<typename K,typename V>
skipList<K,V>::~skipList(){
    if(_file_writer.is_open()){
        _file_writer.close();
    }
    if(_file_reader.is_open()){
        _file_reader.close();
    }
    destroy(_header);
}

template<typename K,typename V>
void skipList<K,V>::destroy(Node<K,V>* head){
    if(head==NULL){
        return;
    }
    destroy(head->forward[0]);
    delete head;
}

template<typename K,typename V>
int skipList<K,V>::insert_element(K key,V value){
    mtx.lock();
    Node<K,V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0,sizeof(Node<K,V>*)*(_max_level+1));

    for (int i = _cur_level; i >= 0;i--){
        while (current->forward[i] != NULL &&current->forward[i]->getKey()<key){
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if(current!=0&&current->getKey()==key){
        std::cout<<"KEY:"<<key<<",exists!"<<std::endl;
        current->setValue(value);
        mtx.unlock();
        return 1;
    }

    int random_level = get_random_level();
    if(random_level>_cur_level){
        for (int i = _cur_level + 1; i < random_level + 1;++i){
            update[i] = _header;
        }
        _cur_level = random_level;
    }
    Node<K, V> *insertNode = create_node(key, value, random_level);
    for (int i = 0; i <= random_level;++i){
        insertNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = insertNode;
    }
    _element_count++;
    mtx.unlock();
    return 0;
}

template<typename K,typename V>
bool skipList<K,V>::search_element(K key){
    std::cout << "search_element:" << std::endl;
    Node<K, V> *current = _header;
    for (int i = _cur_level; i >= 0;--i){
        while (current->forward[i] != NULL &&current->forward[i]->getKey()<key){
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if (current != 0&&current->getKey()==key){
        std::cout << "Found key:value :" << key << ":" << current->getValue() << std::endl;
        return true;
    }
    return false;
}

template<typename K,typename V>
void skipList<K,V>::delete_element(K key){
    mtx.lock();
    Node<K, V> *current = _header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));
    for (int i = _cur_level; i >= 0;i--){
        while(current->forward[i]!=NULL&&current->forward[i]->getKey()<key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if (current != NULL&&current->getKey()==key){
        for (int i = 0; i <= _cur_level;++i){
            if(update[i]->forward[i]!=current){
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        while(_cur_level>0&&_header->forward[_cur_level]==NULL){
            _cur_level--;
        }
        _element_count--;
        std::cout << "Find and successfully delete key: " << key << std::endl;
    }
    mtx.unlock();
    return;
}

template<typename K,typename V>
void skipList<K,V>::display_list(){
    for (int i = 0; i <= _cur_level;++i){
        Node<K, V> *current=_header->forward[i];
        std::cout << "Level " << i << ": ";
        while(current!=NULL){
            std::cout << current->getKey() << ":" << current->getValue() << "\t";
            current = current->forward[i];
        }
        std::cout << std::endl;
    }
}

template<typename K,typename V>
void skipList<K,V>::dump_file(){
    std::cout << "write to file!" <<std::endl;
    _file_writer.open(STORE_FILE);//savepath.c_str(),转为const char*
    Node<K, V> *cur = this->_header->forward[0];
    while(cur!=NULL){
        _file_writer << cur->getKey() << ":" << cur->getValue() <<"\n";
        cur = cur->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

template<typename K,typename V>
void skipList<K,V>::load_file(){
    std::cout << "load file!" <<std::endl;
    _file_reader.open(STORE_FILE);
    std::string line;
    std::string *key=new std::string();
    std::string *value = new std::string();
    while (getline(_file_reader,line)){
        get_key_value_from_string(line, key, value);
        if(key->empty()||value->empty()){
            continue;
        }
        insert_element(*key, *value);
    }
    _file_reader.close();
    return;
}

template<typename K,typename V>
void skipList<K,V>::get_key_value_from_string(const std::string& str,std::string* key,std::string* value){
    if(!is_valid_stirng(str)){
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
    return;
}

template<typename K,typename V>
bool skipList<K,V>::is_valid_stirng(const std::string& str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter)==std::string::npos){
        return false;
    }
    return true;
}

template<typename K,typename V>
int skipList<K,V>::size(){
    return _element_count;
}

template<typename K,typename V>
Node<K, V>* skipList<K,V>::create_node(K k, V v,int level){
    Node<K, V> *node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K,typename V>
int skipList<K,V>::get_random_level(){
    int k = 1;
    while (rand() % 2) {//没有随机数种子，导致每次生成的数都一样。
        k++;
    }
    k = (k < _max_level) ? k : _max_level;//最高层数限制
    return k;
}

#endif