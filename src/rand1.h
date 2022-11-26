#ifndef HLIDAC_RAND1_H
#define HLIDAC_RAND1_H

void srand1();
int rand1();
int randn1(int size);

void swap_rand1(void* ptr1, void* ptr2, unsigned int size);
void randn1mix(void* ptr, unsigned int size, unsigned int count);

#endif
