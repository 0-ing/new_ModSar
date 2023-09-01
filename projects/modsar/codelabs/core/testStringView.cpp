#include "ara/core/string_view.h"
#include <stdio.h>

int main()
{
    ara::core::StringView sv("hello,world",5);
    for (auto it = sv.begin(); it != sv.end();++it) {
        printf("%c ",*it);
    }
    printf("\n");
    for (auto rit = sv.rbegin(); rit != sv.rend();++rit)
    {
        printf("%c ",*rit);
    }
    printf("\n");
}