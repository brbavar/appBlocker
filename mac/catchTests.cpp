#define CATCH_CONFIG_MAIN
#include "../catch.hpp"
#include <vector>
#include <string>
#include <algorithm>

std::vector<std::string> getListItems(std::string);

std::vector<std::string> getListItems(std::string list) {
    if (list.find('\n') == std::string::npos) {
        if(list.size())
            return { list };
        return {};
    }

    std::vector<std::string> items;
    std::string item = "";
    for (int i = 0; i <= list.size(); i++)
        if ((i == list.size() || list[i] == '\n') && item != "") {
            items.push_back(item);
            item = "";
        }
        else
            item += list[i];
    return items;
}

TEST_CASE("List items are stored in vector as separate elements", "[listItems]") {
    std::vector<std::string> v1 {"Item 1", "Item 2", "Item 3"};
    std::vector<std::string> v2 {"\\n"};
    std::vector<std::string> v3 {"\\n", "kj&(#)837"};
    REQUIRE(getListItems("Item 1\nItem 2\nItem 3") == v1);
    REQUIRE(getListItems("\\n\n") == v2);
    REQUIRE(getListItems("\\n\nkj&(#)837") == v3);
}

