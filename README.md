# MiniRedis
实现的主要功能以及接口如下：<br>
1.insertElement：插入元素（键值对） <br>
2.deleteElement： 删除元素<br>
3.searchElement：根据key搜索元素 <br>
4.displayList：打印跳表 <br>
5.dumpFile：持久化操作，数据落盘 <br>
6.loadFile：加载文件数据，以跳表结构存储在内存 <br>
7.size：跳表的大小（键值对数目） <br>

#
跳表的结构示意图如下:<br>
![image](https://user-images.githubusercontent.com/73992103/196988027-6b841097-01e5-4d4f-91fb-334a2ac623e7.png)

#
跳表的核心解释：<br>
在跳表的所有节点中，key是唯一的，并且每个节点都是唯一的，不会因为所谓的索引而存在拷贝<br>
跳表的索引是由node节点的forward数组实现，数组下标表示索引所在的层数，遍历每一层索引只需要根据控制i去取节点的forward[i]就可以<br>
正如前面所说的那样，节点是唯一的，forwward数组控制节点在每一层索引中对下一个节点的指向<br>
理解了以上三点，才能理解搜索的过程，即从最顶层一层一层向下搜索，每一层控制节点向右走几步<br>
注意每一层的横向遍历不是遍历数组，而是控制current跳到哪个节点，所以不需要每一层所以都从头开始遍历，遍历只是为了控制节点要不要跳，跳几步的问题<br>
这就是是跳表索引快速查找的核心所在<br>
