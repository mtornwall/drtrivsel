#include<stdio.h>
#include"fet.h"


FILE *columns(void){
  return popen("column -x | more", "w");
  }
