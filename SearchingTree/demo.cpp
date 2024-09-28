#include <iostream>
#include "SearchingTree.h"

signed main()
{
    SearchingTree<int, int> st;
    st.insert(1,0);
    st.insert(5,0);
    st.insert(2,0);
    st.insert(10,0);
    st.insert(4,0);
    st.insert(5,1);
 
    auto it = st.find(5);
    it->second = 10;

    st.erase(1);

    for (auto &[k, v] : st.range(2, 10)) {
        std::cout << k << ' ' << v << std::endl;
    }

    for (auto &[k,v] : st) {
        std::cout << k << ' ' << v << std::endl;
    }
}