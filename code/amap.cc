#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<map>

#define MAXROOMS 50000

//#define MINLEVEL -20
//#define MAXLEVEL 10

#define MAXCOORDITER 1000

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define UP 4
#define DOWN 5
#define NORTHEAST 6
#define NORTHWEST 7
#define SOUTHEAST 8
#define SOUTHWEST 9

#define CELLSIZE 3
#define SCALEBY 2

const char *dirstrings[]={"north", "east", "south", "west", "up", "down",
		    "northeast", "northwest", "southeast", "southwest"};

int opposite[]={SOUTH, WEST, NORTH, EAST, DOWN, UP, SOUTHWEST, SOUTHEAST,
		NORTHWEST, NORTHEAST};

int sector_colors[66][3]={
  {255,255,255},  // arctic
  {255,255,255},  //
  {255,255,255},  //
  {255,255,255},  //
  {255,255,255},  //
  {86,53,13},     // mountain
  {0,128,0},      // forest
  {0,128,0},      // marsh
  {0,0,255},      // river
  {255,255,255},  //
  {255,255,102},  // beach
  {255,255,255},  //
  {255,255,255},  //
  {51,51,51},     // cave
  {255,255,255},  //
  {255,255,255},  //
  {255,255,255},  //

  {255,255,255},  // unused
  {255,255,255},  // unused
  {255,255,255},  // unused

  {255,255,255},       // plains
  {86,53,13},
  {255,255,255},
  {0,128,0},      // grasslands
  {86,53,13},     // hills
  {86,53,13},     // mountains
  {0,128,0},      // forest
  {0,128,0},      // swamp
  {0,51,255},      // ocean
  {0,51,255},      // river surface
  {0,51,255},      // underwater
  {255,255,102},   // beach
  {255,255,255},
  {51,51,51},     // cave
  {255,255,255},
  {255,255,255},
  {255,255,255},

  {255,255,255},  // unused
  {255,255,255},  // unused
  {255,255,255},  // unused

  {255,255,102},  // desert
  {86,53,13},     // savannah
  {0,128,0},      // veldt
  {255,255,255},
  {255,255,255},
  {0,128,0},      // jungle
  {0,128,0},      // rainforest
  {86,53,13},     // hills
  {86,53,13},     // mountains
  {255,0,0},      // lava
  {0,128,0},      // swamp
  {0,51,255},      // ocean
  {0,51,255},      // river surface
  {0,51,255},      // underwater
  {255,255,102},  // beach
  {255,255,255},
  {51,51,51},     // cave
  {255,255,255},
  {255,255,255},
  {255,255,255},

  {255,255,255},
  {255,255,255},
  {255,0,0},      // fire
  {255,255,255},
  {255,0,0},      // fire atmosphere
  {255,255,255},
};

class NODE {
public:
  int num, x, y, z, sector;
  int done;
  char name[512];
  int idirs[10];
  NODE *pdirs[10];
  NODE *next;


  NODE();

} *head, *nodes[MAXROOMS];


NODE::NODE()
{
  for(int i=0;i<10;++i){
    idirs[i]=-1;
    x=y=z=done=0;
    pdirs[i]=NULL;
  }
}


map<int,bool>zone_enabled;



/*
#<number>
stuff~
more stuff~
one line of stuff
D[0-9]
stuff~
stuff~
0 0 0 0 0 <number>
S

0=n, 1=e, 2=s, 3=w, 4=u, 5=d 6=ne 7=nw 8=se 9=sw


 */

NODE *find_node(int num){
  if(num>MAXROOMS || num<0) 
    return NULL;

  return(nodes[num]);
}

bool isEnabled(int num)
{
  bool enabled=false;
  map<int,bool>::iterator iter;
  
  for(iter=zone_enabled.begin();iter!=zone_enabled.end();++iter){
    if((*iter).first>num){
      enabled=(*iter).second;
      break;
    }
  }

  return enabled;
}


NODE *read_room(FILE *tiny){
  NODE *tmp=new NODE;
  int dir, room, c, end=0, t;
  char tch;

  if(!tiny){
    fprintf(stderr, "read_room() called with null file handle\n");
    return NULL;
  }

  if(!tmp) 
    return NULL;

  do {
    // read until we get to a new room ('#')
    while((tch=fgetc(tiny))){
      if(tch=='#')
	break;
      if(tch==EOF)
	return NULL;
    }
    
    fscanf(tiny, "%i", &tmp->num);
  } while(!isEnabled(tmp->num));

  fscanf(tiny, "%[^~]~", tmp->name); // name
  while(fgetc(tiny)!='~'); // descr

  // parse sector type
  fscanf(tiny, "\n%i %i %i ", &t, &t, &tmp->sector);
  if(tmp->sector == -1){
    fscanf(tiny, "%i %i %i %i ", &t, &t, &t, &tmp->sector);
  } 

  // 1 8265 -1 500 39 0 21 0 0 0 1000
  // 1 41213 60 0 0 0 100

  while((c=fgetc(tiny)) && !end){
    switch(c){
      case 'E': while(fgetc(tiny)!='~'); while(fgetc(tiny)!='~'); break;
      case 'S': end=1; break;
      case 'D':
	fscanf(tiny, "%i", &dir);
	while(fgetc(tiny)!='~');
	while(fgetc(tiny)!='\n');
	while(fgetc(tiny)!='~');
	while(fgetc(tiny)!='\n');
	while(fgetc(tiny)!=' ');
	while(fgetc(tiny)!=' ');
	while(fgetc(tiny)!=' ');
	while(fgetc(tiny)!=' ');
	while(fgetc(tiny)!=' ');
	fscanf(tiny, "%i\n", &room);
	tmp->idirs[dir]=room;
	break;
    }
  }
  return tmp;
}

void consolidate_nodes(){
  NODE *t;
  int i, j=0;
 
  for(t=head;t;t=t->next){
    printf("\rConsolidating %i", ++j);
    for(i=0;i<10;++i){
      t->pdirs[i]=find_node(t->idirs[i]);
    }
  }
}


int coordinates()
{
  NODE *t;
  int i;
  bool again=false;
  static int count, lastcount;
  int roomsleft=0;

  head->done=1;
  for(t=head;t;t=t->next){
    if(!t->done){
      ++roomsleft;
      for(i=0;i<10;++i){
	if(t->pdirs[i] && !t->pdirs[i]->done)
	  again=true;
      }
      continue;
    } else {
      for(i=0;i<10;++i){
	if(t->pdirs[i] && !t->pdirs[i]->done){
	  t->pdirs[i]->x=t->x;
	  t->pdirs[i]->y=t->y;
	  t->pdirs[i]->z=t->z;
	  switch(i){
  	    case 0: // north
	      t->pdirs[i]->y=t->y+1;
	      break;
	    case 1:  // east
	      t->pdirs[i]->x=t->x+1;
	      break;
	    case 2: // south
	      t->pdirs[i]->y=t->y-1;
	      break;
	    case 3: // west 
	      t->pdirs[i]->x=t->x-1;
	      break;
	    case 4: // up
	      t->pdirs[i]->z=t->z+1;
	      break;
	    case 5: // down
	      t->pdirs[i]->z=t->z-1;
	      break;
	    case 6: // northeast
	      t->pdirs[i]->y=t->y+1;
	      t->pdirs[i]->x=t->x+1;
	      break;
	    case 7: // northwest
	      t->pdirs[i]->y=t->y+1;
	      t->pdirs[i]->x=t->x-1;
	      break;
	    case 8: // southeast
	      t->pdirs[i]->y=t->y-1;
	      t->pdirs[i]->x=t->x+1;
	      break;
	    case 9: // southwest
	      t->pdirs[i]->y=t->y-1;
	      t->pdirs[i]->x=t->x-1;
	      break;
	  }
	  t->pdirs[i]->done=1;
	}
      }
    }
  }

  printf("Coordinate iteration %4i (%i rooms left)\r",
	 ++count, roomsleft);
  fflush(stdout);

  if(count>MAXCOORDITER)
    return false;

  if(lastcount==roomsleft)
    return false;

  lastcount=roomsleft;

  return again;
}

char *itoa(int n){
  int c, i, j, sign;
  char *s=(char *)malloc(256);

  if((sign=n)<0) 
    n=-n;
  i=0;
  do {
    s[i++]=n%10+'0';
  } while((n/=10)>0);
  if(sign<0)
    s[i++]='-';
  s[i]='\0';

  for(i=0, j=strlen(s)-1;i<j;i++,j--){
    c=s[i];
    s[i]=s[j];
    s[j]=c;
  }

  return(s);
}

void check_rooms(){
  int count=0;
  
  for(int i=0;i<MAXROOMS;++i){
    for(int j=i+1;j<MAXROOMS;++j){
      if(nodes[i] && nodes[j] &&
	 nodes[i]->done && nodes[j]->done &&
	 nodes[i]->num != nodes[j]->num &&
	 nodes[i]->x == nodes[j]->x &&
	 nodes[i]->y == nodes[j]->y &&
	 nodes[i]->z == nodes[j]->z){
	printf("%i and %i have same coords (%i, %i, %i)\n", 
	       nodes[i]->num, nodes[j]->num, 
	       nodes[i]->x, nodes[i]->y, nodes[i]->z);
	++count;
      }
    }
  }
  printf("%i rooms have same coords\n", count);

  map<int,bool> done;

  for(int i=0;i<MAXROOMS;++i){
    if(nodes[i]){
      for(int j=0;j<10;++j){
	if(nodes[i]->pdirs[j]){
	  if((!nodes[i]->pdirs[j]->pdirs[opposite[j]] ||
	     (nodes[i]->pdirs[j]->pdirs[opposite[j]]->num !=
	     nodes[i]->num)) && !done[i]){
	    
	    printf("Room %i has a one way exit %i (%i) to room %i\n",
		   nodes[i]->num, j, opposite[j], nodes[i]->pdirs[j]->num);

	    done[i]=true;
	  }
	}
      }
    }
  }

  int dx, dy, dz;
  done.clear();

  for(int i=0;i<MAXROOMS;++i){
    if(nodes[i]){
      for(int j=0;j<10;++j){
	if(nodes[i]->pdirs[j] && !done[i]){
	  dx=abs(nodes[i]->x - (nodes[i]->pdirs[j]->x));
	  dy=abs(nodes[i]->y - (nodes[i]->pdirs[j]->y));
	  dz=abs(nodes[i]->z - (nodes[i]->pdirs[j]->z));

	  if(dx>1 || dy>1 || dz>1){
	    printf("Room %i (%i, %i, %i) has a misaligned link to room %i (%i, %i, %i)\n", nodes[i]->num, nodes[i]->x, nodes[i]->y, nodes[i]->z, nodes[i]->pdirs[j]->num, nodes[i]->pdirs[j]->x, nodes[i]->pdirs[j]->y, nodes[i]->pdirs[j]->z);
	    done[i]=true;
	  }
	}
      }
    }
  }


}




void createmap(int MINLEVEL, int MAXLEVEL){
  NODE *t;
  int minx=0, maxx=0, miny=0, maxy=0, mapsize=0, mapwidth, mapheight, loc;
  int newloc, cellx, celly;
  char *mapdata, buff[256], *newmap;
  int i=0, j, k, l;
  FILE *out=fopen("imageout.raw", "wb");

  for(t=head;t;t=t->next){
    if(t->z<MINLEVEL || t->z>MAXLEVEL) continue;
    if(t->x<minx) minx=t->x;
    if(t->x>maxx) maxx=t->x;
    if(t->y<miny) miny=t->y;
    if(t->y>maxy) maxy=t->y;
  }

  mapwidth=abs(minx-maxx)+1;
  mapwidth*=CELLSIZE;
  mapheight=abs(miny-maxy)+1;
  mapheight*=CELLSIZE;
  mapsize=(mapwidth*mapheight);
  mapsize*=3; // for rgb

  printf("x range=%i,%i, y range=%i,%i, size=%i\n", 
	 minx, maxx, miny, maxy, mapsize);

  mapdata=(char *) calloc(1, mapsize);

  for(t=head;t;t=t->next){
    if(t->z<MINLEVEL || t->z>MAXLEVEL) continue;
    t->x+=abs(minx);
    t->y+=abs(miny);
    cellx=t->x*CELLSIZE;
    celly=t->y*CELLSIZE;
    loc=(mapwidth*celly)+cellx;

    mapdata[loc*3]=sector_colors[t->sector][0];
    mapdata[(loc*3)+1]=sector_colors[t->sector][1];
    mapdata[(loc*3)+2]=sector_colors[t->sector][2];

    //0=n, 1=e, 2=s, 3=w, 4=u, 5=d 6=ne 7=nw 8=se 9=sw
    // map is upside down so north south are reversed
    for(i=0;i<10;++i){
      newloc=loc;
      if(t->pdirs[i]){
	switch(i){
	  case 0: newloc=(mapwidth*(celly+1))+cellx; break;
  	  case 1: newloc=(mapwidth*(celly))+cellx+1; break;
	  case 2: newloc=(mapwidth*(celly-1))+cellx; break;
	  case 3: newloc=(mapwidth*(celly))+cellx-1; break;
	  case 4: break;
	  case 5: break;
	  case 6: newloc=(mapwidth*(celly+1))+cellx+1; break;
	  case 7: newloc=(mapwidth*(celly+1))+cellx-1; break;
	  case 8: newloc=(mapwidth*(celly-1))+cellx+1; break;
	  case 9: newloc=(mapwidth*(celly-1))+cellx-1; break;
	}
	if(newloc!=loc){
	  mapdata[newloc*3]=102;
	  mapdata[(newloc*3)+1]=102;
	  mapdata[(newloc*3)+2]=102;
	}
      }
    }
  }
  
  mapsize=mapsize*SCALEBY*SCALEBY;
  newmap=(char *)malloc(mapsize);
  for(j=0;j<mapheight;++j){
    for(l=0;l<SCALEBY;++l){
      for(i=0;i<mapwidth;++i){
	loc=((mapwidth*j)+i)*3;
	newloc=((mapwidth*((j*SCALEBY)+l))+i)*3;
	for(k=0;k<SCALEBY;++k){
	  newmap[(newloc*SCALEBY)+(k*3)]=mapdata[loc];
	  newmap[(newloc*SCALEBY)+1+(k*3)]=mapdata[loc+1];
	  newmap[(newloc*SCALEBY)+2+(k*3)]=mapdata[loc+2];
	}
      }
    }
  }
  mapwidth*=SCALEBY;
  mapheight*=SCALEBY;

  fwrite(newmap, mapsize, 1, out);
  fclose(out);
  sprintf(buff, "/usr/local/bin/rawtoppm -rgb %i %i imageout.raw | /usr/local/bin/pnmflip -tb | /usr/local/bin/ppmtogif > imageout.gif", mapwidth, mapheight);
  printf("%s\n", buff);
  system(buff);
  system("rm -f imageout.raw");

  free(mapdata);
  free(newmap);
}


void makezonelist(FILE *zone){
  char tch, last=0;
  int num, enabled, start_room;
  char buf[512];

  while(1){
    // read until we get to a new zone ('#')
    while((tch=fgetc(zone))){
      if(last=='\n'){
	if(tch=='#')
	  break;
	if(tch==EOF)
	  return;
      }
      last=tch;
    }
    
    fscanf(zone, "%i", &num);
    fscanf(zone, "%[^~]~", buf); // name
    
    fscanf(zone, "%i %i %i %i", &start_room, &num, &num, &enabled);
    
    zone_enabled[start_room]=enabled?true:false;
  }
}


int main(int argc, char **argv){
  FILE *tiny=fopen("/mud/code/lib/tinyworld.wld", "rt");
  FILE *zone=fopen("/mud/code/lib/tinyworld.zon", "rt");
  NODE *last=NULL, *t;
  int i;

  for(i=0;i<MAXROOMS;++i)
    nodes[i]=NULL;

  makezonelist(zone);

  i=0;
  head=read_room(tiny);
  nodes[head->num]=head;
  while(!feof(tiny)){
    if(!(t=read_room(tiny)))
      break;

    printf("\rReading room %i (%i)", ++i, t->num);
    t->next=head;
    head=t;
    nodes[t->num]=t;
  }
  t=head;
  head=head->next;
  nodes[t->num]=NULL;
  delete t;

  // now we fill in the array of exits in each node to point to the nodes
  // that they lead to
  printf("\nBeginning consolidation\n");
  consolidate_nodes();
  printf("\nFinished consolidation\n");

  // find room 100 and make it the head
  for(t=head;t->num!=100;t=t->next)
    last=t;

  last->next=t->next;
  t->next=head;
  head=t;


  printf("Calculating coordinates\n");
  while(coordinates());
  printf("Finished coordinates\n");
  //  check_rooms();

  createmap(-10,20);

  t=head;
  while(t){
    last=t;
    t=t->next;
    free(last);
  }
}


