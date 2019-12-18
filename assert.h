#ifndef BLAU_ASSERT_H
#define BLAU_ASSERT_H
#define assert(x) if(!(x)){kprintf("%s\n",#x);panic();}
#endif