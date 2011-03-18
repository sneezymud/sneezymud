#include <stdio.h>
#include <stdlib.h>

void main(void)
{
  FILE *fp;
  int i, j = 50;

  fp = fopen("./tmp", "w+");

  for (j = 0; j < 260; j += 10) {
    fprintf(fp, "%3d: ", j);
    for (i = 0; i < 10; i++)
      fprintf(fp, "%c", i+j);
    fprintf(fp, "\n");
  }
  fclose(fp);
}
