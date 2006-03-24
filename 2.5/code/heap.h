/*
**  Heap data structs
*/

struct StrHeapList {
   char *string;   /* the matching string */
   int  total;     /* total # of occurences */
};

struct StrHeap {
   int uniq;   /* number of uniq items in list */
   struct StrHeapList *str;   /* the list of strings and totals */
};
