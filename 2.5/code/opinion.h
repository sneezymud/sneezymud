

int FreeFears( struct char_data *ch);
int FreeHates( struct char_data *ch);

int RemHated( struct char_data *ch, struct char_data *pud); 
int AddHated( struct char_data *ch, struct char_data *pud); 
int AddHatred( struct char_data *ch, int parm_type, int parm);
int RemHatred( struct char_data *ch, unsigned short bitv);
int Hates( struct char_data *ch, struct char_data *v);

int RemFeared( struct char_data *ch, struct char_data *pud);
int AddFears( struct char_data *ch, int parm_type, int parm);
int AddFeared( struct char_data *ch, struct char_data *pud);
int RemFears( struct char_data *ch, unsigned short bitv);
int Fears( struct char_data *ch, struct char_data *v);



struct char_data *FindAFearee( struct char_data *ch);
struct char_data *FindAHatee( struct char_data *ch);




