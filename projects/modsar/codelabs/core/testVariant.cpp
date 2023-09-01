#include "ara/core/variant.h"
#include <stdio.h>
int main()
{
    ara::core::Variant<int,double,float> v(100);
    ara::core::Variant<int> aa(10);
   double a = 100.0 ;
    float a = 1000.0;
    (float )(100.0);
    printf("%d\n",v.index());

    v = (1.2);
   // v.set<2>(11);
    

    printf("%d:%f\n",v.index(),*v.get_at_index<1>());

    return 0;
}