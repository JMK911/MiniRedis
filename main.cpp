#include "node.h"
#include "skipList.h"

int main(){
    skipList<int,int> sp(10);
    sp.insert_element(1, 1);
    sp.insert_element(1, 2);
    sp.insert_element(2, 3);
    sp.insert_element(3, 4);
    sp.insert_element(4, 5);
    sp.insert_element(5, 6);
    sp.display_list();
    sp.delete_element(2);
    std::cout << "--" << std::endl;
    sp.display_list();
    sp.dump_file();
    std::cout << "--" << std::endl;
    skipList<std::string, std::string> sp1(10);
    sp1.load_file();
    sp1.display_list();
    system("pause");
    return 0;
}