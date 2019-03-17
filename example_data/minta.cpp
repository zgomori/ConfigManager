struct Item{
   uint16_t id;
   char name[20];
   float amount;
};

struct BASE_STRUCTURE{
   uint16_t id;
   uint16_t orderNumber;
   Item items[2];
   char comments[3][10];
};
