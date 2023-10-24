#include <stdio.h>

enum op_type {New_connection = 1, New_post, List_topics, Subscribe_topic, Disconnect};

unsigned getbits(unsigned x, int p, int n)
{
    return (x >> (p+1-n)) & ~(~0 << n);
}

void bin(unsigned n)
{
    unsigned i;
    for (i = 1 << 31; i > 0; i = i / 2)
        (n & i) ? printf("1") : printf("0");
}

void setBit(unsigned number, int p) {
  number |= (1 << p);
  unsigned i;
  for (i = 1 << 31; i > 0; i = i / 2)
    (number & i) ? printf("1") : printf("0");

}

int main(void){
  //printf("%i\n", New_connection);
  bin(10);
  printf("\n%i\n", getbits(10,2,1));
  setBit(10, 0);
  printf("\n");

  return 0;
}