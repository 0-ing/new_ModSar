#include "ara/core/optional.h"

int main()
{
    ara::core::Optional<int> v;
    v = 100;
    printf("init:%d value:%d",v.has_value(),*v);
}