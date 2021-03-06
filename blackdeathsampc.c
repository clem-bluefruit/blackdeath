#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

unsigned char dtae;

#define BET(A, B, C)  (((A>=B)&&(A<=C))?1:0)    /* a between [b,c] */
#define floor(x) ((int)(x))
#define CELLLEN 32
#define starcount 4
#define NSTEPS  10000
#define dead 255                                                                   
#define MAX_SAM 512

unsigned char *cells, *newcells;
int spointer, mpointer, prog;
int tick, tween, place, readhead, writehead, cellhead;
int maxsamp = MAX_SAM;
signed char insdir,dir,dirr,insdirr; 
int *swap;
int swapping;
int kwhich,dist, oldknob ,oldkn,oldknn,kn,knn;
int susceptible = 0;                                                                   
int recovered = 255;                                                                   
int grainsize=10, rgrainsize=10,chunk=100;
int tau = 2;  
unsigned int modrrr,modrr;
unsigned char filterk, cpu, plague, hardk, fhk, instruction, IP, controls, hardware, samp, count,qqq;
unsigned int instructionp, instructionpr;

unsigned char sinewave[] = //256 values
{
0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
0xb0,0xb3,0xb6,0xb9,0xbc,0xbf,0xc1,0xc4,0xc7,0xc9,0xcc,0xce,0xd1,0xd3,0xd5,0xd8,
0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xed,0xef,0xf0,0xf2,0xf3,0xf5,
0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfd,0xfc,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,
0xf6,0xf5,0xf3,0xf2,0xf0,0xef,0xed,0xec,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
0xda,0xd8,0xd5,0xd3,0xd1,0xce,0xcc,0xc9,0xc7,0xc4,0xc1,0xbf,0xbc,0xb9,0xb6,0xb3,
0xb0,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x63,0x60,0x5d,0x5a,0x57,0x54,0x51,
0x4f,0x4c,0x49,0x46,0x43,0x40,0x3e,0x3b,0x38,0x36,0x33,0x31,0x2e,0x2c,0x2a,0x27,
0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x12,0x10,0x0f,0x0d,0x0c,0x0a,
0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x0a,0x0c,0x0d,0x0f,0x10,0x12,0x13,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
0x25,0x27,0x2a,0x2c,0x2e,0x31,0x33,0x36,0x38,0x3b,0x3e,0x40,0x43,0x46,0x49,0x4c,
0x4f,0x51,0x54,0x57,0x5a,0x5d,0x60,0x63,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
};

typedef struct {
  int ch;int chco;int step;
  double h,a,b,c,lx0,ly0,lz0;
  int inty;
  int intx;
  int intz;
}rosstype;
	
typedef struct{ double x, y; } Point;

typedef struct {
	double prob[5];
	double coeff[4][6];
	Point p1,p2;
	unsigned char returnvalx;
	unsigned char returnvaly;
	unsigned int ifscount;
	unsigned char step;
	} ifss;

ifss ifs;
rosstype ross;
unsigned char xxx[512];
unsigned char *lsamp;
#define MAX_SAM 512

unsigned char ostack[20], stack[20], omem;

/* BIOTA: two dimensional memory map */

unsigned char btdir,dcdir,btdirr,dcdirr;

void initross(rosstype* ross){
  ross->h = 0.01;
  ross->a = 0.2;
  ross->b = 0.2;
  ross->c = 5.7;
  ross->lx0 = 0.1;
  ross->ly0 = 0;
  ross->lz0 = 0;
  ross->step=1;
}

void runross(rosstype* ross){
  double lx0,ly0,lz0,lx1,ly1,lz1;
  double h,a,b,c;

  h = ross->h;
  a = ross->a;
  b = ross->b;
  c = ross->c;
  lx0 = ross->lx0;
  ly0 = ross->ly0;
  lz0 = ross->lz0;
  lx1 = lx0 + h * (-ly0 - lz0);
  ly1 = ly0 + h * (lx0 + (a * ly0));
  lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
  ross->lx0 = lx1;
  ross->ly0 = ly1;
  ross->lz0 = lz1;
  //  ross->intx=(signed int)((float)lx1*1024)%1024;
  //  ross->inty=(signed int)((float)ly1*1024)%1024;
  ross->intz=(signed int)((float)lz1*1024)%1024;
}

void initifs(ifss* ifs){
  int i,iter;
  int column = 6, row = 4;
  ifs->step=1;         
  ifs->ifscount=0;
  ifs->prob[0]=0.0;
  ifs->prob[1]=0.85; 
  ifs->prob[2]=0.92; 
  ifs->prob[3]=0.99; 
  ifs->prob[4]=1.0; 
  ifs->p1.x=0.1;
  ifs->p1.y=0.1;         
                         
  for (iter=0;iter<row;iter++){
    for (i=0;i<column;i++){
      iter=rand()%row;
      i=rand()%column;
      ifs->coeff[iter][i]=((double)rand()/(double)(RAND_MAX));
      if (((double)rand()/(double)(RAND_MAX))>0.5) ifs->coeff[iter][i]= ifs->coeff[iter][i]-1;
      ifs->prob[iter]=((double)rand()/(double)(RAND_MAX));
    }
  }
}

void runifs(ifss* ifs){
  double random_num;
  int iter,i;
  int column = 6, row = 4;

  ifs->ifscount++;
  random_num = (double)rand()/(double)(RAND_MAX);
  for(i = 0; i < row; i++){
    if ( BET(random_num,ifs->prob[i],ifs->prob[i+1]) ){
      ifs->p2.x = ifs->coeff[i][0]*ifs->p1.x + ifs->coeff[i][1]*ifs->p1.y + ifs->coeff[i][4];
      ifs->p2.y = ifs->coeff[i][2]*ifs->p1.x + ifs->coeff[i][3]*ifs->p1.y + ifs->coeff[i][5];
      break;
    }
  }
  ifs->p1=ifs->p2;  
					
  if (ifs->p2.x>0.0)
    ifs->returnvalx=(int)((ifs->p2.x)*1024);
  if (ifs->p2.y>0.0)
    ifs->returnvaly=(int)((ifs->p2.y)*1024);

  if (ifs->ifscount>NSTEPS) {
    iter=rand()%row;
    i=rand()%column;
    ifs->coeff[iter][i]=((double)rand()/(double)(RAND_MAX));
    if (((double)rand()/(double)(RAND_MAX))>0.5) ifs->coeff[iter][i]= ifs->coeff[iter][i]-1;
    ifs->prob[iter]=((double)rand()/(double)(RAND_MAX));
    ifs->p1.x=0.5;
    ifs->p1.y=0.5;
  }
}

void runbrainw(unsigned char *cells){
  unsigned char *xxx; int kl=0,ss;

  // should also be able to move these around

  spointer++; 
  if (spointer==0) spointer=MAX_SAM;
  if (mpointer==0) mpointer=MAX_SAM;
  if (spointer>MAX_SAM) spointer=0;
  if (mpointer>MAX_SAM) mpointer=0;

  xxx=((unsigned char *)(cells+spointer));
  prog=*xxx%8;
  switch(prog) {
    /* Increment pointer value */
  case 0:
    xxx=((unsigned char *)(cells+mpointer));
    *xxx=*xxx+1;
    break;
    /* Decrement pointer value */
  case 1:
    xxx=((unsigned char *)(cells+mpointer));
    *xxx=*xxx-1;
    break;
    /* Increment pointer */
  case 2:
    mpointer++;
    break;
    /* Decrement pointer */
  case 3:
    mpointer--;
    break;
    /* Print current pointer value */
  case 4:
    xxx=((unsigned char *)(cells+mpointer));
    dtae=*xxx;
    break;
    /* Read value and store in current pointer */
  case 5:
    xxx=((unsigned char *)(cells+mpointer));
    *xxx=dtae;
    break;
    /* Start loop */
  case 6:
    xxx=((unsigned char *)(cells+mpointer));
    if (*xxx == 0) {
      /* Find matching ] */
      spointer++;
      if (spointer>MAX_SAM) spointer=0;

      /* If kl == 0 and space[pointer] == ']' we found
       * the matching brace */
      ss=0;
      xxx=((unsigned char *)(cells+spointer));
      while ((kl > 0 || (*xxx)%8 != 7) &&  ss<254)
	{
	  //	  printf("%d ",ss);
	  ss++;
	  if ((*xxx)%8 == 6) kl++;
	  if ((*xxx)%8 == 7) kl--;
	  /* Go in right direction */
	  spointer++;
	  if (spointer>MAX_SAM) spointer=0;
	  xxx=((unsigned char *)(cells+spointer));
	}
    }
    break;
    /* End loop */
  case 7:
    xxx=((unsigned char *)(cells+mpointer));
    if (*xxx != 0) { // a fix
      /* Find matching [ */
      spointer--;
      if (spointer==0) spointer=MAX_SAM;
      xxx=((unsigned char *)(cells+spointer));
      ss=0;
      while ((kl > 0 || (*xxx)%8 != 6) && ss<254) {
	ss++;
	if ((*xxx)%8 == 6) kl--;
	if ((*xxx)%8 == 7) kl++;
	/* Go left */
	spointer--;
	if (spointer==0) spointer=MAX_SAM;
	xxx=((unsigned char *)(cells+spointer));
      }
      spointer--;
      if (spointer==0) spointer=MAX_SAM;
    }
    break;
  }
}


unsigned char btempty(unsigned char* cells, unsigned char IP){
  // turn around
  if (btdir==0) btdir=1;
  else if (btdir==1) btdir=0;
  else if (btdir==2) btdir=3;
  else if (btdir==3) btdir=2;
  return IP;
}

unsigned char btoutpw(unsigned char* cells, unsigned char IP){
  dtae=cells[omem];
  return IP;
}

unsigned char btoutpr(unsigned char* cells, unsigned char IP){
  dtae=cells[omem];
  return IP;
}

unsigned char btstraight(unsigned char* cells, unsigned char IP){
  if (dcdir==0) omem+=1;
  else if (dcdir==1) omem-=1;
  else if (dcdir==2) omem+=16;
  else if (dcdir==3) omem-=16;

  if (cells[omem]==0) 
    { // change dir
  if (btdir==0) btdir=1;
  else if (btdir==1) btdir=0;
  else if (btdir==2) btdir=3;
  else if (btdir==3) btdir=2;
    }
  return IP;
}

unsigned char btbackup(unsigned char* cells, unsigned char IP){
  if (dcdir==0) omem-=1;
  else if (dcdir==1) omem+=1;
  else if (dcdir==2) omem-=16;
  else if (dcdir==3) omem+=16;
  if (cells[omem]==0) 
    {
  if (btdir==0) btdir=1;
  else if (btdir==1) btdir=0;
  else if (btdir==2) btdir=3;
  else if (btdir==3) btdir=2;
    }
  return IP;
}

unsigned char btturn(unsigned char* cells, unsigned char IP){
  if (dcdir==0) omem+=16;
  else if (dcdir==1) omem-=16;
  else if (dcdir==2) omem+=1;
  else if (dcdir==3) omem-=1;
  return IP;
}

unsigned char btunturn(unsigned char* cells, unsigned char IP){
  if (dcdir==0) omem-=16;
  else if (dcdir==1) omem+=16;
  else if (dcdir==2) omem-=1;
  else if (dcdir==3) omem+=1;
  return IP;
}

unsigned char btg(unsigned char* cells, unsigned char IP){
  unsigned char x;
  while (x<20 && cells[omem]!=0){
    if (dcdir==0) omem+=1;
    else if (dcdir==1) omem-=1;
    else if (dcdir==2) omem+=16;
    else if (dcdir==3) omem-=16;
    x++;
  }
  return IP;
}

unsigned char btclear(unsigned char* cells, unsigned char IP){
  if (cells[omem]==0){
  if (btdir==0) btdir=1;
  else if (btdir==1) btdir=0;
  else if (btdir==2) btdir=3;
  else if (btdir==3) btdir=2;
  }
  else cells[omem]=0;
  return IP;
}

unsigned char btdup(unsigned char* cells, unsigned char IP){
  if (cells[omem]==0 || cells[omem-1]!=0){
  if (btdir==0) btdir=1;
  else if (btdir==1) btdir=0;
  else if (btdir==2) btdir=3;
  else if (btdir==3) btdir=2;
  }
  else cells[omem-1]=cells[omem];
  return IP;
}

unsigned char clock, count;

// reddeath

//1- the plague within (12 midnight) - all the cells infect

unsigned char redplague(unsigned char* cells, unsigned char IP){
  if (clock==12){
    clock=12;
    cells[IP+1]=cells[IP];
    if (IP==255) clock=13;
    return IP+1;
  }
  else return IP+insdir;
}

//2- death - one by one fall dead
unsigned char reddeath(unsigned char* cells, unsigned char IP){
  if (clock==13){
    clock=13;
    cells[IP+count]=*((unsigned char *)(cells+(dtae%MAX_SAM))); //????
    count++;
    return IP; // just keeps on going
  }
  else return IP+insdir;
}

//3- clock every hour - instruction counter or IP -some kind of TICK
unsigned char redclockw(unsigned char* cells, unsigned char IP){
  clock++;
  if (clock%60==0) {
    dtae^=255;
    return IP; // everyone stops
  }
  else return IP+insdir;
}

unsigned char redclockr(unsigned char* cells, unsigned char IP){
  clock++;
  if (clock%60==0) {
    dtae^=255;
    return IP; // everyone stops
  }
  else return IP+insdir;
}


//4- seven rooms: divide cellspace into 7 - 7 layers with filter each
unsigned char redrooms(unsigned char* cells, unsigned char IP){
  // need to change for blackdeath

  switch(IP%7){
  case 0:
    //blue
    modrrr=0;
     break;
  case 1:
    //purple
    modrrr=1;
     break;
  case 2:
    //green
    modrrr=2;
     break;
  case 3:
    //orange
    modrrr=3;
     break;
  case 4:
    //white
    modrrr=4;
     break;
  case 5:
    //violet
    modrrr=5;
    break;
  case 6:
    // black
    modrrr=6;
  }
  return IP+insdir;
}

  //5- unmasking (change neighbouring cells)

unsigned char redunmask(unsigned char* cells, unsigned char IP){
  cells[IP-1]^=255;
  cells[IP+1]^=255;
return IP+insdir;
}
  //6- the prince (omem) - the output! walking through 7 rooms 

unsigned char redprosperow(unsigned char* cells, unsigned char IP){

  unsigned char dirrr;
  // prince/omem moves at random through rooms
  dirrr=*((unsigned char *)(cells+(dtae%MAX_SAM)))%4;
  if (dirrr==0) omem=omem+1;
  else if (dirrr==1) omem=omem-1;
  else if (dirrr==2) omem=omem+16;
  else if (dirrr==3) omem=omem-16;

  // output
  dtae=cells[omem]; 
  return IP+insdir;
}

unsigned char redprosperor(unsigned char* cells, unsigned char IP){

  unsigned char dirrr;
  // prince/omem moves at random through rooms
  dirrr=*((unsigned char *)(cells+(dtae%MAX_SAM)))%4;
  if (dirrr==0) omem=omem+1;
  else if (dirrr==1) omem=omem-1;
  else if (dirrr==2) omem=omem+16;
  else if (dirrr==3) omem=omem-16;

  // output
  dtae=cells[omem]; 
  return IP+insdir;
}


  //7- the outside - the input!
unsigned char redoutside(unsigned char* cells, unsigned char IP){

  // input sample to cell (which one neighbour to omem)
  cells[omem+1]=*((unsigned char *)(cells+(dtae%MAX_SAM)));

  // output to filter 
  //  (*filtermod[qqq]) ((int)cells[omem]);
  return IP+insdir;
}

// plague

unsigned char ploutpw(unsigned char* cells, unsigned char IP){
  dtae=cells[IP+1]+cells[IP-1];
  return IP+insdir;
}

unsigned char ploutpr(unsigned char* cells, unsigned char IP){
  dtae=cells[IP+1]+cells[IP-1];
  return IP+insdir;
}


unsigned char plenclose(unsigned char* cells, unsigned char IP){
  cells[IP]=255; cells[IP+1]=255;
  return IP+2;
}

unsigned char plinfect(unsigned char* cells, unsigned char IP){

  if (cells[IP]<128) {
    cells[IP+1]= cells[IP];
    cells[IP-1]= cells[IP];
  }
  return IP+insdir;
}

unsigned char pldie(unsigned char* cells, unsigned char IP){
  cells[IP-1]=0; cells[IP+1]=0;
  return IP+insdir;
}

unsigned char plwalk(unsigned char* cells, unsigned char IP){
  // changing direction
  if (dir<0 && (cells[IP]%0x03)==1) dir=1;
  else if (dir>1 && (cells[IP]%0x03)==0) dir=-1;
  else
    // changing pace
    insdir=(int)dir*cells[IP]>>4;
  
  return IP+insdir;
}

// redcode

unsigned char rdmov(unsigned char* cells, unsigned char IP){
  cells[(IP+cells[IP+2])]=cells[(IP+cells[IP+1])];
  return IP+=3;
}

unsigned char rdadd(unsigned char* cells, unsigned char IP){
  cells[(IP+cells[IP+2])]=cells[(IP+cells[IP+2])]+cells[(IP+cells[IP+1])];
  return IP+=3;
}

unsigned char rdsub(unsigned char* cells, unsigned char IP){
  cells[(IP+cells[IP+2])]=cells[(IP+cells[IP+2])]-cells[(IP+cells[IP+1])];
  return IP+=3;
}

unsigned char rdjmp(unsigned char* cells, unsigned char IP){
  IP=IP+cells[IP+1];
  return IP;
}

unsigned char rdjmz(unsigned char* cells, unsigned char IP){
  if (cells[(IP+cells[IP+2])]==0) IP=cells[IP+1];
  else IP+=3;
  return IP;
}

unsigned char rdjmg(unsigned char* cells, unsigned char IP){
  if (cells[(IP+cells[IP+2])]>=0) IP=cells[IP+1];
  else IP+=3;
  return IP;
}

unsigned char rddjz(unsigned char* cells, unsigned char IP){
  unsigned char x;
  x=(IP+cells[IP+2]);
  cells[x]=cells[x]-1;
  if (cells[x]==0) IP=cells[IP+1];
  else IP+=3;
  return IP;
}

unsigned char rddat(unsigned char* cells, unsigned char IP){
  IP+=3;
  return IP;
}

unsigned char rdcmp(unsigned char* cells, unsigned char IP){
  if (cells[(IP+cells[IP+2])]!=cells[(IP+cells[IP+1])]) IP+=6;
  else IP+=3;
  return IP;
}

unsigned char rdoutpw(unsigned char* cells, unsigned char IP){
  dtae=cells[(IP+2)]; 
  IP+=3;
  return IP;
}

unsigned char rdoutpr(unsigned char* cells, unsigned char IP){
  dtae=cells[(IP+2)]; 
  IP+=3;
  return IP;
}


// SIR: inc if , die if, recover if, getinfected if 

unsigned char SIRoutpw(unsigned char* cells, unsigned char IP){
  dtae=cells[(IP+1)]+cells[IP-1];
  return IP+insdir;
}

unsigned char SIRoutpr(unsigned char* cells, unsigned char IP){
  dtae=cells[(IP+1)]+cells[IP-1];
  return IP+insdir;
}


unsigned char SIRincif(unsigned char* cells, unsigned char IP){
  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) cells[IP]++;
  return IP+insdir;
}

unsigned char SIRdieif(unsigned char* cells, unsigned char IP){
  
  if ((cells[(IP+1)]>0 && cells[(IP+1)]<128)) {
    if (rand()%10 < 4) cells[IP] = dead;       
  }
  return IP+insdir;
}

unsigned char SIRrecif(unsigned char* cells, unsigned char IP){
  if (cells[(IP+1)] >= 128) cells[IP] = recovered;                                             
  return IP+insdir;
}

unsigned char SIRinfif(unsigned char* cells, unsigned char IP){

  if (cells[(IP+1)] == 0) {   
                                                 
    if ((cells[IP-1]>0 && cells[IP-1]<128) ||
	(cells[(IP+1)]>0 && cells[(IP+1)]<128))
	{
	if (rand()%10 < 4) cells[IP] = 1;       
	}
}
  return IP+insdir;
}

// brainfuck

unsigned char cycle;

unsigned char bfinc(unsigned char* cells, unsigned char IP){
  omem++; 
  return IP++;
}

unsigned char bfdec(unsigned char* cells, unsigned char IP){
  omem--; 
  return IP++;
}

unsigned char bfincm(unsigned char* cells, unsigned char IP){
  cells[omem]++; 
  return IP++;
}

unsigned char bfdecm(unsigned char* cells, unsigned char IP){
  cells[omem]--; 
  return IP++;
}

unsigned char bfoutpw(unsigned char* cells, unsigned char IP){
  dtae=cells[omem]; 
  return IP++;
}

unsigned char bfoutpr(unsigned char* cells, unsigned char IP){
  dtae=cells[omem]; 
  return IP++;
}


unsigned char bfin(unsigned char* cells, unsigned char IP){
  cells[omem] = *((unsigned char *)(cells+(dtae%MAX_SAM)));
  return IP++;
}

unsigned char bfbrac1(unsigned char* cells, unsigned char IP){
  cycle++; 
  if(cycle>=20) cycle=0; 
  ostack[cycle] = IP; 
  return IP++;
}

unsigned char bfbrac2(unsigned char* cells, unsigned char IP){
  int i;
  if(cells[omem] != 0) i = ostack[cycle]-1; 
  cycle--; 
  if(cycle<-1) cycle=20;
  return i;
}

// first attempt

unsigned char finc(unsigned char* cells, unsigned char IP){
  omem++; 
  return IP+insdir;
}

unsigned char fin1(unsigned char* cells, unsigned char IP){
  omem=*((unsigned char *)(cells+(dtae%MAX_SAM)));
  return IP+insdir;
}

unsigned char fdec(unsigned char* cells, unsigned char IP){
  omem--; 
  return IP+insdir;
}

unsigned char fincm(unsigned char* cells, unsigned char IP){
  cells[omem]++; 
  return IP+insdir;
}

unsigned char fdecm(unsigned char* cells, unsigned char IP){
  cells[omem]--; 
  return IP+insdir;
}


unsigned char outpw(unsigned char* cells, unsigned char IP){
  dtae=cells[omem];
  return IP+insdir;
}

unsigned char outppw(unsigned char* cells, unsigned char IP){
  dtae=omem;
  return IP+insdir;
}

unsigned char outpr(unsigned char* cells, unsigned char IP){
  dtae=cells[omem];
  return IP+insdir;
}

unsigned char outppr(unsigned char* cells, unsigned char IP){
  dtae=omem;
  return IP+insdir;
}


unsigned char plus(unsigned char* cells, unsigned char IP){
  cells[IP]+=1;
  return IP+insdir;
}

unsigned char minus(unsigned char* cells, unsigned char IP){
  cells[IP]-=1;
  return IP+insdir;
}

unsigned char bitshift1(unsigned char* cells, unsigned char IP){
  cells[IP]=cells[IP]<<1;
  return IP+insdir;
}

unsigned char bitshift2(unsigned char* cells, unsigned char IP){
  cells[IP]=cells[IP]<<2;
  return IP+insdir;
}

unsigned char bitshift3(unsigned char* cells, unsigned char IP){
  cells[IP]=cells[IP]<<3;
  return IP+insdir;
}

unsigned char branch(unsigned char* cells, unsigned char IP){
  if (cells[IP+1]==0) IP=cells[omem];
  return IP+insdir;
}

unsigned char jump(unsigned char* cells, unsigned char IP){
  if (cells[(IP+1)]<128) return IP+cells[(IP+1)];
  else return IP+insdir;
}

unsigned char infect(unsigned char* cells, unsigned char IP){
  int x=IP-1;
  if (x<0) x=MAX_SAM;
  if (cells[x]<128) cells[(IP+1)]= cells[IP];
  return IP+insdir;
}

unsigned char store(unsigned char* cells, unsigned char IP){
  cells[IP]=cells[cells[IP+1]];
  return IP+insdir;
}

unsigned char skip(unsigned char* cells, unsigned char IP){
  return IP+insdir;
}

unsigned char direction(unsigned char* cells, unsigned char IP){
  if (dir<0) dir=1;
  else dir=-1;
  return IP+insdir;
}

unsigned char die(unsigned char* cells, unsigned char IP){
  return IP+insdir;
}

unsigned char writesamp(unsigned char* cells, unsigned char IP){
  cells[IP]=*((unsigned char *)(cells+(dtae%MAX_SAM)));
  return IP+insdir;
}

  unsigned char (*instructionsetfirstw[])(unsigned char* cells, unsigned char IP) = {outppw,finc,fdec,fincm,fdecm,fin1,outpw,plus,minus,bitshift1,bitshift2,bitshift3,branch,jump,infect,store,writesamp,skip,direction,die}; // 20 instructions

  unsigned char (*instructionsetplaguew[])(unsigned char* cells, unsigned char IP) = {writesamp, ploutpw, plenclose, plinfect, pldie, plwalk}; // 6

  unsigned char (*instructionsetbfw[])(unsigned char* cells, unsigned char IP) = {bfinc,bfdec,bfincm,bfdecm,bfoutpw,bfin,bfbrac1,bfbrac2}; // 8

  unsigned char (*instructionsetSIRw[])(unsigned char* cells, unsigned char IP) = {SIRoutpw,SIRincif,SIRdieif,SIRrecif,SIRinfif}; // 5

  unsigned char (*instructionsetredcodew[])(unsigned char* cells, unsigned char IP) = {rdmov,rdadd,rdsub,rdjmp,rdjmz,rdjmg,rddjz,rddat,rdcmp,rdoutpw}; // 10

  unsigned char (*instructionsetbiotaw[])(unsigned char* cells, unsigned char IP) = {btempty,btoutpw,btstraight,btbackup,btturn,btunturn,btg,btclear,btdup}; // 9

  unsigned char (*instructionsetreddeathw[])(unsigned char* cells, unsigned char IP) = {redplague,reddeath,redclockw,redrooms,redunmask,redprosperow,redoutside}; // 7

  // *2

  unsigned char (*instructionsetfirstr[])(unsigned char* cells, unsigned char IP) = {outppr,finc,fdec,fincm,fdecm,fin1,outpr,plus,minus,bitshift1,bitshift2,bitshift3,branch,jump,infect,store,writesamp,skip,direction,die}; // 20 instructions

  unsigned char (*instructionsetplaguer[])(unsigned char* cells, unsigned char IP) = {writesamp, ploutpr, plenclose, plinfect, pldie, plwalk}; // 6

  unsigned char (*instructionsetbfr[])(unsigned char* cells, unsigned char IP) = {bfinc,bfdec,bfincm,bfdecm,bfoutpr,bfin,bfbrac1,bfbrac2}; // 8

  unsigned char (*instructionsetSIRr[])(unsigned char* cells, unsigned char IP) = {SIRoutpr,SIRincif,SIRdieif,SIRrecif,SIRinfif}; // 5

  unsigned char (*instructionsetredcoder[])(unsigned char* cells, unsigned char IP) = {rdmov,rdadd,rdsub,rdjmp,rdjmz,rdjmg,rddjz,rddat,rdcmp,rdoutpr}; // 10

  unsigned char (*instructionsetbiotar[])(unsigned char* cells, unsigned char IP) = {btempty,btoutpr,btstraight,btbackup,btturn,btunturn,btg,btclear,btdup}; // 9

  unsigned char (*instructionsetreddeathr[])(unsigned char* cells, unsigned char IP) = {redplague,reddeath,redclockr,redrooms,redunmask,redprosperor,redoutside}; // 7

  // ops: ++,-,&,|,division, *, bitshifts << >>

unsigned char inc(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink+param;}

unsigned char dec(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink-param;}

unsigned char inca(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=0;
  return ink+param;}

unsigned char deca(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=0;
  return ink-param;}

unsigned char incb(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=1;
  return ink+param;}

unsigned char decb(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=1;
  return ink-param;}

unsigned char incc(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=2;
  return ink+param;}

unsigned char decc(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=2;
  return ink-param;}

unsigned char incd(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=3;
  return ink+param;}

unsigned char decd(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=3;
  return ink-param;}


unsigned char andy(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink&=param;}

unsigned char orry(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink|=param;}

unsigned char excy(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink^=param;}

unsigned char divvy(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink/param;}

unsigned char starry(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink*param;}

unsigned char lefty(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink<<param;}

unsigned char righty(unsigned char* cellies, unsigned int ink, unsigned int param){
  return ink>>param;}

unsigned char rossy(unsigned char* cellies, unsigned int ink, unsigned int param){
  runross(&ross);
  return ross.intz<<param;
}

unsigned char rossya(unsigned char* cellies, unsigned int ink, unsigned int param){
  runross(&ross);
  modrr=0;
  return ross.intz<<param;
}

unsigned char rossyb(unsigned char* cellies, unsigned int ink, unsigned int param){
  runross(&ross);
  modrr=1;
  return ross.intz<<param;
}

unsigned char rossyc(unsigned char* cellies, unsigned int ink, unsigned int param){
  runross(&ross);
  modrr=2;
  return ross.intz<<param;
}

unsigned char rossyd(unsigned char* cellies, unsigned int ink, unsigned int param){
  runross(&ross);
  modrr=3;
  return ross.intz<<param;
}

unsigned char ifsy(unsigned char* cellies, unsigned int ink, unsigned int param){
  runifs(&ifs);
  return (ifs.returnvalx+1)<<param;
}

unsigned char ifsya(unsigned char* cellies, unsigned int ink, unsigned int param){
  runifs(&ifs);
  modrr=0;
  return (ifs.returnvalx+1)<<param;
}

unsigned char ifsyb(unsigned char* cellies, unsigned int ink, unsigned int param){
  runifs(&ifs);
  modrr=1;
  return (ifs.returnvalx+1)<<param;
}

unsigned char ifsyc(unsigned char* cellies, unsigned int ink, unsigned int param){
  runifs(&ifs);
  modrr=2;
  return (ifs.returnvalx+1)<<param;
}

unsigned char ifsyd(unsigned char* cellies, unsigned int ink, unsigned int param){
  runifs(&ifs);
  modrr=3;
  return (ifs.returnvalx+1)<<param;
}

unsigned char iffsy(unsigned char* cellies, unsigned int ink, unsigned int param){
  int x;
  x=(ink*ink);
  return x;
}

unsigned char swappy(unsigned char* cellies, unsigned int ink, unsigned int param){
  if (ink==dtae) return dtae;
  else return dtae;
}

unsigned char brainy(unsigned char* cellies, unsigned int ink, unsigned int param){
  runbrainw(cellies);
  return spointer<<param;
}

unsigned char ins1(unsigned char* cellies, unsigned int ink, unsigned int param){
  instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
  instructionp=(*instructionsetplaguew[instruction%6]) (cellies, instructionp);
  insdir=dir;
  if (cellies[instructionp%255]==255 && dir<0) dir=1;
  else if (cellies[instructionp%255]==255 && dir>0) dir=-1; // barrier
  return dtae;
}

unsigned char insl1(unsigned char* cellies, unsigned int ink, unsigned int param){
  instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
  instructionpr=(*instructionsetplaguew[instruction%6]) (cellies, instructionpr);
  insdirr=dirr;
  if (cells[instructionpr]==255 && dirr<0) dirr=1;
  else if (cells[instructionpr]==255 && dirr>0) dirr=-1; // barrier
  return dtae;
}


unsigned char sine(unsigned char* cellies, unsigned int ink, unsigned int param){
  ink+=param;
  return(sinewave[ink%255]<<param); // frequency
}

unsigned char insl2(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetfirstr[instruction%20]) ((unsigned char*)cellies, instructionpr);
      insdirr=dirr;
      return dtae;
}

unsigned char insl3(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetbfr[instruction%8]) ((unsigned char *)cellies, instructionpr);
      insdirr=dirr;
      return dtae;
}

unsigned char insl4(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetSIRr[instruction%5]) ((unsigned char *)cellies, instructionpr);
      insdirr=dirr;
      return dtae;
}


unsigned char insl5(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetredcoder[instruction%10]) ((unsigned char *)lsamp, instructionpr);
      insdirr=dirr;
      return dtae;
}

unsigned char insl6(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetreddeathr[instruction%7]) ((unsigned char *)lsamp, instructionpr);
      insdirr=dirr;
      return dtae;
}

unsigned char insl7(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionpr%grainsize));
      instructionpr=(*instructionsetbiotar[instruction%9]) ((unsigned char *)cellies, instructionpr); 
      if (btdir==0) instructionpr+=1;
      else if (btdir==1) instructionpr-=1;
      else if (btdir==2) instructionpr+=16;
      else if (btdir==3) instructionpr-=16;
      return dtae;
}

unsigned char ins2(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetfirstr[instruction%20]) ((unsigned char*)cellies, instructionp);
      insdir=dir;
      return dtae;
}

unsigned char ins3(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetbfr[instruction%8]) ((unsigned char *)cellies, instructionp);
      insdir=dir;
      return dtae;
}

unsigned char ins4(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetSIRr[instruction%5]) ((unsigned char *)cellies, instructionp);
      insdir=dir;
      return dtae;
}


unsigned char ins5(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetredcoder[instruction%10]) ((unsigned char *)lsamp, instructionp);
      insdir=dir;
      return dtae;
}

unsigned char ins6(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetreddeathr[instruction%7]) ((unsigned char *)lsamp, instructionp);
      insdir=dir;
      return dtae;
}

unsigned char ins7(unsigned char* cellies, unsigned int ink, unsigned int param){
      instruction=*(unsigned char *)(cellies+(instructionp%grainsize));
      instructionp=(*instructionsetbiotaw[instruction%9]) ((unsigned char *)cellies, instructionp); 
      if (btdir==0) instructionp+=1;
      else if (btdir==1) instructionp-=1;
      else if (btdir==2) instructionp+=16;
      else if (btdir==3) instructionp-=16;
      return dtae;
}


unsigned char worm(unsigned char* cellies, unsigned int ink, unsigned int param){

  if (*(cellies)%0x01) ink+=16;
  else if (*(cellies)%0x02) ink-=16;
  else if (*(cellies)%0x04) ink+=1;
  else if (*(cellies)%0x08) ink-=1;
  return ink<<param;
}

unsigned char back(unsigned char* cellies, unsigned int ink, unsigned int param){
  return (chunk-ink);
}

unsigned char munge(unsigned char* cellies, unsigned int ink, unsigned int param){
  *(cellies)+=*(cellies+1);
  return (ink++)<<param;
}

unsigned char coded(unsigned char* cellies, unsigned int ink, unsigned int param){
  return 0;
  }


unsigned char wredo(unsigned char* cellies, unsigned int ink, unsigned int param){
  grainsize=ink;
  return ink<<param;
}

unsigned char rredo(unsigned char* cellies, unsigned int ink, unsigned int param){
  rgrainsize=ink;
  return ink<<param;
}


unsigned char non(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=0;
  return 0;
}

unsigned char nona(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=0;
  return 0;
}

unsigned char nonb(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=1;
  return 0;
}

unsigned char nonc(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=2;
  return 0;
}

unsigned char nond(unsigned char* cellies, unsigned int ink, unsigned int param){
  modrr=3;
  return 0;
}

unsigned char hodge(unsigned char* cellies, unsigned int wr, unsigned int param){
  int sum=0, numill=0, numinf=0;
  unsigned char q,k1,k2,g;
  static unsigned char x;
  static unsigned char flag=0;
  unsigned char *newcells, *cells;
  x=chunk;
  if ((flag&0x01)==0) {
    cells=cellies; newcells=&cells[chunk/2];
  }
  else {
    cells=&cells[chunk/2]; newcells=cellies;
  }      

  q=cells[0];k1=cells[1];k2=cells[2];g=cells[3];
  if (k1==0) k1=1;
  if (k2==0) k2=1;

  sum=cells[x]+cells[x-1]+cells[x+1]+cells[x-chunk]+cells[x+chunk]+cells[x-chunk-1]+cells[x-chunk+1]+cells[x+chunk-1]+cells[x+chunk+1];

  if (cells[x-1]==(q-1)) numill++; else if (cells[x-1]>0) numinf++;
  if (cells[x+1]==(q-1)) numill++; else if (cells[x+1]>0) numinf++;
  if (cells[x-chunk]==(q-1)) numill++; else if (cells[x-chunk]>0) numinf++;
  if (cells[x+chunk]==(q-1)) numill++; else if (cells[x+chunk]>0) numinf++;
  if (cells[x-chunk-1]==q) numill++; else if (cells[x-chunk-1]>0) numinf++;
  if (cells[x-chunk+1]==q) numill++; else if (cells[x-chunk+1]>0) numinf++;
  if (cells[x+chunk-1]==q) numill++; else if (cells[x+chunk-1]>0) numinf++;
  if (cells[x+chunk+1]==q) numill++; else if (cells[x+chunk+1]>0) numinf++;

  if(cells[x] == 0)
    newcells[x%128] = floor(numinf / k1) + floor(numill / k2);
  else if(cells[x] < q - 1)
    newcells[x%128] = floor(sum / (numinf + 1)) + g;
  else
    newcells[x%128] = 0;

  if(newcells[x%128] > q - 1)
    newcells[x%128] = q - 1;

  x++;
  if (x>((chunk/2)-chunk-1)) {
    x=chunk+1;
    flag^=0x01;
  }
  return sum;
}

unsigned char cel(unsigned char* cels, unsigned int ink, unsigned int param){

  static unsigned char l=0; unsigned char cell, state, res;
  unsigned char *cells=(unsigned char*)(cels+(CELLLEN*CELLLEN));
  unsigned char rule=13;
  res=0;
  l++;
  l%=CELLLEN;

  for (cell = 1; cell < CELLLEN; cell++){ 
      state = 0;
      if (cells[cell + 1+ (l*CELLLEN)]>128)
	state |= 0x4;
      if (cells[cell+(CELLLEN*l)]>128)
	state |= 0x2;
      if (cells[cell - 1 +(CELLLEN*l)]>128)
	state |= 0x1;
                     
      if ((rule >> state) & 1){
	res += 1; 
	cells[cell+(((l+1)%CELLLEN)*CELLLEN)] = 255;
      }
      else{
	cells[cell+(((l+1)%CELLLEN)*CELLLEN)] = 0;
      } 
  }
  return res;
}

unsigned char SIR(unsigned char* cellies, unsigned int ink, unsigned int param){
  unsigned char cell,x,sum=0;
  static unsigned char flag=0;
  unsigned char *newcells, *cells;
  unsigned char kk=cellies[0], p=cellies[1];

  if ((flag&0x01)==0) {
    cells=(unsigned char*)cellies; newcells=&cells[256];
  }
  else {
    cells=&cells[256]; newcells=cellies;
  }      


  for (x=CELLLEN;x<(256-CELLLEN);x++){
    cell = cells[x];
    newcells[x]=cell;
    if (cell >= kk) newcells[x] = recovered;                                                 else if ((cell>0 && cell<kk)){
      newcells[x]++;                                                       
    }
    else if (cell == susceptible) {   
      sum++;
      if ( (cells[x-CELLLEN]>0 && cells[x-CELLLEN]<kk) ||
	   (cells[x+CELLLEN]>0 && cells[x+CELLLEN]<kk) ||
	   (cells[x-1]>0 && cells[x-1]<kk) ||
	   (cells[x+1]>0 && cells[x+1]<kk))
	{
	if (rand()%10 < p) newcells[x] = 1;       
      }
    }
  }
  flag^=0x01;
  return sum;
}


unsigned char life(unsigned char* cellies, unsigned int ink, unsigned int param){
  unsigned char x, sum;

  static unsigned char flag=0;
  unsigned char *newcells, *cells;

  if ((flag&0x01)==0) {
    cells=cellies; newcells=&cells[128];
  }
  else {
    cells=&cellies[128]; newcells=cellies;
  }      

  for (x=CELLLEN+1;x<(128-CELLLEN-1);x++){
    sum=cells[x]%2+cells[x-1]%2+cells[x+1]%2+cells[x-CELLLEN]%2+cells[x+CELLLEN]%2+cells[x-CELLLEN-1]%2+cells[x-CELLLEN+1]%2+cells[x+CELLLEN-1]%2+cells[x+CELLLEN+1]%2;
    sum=sum-cells[x]%2;
    if (sum==3 || (sum+(cells[x]%2)==3)) newcells[x]=255;
    else newcells[x]=0;
  }
  
  // swapping 
    flag^=0x01;
  return sum;
}



int main(int argc, char **argv) 
{int x,xx=0;
  dtae=13;
unsigned char (*wplag[])(unsigned char* cells, unsigned int rt, unsigned int p) = {hodge,inc,dec,andy,orry,excy,divvy,starry,lefty,righty,iffsy, swappy, cel,SIR,life,rossy,ifsy,brainy,ins1,sine,ins2,ins3,ins4,ins5,ins6,ins7,worm,back,munge,coded,rredo,non}; //32!

///0hodge,1inc,2dec,3andy,4orry,5excy,6divvy,7starry,8lefty,9righty,10iffsy, 11swappy, 12cel,13SIR,14life,15rossy,16ifsy,17brainy,18ins1*,19sineOK,20ins2,21ins3,22ins4-OK,23ins5-ok,24ins6,25ins7-ok,26worm-ok,//27back,28munge,29coded,30rredo,31non}; //32!

//problems with: scaler makes no sense for logic opps

// fixes to: runbrainw (doesn't get stuck), check in cels and life how swap was working, also in all how cellies and cells are assigned

// fix this then test how they SOUND!! poor=12,14 instruction sets sound a bit similar!

 int dii=atoi(argv[1]),scaler=1;
  srandom(time(0));

  for (x=0;x<512;x++){
    xxx[x]=rand()%255;
  }
  lsamp=xxx;


  initross(&ross);
  initifs(&ifs);
  x=0;
  while(1){
  for (x=0;x<512;x++){
    xxx[x]=rand()%255;
  }
  lsamp=xxx;

    xx++;
    dtae=(*wplag[dii])((unsigned char*)lsamp,dtae,scaler);
    printf("%c", dtae);


  }
}
