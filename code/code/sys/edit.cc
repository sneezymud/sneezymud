///////////////////////////////////////////////////////////////////////////////
//
//   edit.cc : All routines related to vt100 editor.                        
//
//   Coded by : Russ Russell, June 1995, Last update June 10th 1995
//
///////////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "socket.h"

#include <unistd.h>
#include <arpa/telnet.h>
#include <sys/syscall.h>
#include <netdb.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>

int Descriptor::move(int hor, int vert)
{
  char buf[256];

  if (edit.x < 1 || edit.y < 1) {
    edit.x = (edit.x < 1) ? 1 : edit.x;
    edit.y = (edit.y < 1) ? 1 : edit.y;
    beep();
    return FALSE;
  }
  sprintf(buf, VT_CURSPOS, vert, hor);
  writeToQ(buf);
  return TRUE;
}

bool Descriptor::isEditing()
{
  return (connected == CON_EDITTING);
}

void Descriptor::Edit(char *arg)
{
  char buf[256] = "\0";
  char test[] = {IAC, WONT, '\x03', '\0'};
  char test2[] = {IAC, DO, '\x18', '\0'};

  if (!*arg || !arg)
    return;

  while (*arg) {
    switch (*arg) {
      case '\033':
        switch (*(arg + 1)) {
          case '[':
            switch (*(arg + 2)) {
              case 'A':   // Up Arrow
                move(edit.x, --edit.y);
                arg += 2;
                break;
              case 'B':
                move(edit.x, ++edit.y);
                arg += 2;
                break;
              case 'C':
                move(++edit.x, edit.y);
                arg += 2;
                break;
              case 'D':
                move(--edit.x, edit.y);
                arg += 2;
                break;
              default:
                beep();
                break;
            }
            break;
          default:
            break;
        }
        break;
      case '\177':
      case ('H' & 037):
        if (move(--edit.x, edit.y)) {
          sprintf(buf, "%c", ' ');
          writeToQ(buf);
          move(edit.x, edit.y);
        }
        break;
      case ('X' & 037):
        connected = CON_PLYNG;
  
        EchoOn();
        write(socket->m_sock, test, 4);
        write(socket->m_sock, test2, 4);
        break;
      case ('M' & 037):
        writeToQ("\n\r");
        edit.y++;
        edit.x = 1;
        break;
      default:
	//        if (isascii(*raw) || isprint(*raw) || iscntrl(*raw)) {
        if (isascii(*buf) || isprint(*buf) || iscntrl(*buf)) {
          sprintf(buf, "%c", *arg);
          writeToQ(buf);
          move(++edit.x, edit.y);
        }
        break;
    }
    arg++;
  }
  arg[0] = '\0';
}
