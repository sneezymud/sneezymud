#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<map>
#include<vector>
#include<ctype.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<dirent.h>
#include"stdsneezy.h"
#include"lowtools.h"
#include"parse.h"
#include"database.h"

// this code is totally janky, but it gets the job done - peel

#define MAXCOORDITER 1000

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

} *head;

map <int, class NODE *> nodes;


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
  if(num<0) 
    return NULL;

  return(nodes[num]);
}


bool isEnabled(int num)
{
  bool enabled=false;
  map<int,bool>::iterator iter;

  if(zone_enabled.size()==0)
    return true;
  
  for(iter=zone_enabled.begin();iter!=zone_enabled.end();++iter){
    if((*iter).first>=num){
      enabled=(*iter).second;
      break;
    }
  }

  return enabled;
}

NODE *read_room(TDatabase *db, TDatabase *dbexits)
{
  NODE *tmp=new NODE;
  tmp->next=NULL;

  if(!tmp) {
    fprintf(stderr, "read_room(): unable to allocate new NODE (tmp==NULL)\n");
    return NULL;
  }

  do {
    if(!db->fetchRow()){
      //	fprintf(stderr, "read_room(): couldn't find a room\n");
	return NULL;      
    }

    tmp->num=convertTo<int>((*db)["vnum"]);

  } while(!isEnabled(tmp->num));

  
  strcpy(tmp->name, (*db)["name"].c_str());

  // parse sector type
  tmp->sector=convertTo<int>((*db)["sector"]);

  do{
    if(convertTo<int>((*dbexits)["vnum"]) < tmp->num){
      continue;
    } else if(convertTo<int>((*dbexits)["vnum"]) > tmp->num){
      break;
    }

    tmp->idirs[convertTo<int>((*dbexits)["direction"])]=convertTo<int>((*dbexits)["destination"]);
  } while((*dbexits).fetchRow());


  return tmp; 
}


NODE *read_room(FILE *tiny){
  NODE *tmp=new NODE;
  int dir, room, c, end=0, t;
  char tch;

  if(!tiny){
    fprintf(stderr, "read_room() called with null file handle\n");
    return NULL;
  }

  if(!tmp) {
    fprintf(stderr, "read_room(): unable to allocate new NODE (tmp==NULL)\n");
    return NULL;
  }

  do {
    // read until we get to a new room ('#')
    while((tch=fgetc(tiny))){
      if(tch=='#')
	break;
      if(tch==EOF){
	fprintf(stderr, "read_room(): couldn't find a room\n");
	return NULL;
      }
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

void consolidate_nodes(bool quiet=false){
  NODE *t;
  int i, j=0;
 
  for(t=head;t;t=t->next){
    ++j;
    if(!quiet && !(j % 10))
      printf("\rConsolidating %i", j);
    for(i=0;i<10;++i){
      t->pdirs[i]=find_node(t->idirs[i]);
    }
  }
}


int coordinates(bool quiet=false)
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
  
  if(!quiet){
    printf("Coordinate iteration %4i (%i rooms left)\r",
	   ++count, roomsleft);
    fflush(stdout);
  }

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

void check_rooms(int MAXROOMS){
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
	  if((!nodes[i]->pdirs[j]->pdirs[rev_dir[j]] ||
	     (nodes[i]->pdirs[j]->pdirs[rev_dir[j]]->num !=
	     nodes[i]->num)) && !done[i]){
	    
	    printf("Room %i has a one way exit %i (%i) to room %i\n",
		   nodes[i]->num, j, rev_dir[j], nodes[i]->pdirs[j]->num);

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

map <int,int> makeroomcount(FILE *log, int &max){
  sstring buf;
  unsigned int s, e;
  map <int,int> roomcount;
  char sbuf[1024];

  while(!feof(log)){
    fscanf(log, "%[^\n]", sbuf);
    fscanf(log, "\n");
    buf=sbuf;

    if((s=buf.find_first_of("::", 0))==sstring::npos)
      continue;

    if((s=buf.find_first_of("(", s))==sstring::npos)
      continue;

    if((e=buf.find_first_of("):", s))==sstring::npos)
      continue;

    if(convertTo<int>(buf.substr(s+1,e-s+1))==0)
      continue;

    roomcount[convertTo<int>(buf.substr(s+1,e-s+1))]++;

    if(roomcount[convertTo<int>(buf.substr(s+1,e-s+1))] > max)
      max=roomcount[convertTo<int>(buf.substr(s+1,e-s+1))];

  }

  return roomcount;
}


void createmap(int MINLEVEL, int MAXLEVEL, int SCALEBY, sstring outputfile, bool sideways, FILE *logf)
{
  NODE *t;
  int minx=0, maxx=0, miny=0, maxy=0, mapsize=0, mapwidth, mapheight, loc;
  int newloc, cellx, celly;
  char *mapdata, buff[256], *newmap;
  int i=0, j, k, l, tmp;
  FILE *out=fopen("imageout.raw", "wb");
  const int CELLSIZE=3;
  map <int,int> roomcount;
  int max=0;

  if(logf)
    roomcount=makeroomcount(logf, max);

  if(sideways){
    for(t=head;t;t=t->next){
      tmp=t->y;
      t->y=t->z;
      t->z=tmp;
    }
  }

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
    if(t->z<MINLEVEL || t->z>MAXLEVEL)
      continue;

    // shift the x,y values so that we always have positive coords
    t->x+=abs(minx);
    t->y+=abs(miny);

    cellx=t->x*CELLSIZE;
    celly=t->y*CELLSIZE;
    loc=(mapwidth*celly)+cellx;

    // color the room
    mapdata[loc*3]=sector_colors[t->sector][0];
    mapdata[(loc*3)+1]=sector_colors[t->sector][1];
    mapdata[(loc*3)+2]=sector_colors[t->sector][2];
    
    // color by popularity
    if(logf){
      double perc =  log(double(roomcount[t->num])) / log(double(max));
      perc=1.0-perc;
      double red, green, blue;
      
      if (perc < 0.5 && perc >= 0.25)
	red = min(255.0, 255  * ((0.25 - (perc - 0.25)) / 0.25));
      else if (perc < 0.25)
	red = 255 ;
      else
	red = 0;
      
      if (perc > 0.5 && perc <= 0.75)
	blue = min(255.0, 255 * ((perc - 0.50) / 0.25));
      else if (perc > 0.75)
	blue = 255;
      else
	blue = 0;
      
      if (perc < 0.25)
	green = min(255.0, 255 * (1 - ((0.25 - perc) / 0.25)));
      else if (perc > 0.75)
	green = min(255.0, 255 * (1 - ((perc - 0.75) / 0.25)));
      else
	green = 255;
      
      
      mapdata[loc*3]=(int)red;
      mapdata[(loc*3)+1]=(int)green;
      mapdata[(loc*3)+2]=(int)blue;
    }


    // color the exits
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

  // scale it larger
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
  sprintf(buff, "/usr/local/bin/rawtoppm -rgb %i %i imageout.raw | /usr/local/bin/pnmflip -tb | /usr/local/bin/ppmtojpeg > %s", mapwidth, mapheight, outputfile.c_str());
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

  // read until we get to a new zone ('#')
  while((tch=fgetc(zone))){
    if(last=='\n' || tch=='#' || tch==EOF){
      if(tch=='#'){
	break;
      }
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



void usage(){
  printf("Syntax: amap <options>\n");
  printf("  -f <tinyfile>   - any room file will work.  the default is to\n");
  printf("                    use /mud/code/lib/tinyworld.wld.\n");
  printf("  -s <scale by>   - scaling factor for the map, default is 2\n");
  printf("                    factor of 1 will give you a more compact view\n");
  printf("  -c              - print consistency errors.  this will create\n");
  printf("                    tons of spam, so redirect to a file.  suggest\n");
  printf("                    using the q option with this as well\n");
  printf("  -q              - quiet mode, useful for slow connections\n");
  printf("  -h <head room>  - specify the 0,0,0 coordinate room\n");
  printf("  -x              - make a sideways map with y and z swapped\n");
  printf("  -z <z range>    - specify the z range to map, default -10,20\n");
  printf("  -o <file>       - specify the image output file default is\n");
  printf("                    imageout.gif in the current directory\n");
  printf("  -p              - color code rooms by popularity instead of sector\n");
  printf("  -l <file>       - specify a log file to use for popularity parsing\n");
  printf("                    defaults to /mud/prod/lib/logs/logcurrent\n");
  printf("  -r <room range> - a list of room numbers to map, in the same\n");
  printf("                    format as the other tools.\n");
  printf("                    MUST BE THE LAST ARGUMENT\n");
  printf("\n");
  printf("You may attempt to map multiple zones if they are connected, by\n");
  printf("specifying both of their room ranges, ex: 2350-2374 600-649\n");
  printf("will attempt to map both the casino and twilight square.\n");
  printf("The mapping program needs to have a base room to use as the\n");
  printf("0,0,0 coordinate.  With a full world map, this room is center\n");
  printf("square, or you may specify a room to use with the -h option or\n");
  printf("the first room listed in the room range will be used.\n");
}

int main(int argc, char **argv)
{
  FILE *tiny=NULL, *logf=NULL;
  FILE *zone;
  DIR *dfd=opendir("/mud/code/lib/zonefiles");
  struct dirent *dp;
  NODE *last=NULL, *t;
  int SCALEBY=2, rcount, ch, zmax=20, zmin=-10, tmp;
  vector <int> roomrange_t;
  map <int, bool> roomrange;
  bool use_range=false, checkrooms_p=false, quiet=false, sideways=false;
  bool popularity=false;
  int headroom=100;
  sstring infile, buf, outputfile="imageout.jpg";
  sstring logfile="/mud/prod/lib/logs/logcurrent";
  TDatabase db(DB_SNEEZYBETA), dbexits(DB_SNEEZYBETA);

  while ((ch = getopt(argc, argv, "r:f:s:ch:qz:o:xl:p")) != -1){
    switch (ch) {
      case 'r':
	parse_num_args(argc-optind+1, argv+optind-1, roomrange_t);
	for(unsigned int i=0;i<roomrange_t.size();++i)
	  roomrange[roomrange_t[i]]=true;

	use_range=true;
	printf("%i rooms in custom range\n", roomrange_t.size());
	break;
      case 'f':
	infile=optarg;
	break;
      case 's':
	SCALEBY=convertTo<int>(optarg);
	printf("Scaling by %i\n", SCALEBY);
	break;
      case 'c':
        checkrooms_p=true;
	break;
      case 'h':
	headroom=convertTo<int>(optarg);
	printf("Using %i as head room (0,0,0)\n", headroom);
	break;
      case 'q':
	quiet=true;
	break;
      case 'z':
	buf=optarg;
	tmp=buf.find_first_of(',', 0);
	zmin=convertTo<int>(buf.substr(0, tmp));
	zmax=convertTo<int>(buf.substr(tmp+1, buf.size()-tmp));
	printf("Using z range of %i to %i\n", zmin, zmax);
	break;
      case 'o':
	outputfile=optarg;
	printf("Outputting image file to %s\n", outputfile.c_str());
	break;
      case 'x':
	sideways=true;
	break;
      case 'p':
	popularity=true;
	break;
      case 'l':
	logfile=optarg;
	break;
      case '?':
      default:
	usage();
	exit(0);
    }
  }

  if(popularity){
    logf=fopen(logfile.c_str(), "rt");
    printf("Coloring by popularity using %s\n", logfile.c_str());
  }


  if(!infile.empty()){
    printf("Using '%s' as room file.\n", infile.c_str());
    tiny=fopen(infile.c_str(), "rt");
    
    if(!tiny){
      printf("Unable to open tiny file.\n");
      usage();
      exit(0);
    }
  } else {
    printf("Using sneezybeta database for room data.\n");
    db.query("select * from room order by vnum");
    dbexits.query("select * from roomexit order by vnum");
    dbexits.fetchRow();
  }

  printf("Making zone list.\n");
  while ((dp = readdir(dfd))) {
    if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..") &&
	strcmp(dp->d_name, "CVS")){
      buf=fmt("/mud/code/lib/zonefiles/%s") % dp->d_name;

      if((zone=fopen(buf.c_str(), "rt"))){
	makezonelist(zone);
	fclose(zone);
      }
    }
  }

  if(tiny){
    head=read_room(tiny);
  } else {
    head=read_room(&db, &dbexits);
  }

  nodes[head->num]=head;
  head->next=NULL;

  for(rcount=1; (!tiny || !feof(tiny)); ++rcount){
    if(tiny){
      if(!(t=read_room(tiny)))
	break;
    } else {
      if(!(t=read_room(&db, &dbexits)))
	break;
    }      


    if(use_range && roomrange.count(t->num)==0)
      continue;

    if(!quiet && !(rcount%10))
      printf("\rReading room %i (%i)", rcount, t->num);
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
  consolidate_nodes(quiet);
  printf("\nFinished consolidation\n");
  
  // this lets us specify a better 0,0,0 point for custom ranges
  // the first room passed in the argument is used as the 0,0,0 point
  if(use_range && headroom==100)
    headroom=roomrange_t[0];

  // find desired room and make it the head
  for(t=head;t && t->num!=headroom;t=t->next)
    last=t;

  if(t && last){
    last->next=t->next;
    t->next=head;
    head=t;
  }

  printf("Using %i as the base room (0,0,0)\n", head->num);

  printf("Calculating coordinates\n");
  while(coordinates(quiet));
  printf("Finished coordinates\n");

  if(checkrooms_p)
    check_rooms(rcount);
    
  createmap(zmin, zmax, SCALEBY, outputfile, sideways, logf);

  t=head;
  while(t){
    last=t;
    t=t->next;
    free(last);
  }
}


