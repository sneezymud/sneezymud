//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: mail.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __MAIL_H
#define __MAIL_H

const int MIN_MAIL_LEVEL=2;
const int STAMP_PRICE=50;
const int MAX_MAIL_SIZE=4000;
const int NAME_SIZE =15;

#ifdef OSF
const int BLOCK_SIZE=104;
#else
const int BLOCK_SIZE=100;
#endif

const int INT_SIZE  =sizeof(int);
const int CHAR_SIZE =sizeof(char);
const int LONG_SIZE =sizeof(long);
 
const unsigned int HEADER_BLOCK_DATASIZE =(BLOCK_SIZE-1-((CHAR_SIZE*(NAME_SIZE+1)*2)+(3*LONG_SIZE)));
 
const int DATA_BLOCK_DATASIZE =(BLOCK_SIZE-LONG_SIZE-1);
 
const int HEADER_BLOCK  =-1;
const int LAST_BLOCK    =-2;
const int DELETED_BLOCK =-3;
 
typedef struct header_block_type_d
{
  long block_type;/* is this a header block or data block? */
  long next_block;/* if header block, link to next block   */
		/* note: next_block is part of header_blk*/
		/* in a data block; we can't combine them*/
		/* here because we have to be able to    */
		/* differentiate a data block from a     */
		/* header block when booting mail system */
  char from[NAME_SIZE+1]; /* who is this letter from?*/
  char to[NAME_SIZE+1];/* who is this letter to?	*/
  time_t mail_time;	/* when was the letter mailed?	*/
  char txt[HEADER_BLOCK_DATASIZE+1]; /* the actual text*/
} header_block_type;
 
typedef struct data_block_type_d
{
  long block_type; 
  char txt[DATA_BLOCK_DATASIZE+1]; /* the actual text		 */
} data_block_type;
 
class position_list_type
{
  public:
    long position;
    position_list_type *next;

    position_list_type();
    ~position_list_type();
};
 
class mail_index_type
{
  public:
    char recipient[NAME_SIZE+1]; /* who the mail is for */
    position_list_type *list_start;  /* list of mail positions    */
    mail_index_type *next;

    mail_index_type();
    ~mail_index_type();
};

extern bool has_mail(const char *);
extern bool scan_file(silentTypeT);
extern bool no_mail;

extern void autoMail(TBeing *, const char *, const char *);
extern char *read_delete(const char *recipient, const char *recipient_formatted);

#endif
