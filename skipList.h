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

//mutex锁
std::mutex mtx;

//字符串分隔符，用于从字符串中区分key和value
std::string delimiter = ":";

//文件的存储路径
#define STORE_FILE "store/dumpFile.txt"

//跳表
template<typename K,typename V>
class skipList{
public:
    skipList(int);
    ~skipList();

    //插入键值对
    int insert_element(K k, V v);

    //搜索键值对
    bool search_element(K k);

    //删除键值对
    void delete_element(K k);

    //跳表的打印
    void display_list();

    //跳表存储数据的落盘存储操作
    void dump_file();

    //从文件读取数据生成跳表结构
    void load_file();

    //获取跳表的大小，即键值对数目
    int size();

private:
    //创建节点
    Node<K, V> *create_node(K, V, int);

    //获取节点索引的随机层数
    int get_random_level();

    //从字符串中获取并分离键值对
    void get_key_value_from_string(const std::string& str,std::string* key,std::string* value);

    //判断字符串符不符合键值对格式要求
    bool is_valid_stirng(const std::string& str);

    //递归销毁所有节点
    void destroy(Node<K,V>*);

private:
    //不存值的头结点，方便每一层的遍历
    Node<K, V>* _header;

    //设定跳表的最大层数
    int _max_level;
    
    //当前的所有节点中的最高层，随着插入动态变化
    int _cur_level;

    //跳表存储的键值对数目
    int _element_count;

    //文件操作
    std::ifstream _file_reader;
    std::ofstream _file_writer;
};

//跳表初始化
template<typename K,typename V>
skipList<K,V>::skipList(int maxlevel){
    this->_max_level = maxlevel;
    this->_cur_level = 0;
    this->_element_count = 0;

    //初始化头结点
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

//跳表析构
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

//递归销毁跳表节点
template<typename K,typename V>
void skipList<K,V>::destroy(Node<K,V>* head){
    if(head==NULL){
        return;
    }
    destroy(head->forward[0]);
    delete head;
}

//插入键值对
template<typename K,typename V>
int skipList<K,V>::insert_element(K key,V value){
    //加锁
    mtx.lock();

    //currrent节点用于跳表的遍历，初始化为头结点
    Node<K,V> *current = this->_header;

    //update节点数组，用于保存插入位置的前一个位置，注意下标表示的索引的层数
    Node<K, V> *update[_max_level + 1];

    //update初始化
    memset(update, 0,sizeof(Node<K,V>*)*(_max_level+1));

    //核心部分，注意是从跳表当前的最高层数往下遍历，用于确认非头结点与插入节点的关系
    for (int i = _cur_level; i >= 0;i--){
        
        //跳出循环时，current的下一个节点是第一个大于等于目标key的，保证了当前的current就是目标插入位置的前一个位置
        while (current->forward[i] != NULL &&current->forward[i]->getKey()<key){
            current = current->forward[i];
        }

        //update记录每一层索引插入前的位置
        update[i] = current;
    }

    //第0层包含了所有的节点，current经过上述的循环，现在也处在第0层，现在的current是目标插入位置
    current = current->forward[0];

    //如果目标位置存在则直接赋值
    if(current!=0&&current->getKey()==key){
        std::cout<<"KEY:"<<key<<",exists!"<<std::endl;
        current->setValue(value);
        
        //解锁并结束函数
        mtx.unlock();
        return 1;
    }

    //如果目标位置不存在目标键的节点，则需要创建节点
    //获取节点的索引层数
    int random_level = get_random_level();

    //如果节点的索引层数大于跳表当前所处的最高层，需要将update数组高于_cur_level的部分设置为头结点
    //因为跳表高于_cur_level的部分，很明显没有其他节点,所以插入位置的前一位置只能是头结点，此处体现了创建空值的头结点的好处
    if(random_level>_cur_level){
        for (int i = _cur_level + 1; i < random_level + 1;++i){
            update[i] = _header;
        }

        //更新跳表当前所处的最高层数
        _cur_level = random_level;
    }

    //创建节点
    Node<K, V> *insertNode = create_node(key, value, random_level);

    //用update数组更新跳表内的节点关系
    for (int i = 0; i <= random_level;++i){
        insertNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = insertNode;
    }

    //跳变元素数目更新
    _element_count++;
    mtx.unlock();
    return 0;
}

//根据key搜索对应value
template<typename K,typename V>
bool skipList<K,V>::search_element(K key){
    std::cout << "search_element:" << std::endl;
    
    //初始化curren节点为头结点用于遍历
    Node<K, V> *current = _header;

    //搜索过程是从最高层_cur_level一层一层往下搜索，向右遍历
    for (int i = _cur_level; i >= 0;--i){
        
        //搜索的遍历过程和插入一样，都是遍历到目标key的前一个位置
        while (current->forward[i] != NULL &&current->forward[i]->getKey()<key){
            current = current->forward[i];
        }
    }

    //详细讲解一下跳表中的节点，在跳表的所有节点中，key是唯一的，并且每个节点都是唯一，不会因为所谓的索引而存在拷贝
    //跳表的索引是由node节点的forward数组实现，数组下标表示索引所在的层数，遍历每一层索引只需要根据控制i去取节点的forward[i]就可以
    //正如前面所说的那样，节点是唯一的，forwward数组控制节点在每一层索引中对下一个节点的指向
    //理解了以上三点，才能理解搜索的过程，即从最顶层一层一层向下搜索，每一层控制节点向右走几步
    //注意每一层的横向遍历不是遍历数组，而是控制current跳到哪个节点，所以不需要每一层所以都从头开始遍历，遍历只是为了控制节点要不要跳，跳几步的问题
    //这才是跳表索引快速查找的核心所在

    //current循环后的位置在第0层，且第0层拥有所有的节点；current移动到目前位置
    current = current->forward[0];

    //如果curre的key是目标key则找到，否则就是不存在目标key返回false
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

//打印跳表
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

//持久化操作，跳表存储的键值对落盘
template<typename K,typename V>
void skipList<K,V>::dump_file(){
    std::cout << "write to file!" <<std::endl;

    //打开目标文件
    _file_writer.open(STORE_FILE);
    
    //第0层能遍历所有节点
    Node<K, V> *cur = this->_header->forward[0];
    while(cur!=NULL){
        _file_writer << cur->getKey() << ":" << cur->getValue() <<"\n";
        cur = cur->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

//加载目标文件，形成跳表结构，用于内存缓存
template<typename K,typename V>
void skipList<K,V>::load_file(){
    std::cout << "load file!" <<std::endl;
    _file_reader.open(STORE_FILE);
    std::string line;
    std::string *key=new std::string();
    std::string *value = new std::string();

    //从文件中一行一行读取键值对到字符串line中
    while (getline(_file_reader,line)){
        
        //从line中分割出key和value
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
    
    //判断字符串是否符合键值对格式
    if(!is_valid_stirng(str)){
        return;
    }

    //str.find(delimiter)是分割符的位置而substr的输入是（起始位置，字符串长度）
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
    return;
}

template<typename K,typename V>
bool skipList<K,V>::is_valid_stirng(const std::string& str){
    if(str.empty()){
        return false;
    }

    //字符串中没有找到分割符
    if(str.find(delimiter)==std::string::npos){
        return false;
    }
    return true;
}

//获取跳表存储的键值对数目
template<typename K,typename V>
int skipList<K,V>::size(){
    return _element_count;
}

//创建目标节点
template<typename K,typename V>
Node<K, V>* skipList<K,V>::create_node(K k, V v,int level){
    Node<K, V> *node = new Node<K, V>(k, v, level);
    return node;
}

//获取随机层数，用于决定目标节点的索引层数
template<typename K,typename V>
int skipList<K,V>::get_random_level(){
    int k = 1;

    //没有随机数种子，导致每次生成的数都一样。
    while (rand() % 2) {
        k++;
    }

    //最高层数限制
    k = (k < _max_level) ? k : _max_level;
    return k;
}

#endif