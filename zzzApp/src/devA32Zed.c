/* 	devA32Vme.c ---> devA32Zed.c	*/

/*****************************************************************
 *
 *      Author :                     John T Weizeorick
 *      Date:                        03/25/2016
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *****************************************************************
 *                         COPYRIGHT NOTIFICATION
 *****************************************************************

 * THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
 * AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
 * AND IN ALL SOURCE LISTINGS OF THE CODE.
 
 * (C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO
 
 * Argonne National Laboratory (ANL), with facilities in the States of 
 * Illinois and Idaho, is owned by the United States Government, and
 * operated by the University of Chicago under provision of a contract
 * with the Department of Energy.

 * Portions of this material resulted from work developed under a U.S.
 * Government contract and are subject to the following license:  For
 * a period of five years from March 30, 1993, the Government is
 * granted for itself and others acting on its behalf a paid-up,
 * nonexclusive, irrevocable worldwide license in this computer
 * software to reproduce, prepare derivative works, and perform
 * publicly and display publicly.  With the approval of DOE, this
 * period may be renewed for two additional five year periods. 
 * Following the expiration of this period or periods, the Government
 * is granted for itself and others acting on its behalf, a paid-up,
 * nonexclusive, irrevocable worldwide license in this computer
 * software to reproduce, prepare derivative works, distribute copies
 * to the public, perform publicly and display publicly, and to permit
 * others to do so.

 *****************************************************************
 *                               DISCLAIMER
 *****************************************************************

 * NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
 * THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
 * MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
 * LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
 * USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
 * DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
 * OWNED RIGHTS.  

 *****************************************************************
 * LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
 * DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
 *****************************************************************

 * Modification Log:
 * -----------------
 * 01-23-98   nda       initially functional Ned Arnold
 * 02-25-98   mr        modified ai and ao to support 2's complement.
 * 07-22-98   mr        Fixed Param field to accomadate both i`uni and bi polar
 * 			Inputs and outputs (AI, AO records)..
 * 10-06-98   nda       fixed a bug with li,lo,ai,ao where sum of bit+
 *                      numbits > MAX_ACTIVE_BITS
 * 03-21-2016 jtw	Changed devA32Vme to devA32Zed - Non VxWorks!!
 * 03-22-2016 jtw	Need to register calls to shell
 * 03-23-2016 jtw	writing wf record to scan pixel given by lo with signal = 10
 * 04-11-2016 jtw	modify write_lo to set pixelToScanLO signal = 10
 * 04-22-2016 jtw       modify write_lo to set DAC Trim Bits LO with signal = 11
 * 04-25-2016 jtw	modify write_lo to set Configure Bits to Count LO with signal = 12
*              
 */


/*To Use this device support, Include the following before iocInit */
/* devA32ZedConfig(card,a32base,nreg,iVector,iLevel)  */
/*    card    = card number                           */
/*    a32base = base address of card                  */
/*    nreg    = number of A32 registers on this card  */
/*    iVector = interrupt vector (MRD100 ONLY !!)     */
/*    iLevel  = interrupt level  (MRD100 ONLY !!)     */
/* For Example					      */
/* devA32ZedConfig(0, 0x80000000, 44, 0x3e, 5)        */


 /**********************************************************************/
 /** Brief Description of device support                              **/
 /**						    	              **/
 /** This device support allows access to any register of a ZED       **/
 /** module found in the A32/D32 ZED space. The bit field of interest **/
 /** is described in the PARM field of the INP or OUT link.           **/
 /** This allows a generic driver to be used without hard-coding      **/
 /** register numbers within the software.                            **/
 /**						    	              **/
 /** Record type     Signal #           Parm Field                    **/
 /**                                                                  **/
 /**    ai          reg_offset     lsb, width, type                   **/
 /**    ao          reg_offset     lsb, width, type                   **/
 /**    bi          reg_offset     bit #                              **/
 /**    bo          reg_offset     bit #                              **/
 /**    longin      reg_offset     lsb, width                         **/
 /**    longout     reg_offset     lsb, width                         **/
 /**    mbbi        reg_offset     lsb, width                         **/
 /**    mbbo        reg_offset     lsb, width                         **/
 /**                                                                  **/
 /** reg_offset is specified by the register number (0,1,2,3, etc)    **/
 /** Parm field must be provided, no defaults are assumed ...         **/
 /** In ai and ao type is either 0 - unipolar, 1 -bipolar             **/
 /**                                                                  **/
 /**                                                                  **/
 /**********************************************************************/

//#include	<vxWorks.h>
//#include	<vxLib.h>
//#include	<sysLib.h>
//#include	<vme.h>
//#include	<types.h>
//#include	<stdioLib.h>
#include	<stdlib.h>
//#include	<intLib.h>
#include	<string.h>
#include	<math.h>
//#include	<iv.h>
//#include	<rebootLib.h>

#include	<stddef.h>
#include	<stdio.h>
#include	<sys/types.h>

#include	<unistd.h>
#include	<sys/mman.h>
#include	<fcntl.h>
#include	<sys/stat.h>

#include	<alarm.h>
#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<recGbl.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<link.h>

#include	<epicsPrint.h>
#include	<epicsExport.h>
#include 	<iocsh.h>

#include	<aoRecord.h>
#include	<aiRecord.h>
#include	<boRecord.h>
#include	<biRecord.h>
#include	<longinRecord.h>
#include	<longoutRecord.h>
#include	<mbboRecord.h>
#include	<mbbiRecord.h>

#include        <subRecord.h>
#include	<waveformRecord.h>

#include	<dbScan.h>

#include 	<linux/i2c-dev.h>

#define ERROR (-1)


// vipic c code info
// Register Addresses
#define FPGAVER 4
#define PIXCFG 8
#define FRAMETRIG 9
#define GLOBRST 12
#define WINTIMEPAD 14
#define FIFODATA 20 
#define FIFOWDCNT 21 
#define FIFORST 22 
#define FRAMENUM 24
#define DACTF 0.000152588   // 2.5v / 2^14 

static long init_ai(), read_ai();
static long init_ao(), write_ao();
static long init_bi(), read_bi();
static long init_bo(), write_bo();
static long init_li(), read_li();
static long init_lo(), write_lo();
static long init_mbbi(), read_mbbi();
static long init_mbbo(), write_mbbo();
static long checkCard(), write_card(), read_card();
static long get_bi_int_info();
static void devA32_isr();
static void devA32RebootFunc();

static long initWfRecord(struct waveformRecord*);
static long readWf(struct waveformRecord*);

static long devA32ZedReport();

int scanPixelN( int pixel, int datTrimSet, int configPixelCnt, waveformRecord *pwf );
int pixelToScanLO = 0;
int dacTrim = 0;
int configPixelCnt = 1984;


int  devA32ZedDebug = 0;
int  OK = 0;

/***** devA32ZedDebug *****/

/** devA32ZedDebug == 0 --- no debugging messages **/
/** devA32ZedDebug >= 5 --- hardware initialization information **/
/** devA32ZedDebug >= 10 -- record initialization information **/
/** devA32ZedDebug >= 15 -- write commands **/
/** devA32ZedDebug >= 20 -- read commands **/


#define MAX_NUM_CARDS    4
#define MAX_A32_ADDRESS  0xf0000000
#define MAX_ACTIVE_REGS  256   /* largest register number allowed */
#define MAX_ACTIVE_BITS  32   /* largest bit # expected */
#define MAX_PIXEL 1024

#define IVEC_REG           29   /* Interrupt Vector Register (MRD100 ONLY) */
#define IVEC_ENA_REG       28   /* Interrupt Enable Register (MRD100 ONLY) */
#define IVEC_MASK          0xff /* Interrupt Vector Mask     (MRD100 ONLY) */
#define IVEC_ENA_MASK      0x10 /* Interrupt Enable Mask     (MRD100 ONLY) */
#define IVEC_REENABLE_MASK 0xf  /* Interrupt Re-enable Mask  (MRD100 ONLY) */
#define IVEC_REENABLE_REG  27   /* Interrupt Re-enable Reg   (MRD100 ONLY) */

/* Register layout */
typedef struct a32Reg {
  unsigned long reg[MAX_ACTIVE_REGS];
}a32Reg;

typedef struct ioCard {  /* Unique for each card */
  volatile a32Reg  *base;    /* address of this card's registers */
  int               nReg;    /* Number of registers on this card */
  epicsMutexId      lock;    /* semaphore */
  IOSCANPVT         ioscanpvt; /* records to process upon interrupt */
}ioCard;

static struct ioCard cards[MAX_NUM_CARDS]; /* array of card info */

typedef struct a32ZedDpvt { /* unique for each record */
  unsigned short  reg;   /* index of register to use (determined by signal #*/
  unsigned short  lbit;  /* least significant bit of interest */
  unsigned short  nbit;  /* no of significant bits */
  unsigned short  type;  /* Type either 0 or 1 for uni, bi polar */
  unsigned long   mask;  /* mask to use ...  */
  unsigned long   pixel; /* pixel to do threshold scan on */
}a32ZedDpvt;

/* Define the dset for A32ZED */
typedef struct {
	long		number;
	DEVSUPFUN	report;		/* used by dbior */
	DEVSUPFUN	init;	
	DEVSUPFUN	init_record;	/* called 1 time for each record */
	DEVSUPFUN	get_ioint_info;	
	DEVSUPFUN	read_write;
        DEVSUPFUN       special_linconv;
} A32ZED_DSET;

A32ZED_DSET devAiA32Zed =   {6, NULL, NULL, init_ai, NULL, read_ai,  NULL};
A32ZED_DSET devAoA32Zed =   {6, NULL, NULL, init_ao, NULL, write_ao, NULL};
A32ZED_DSET devBiA32Zed =   {5, devA32ZedReport,NULL,init_bi, get_bi_int_info, 
                             read_bi,  NULL};
A32ZED_DSET devBoA32Zed =   {5, NULL, NULL, init_bo, NULL, write_bo, NULL};
A32ZED_DSET devLiA32Zed =   {5, NULL, NULL, init_li, NULL, read_li,  NULL};
A32ZED_DSET devLoA32Zed =   {5, NULL, NULL, init_lo, NULL, write_lo, NULL};
A32ZED_DSET devMbbiA32Zed = {5, NULL, NULL, init_mbbi, NULL, read_mbbi,  NULL};
A32ZED_DSET devMbboA32Zed = {5, NULL, NULL, init_mbbo, NULL, write_mbbo, NULL};

// read 
A32ZED_DSET devWaveformA32Zed = { 6, NULL, NULL, initWfRecord, NULL, readWf,  NULL};

epicsExportAddress(dset, devAiA32Zed);
epicsExportAddress(dset, devAoA32Zed);
epicsExportAddress(dset, devBiA32Zed);
epicsExportAddress(dset, devBoA32Zed);
epicsExportAddress(dset, devLiA32Zed);
epicsExportAddress(dset, devLoA32Zed);
epicsExportAddress(dset, devMbbiA32Zed);
epicsExportAddress(dset, devMbboA32Zed);
epicsExportAddress(dset, devWaveformA32Zed);


/**************************************************************************
 **************************************************************************/
static long devA32ZedReport()
{
int             i;
int		cardNum = 0;
unsigned long   regData;

  for(cardNum=0; cardNum < MAX_NUM_CARDS; cardNum++) {
    if(cards[cardNum].base != NULL) {
      epicsPrintf("  Card #%d at %p\n", cardNum, cards[cardNum].base);
      for(i=0; i < cards[cardNum].nReg; i++) {
          regData = cards[cardNum].base->reg[i];
          epicsPrintf("    Register %d -> 0x%4.4lX (%ld)\n", i, regData, regData);
      }
    }
  }
return(0);
}


/**************************************************************************
*
* Initialization of A32/D32 Card - called in st.cmd
*
***************************************************************************/
int devA32ZedConfig(card,a32base,nregs,iVector,iLevel)
int	      card;
unsigned long a32base;
//int           a32base;
int	      nregs;
int	      iVector;
int	      iLevel;
{

  //unsigned long probeVal;
  int	fd;

  if((card >= MAX_NUM_CARDS) || (card < 0)) {
      epicsPrintf("devA32ZedConfig: Invalid Card # %d \n",card);
      return(ERROR);
  }

  if(a32base >= MAX_A32_ADDRESS) {
      epicsPrintf("devA32ZedConfig: Invalid Card Address %ld \n",a32base);
      return(ERROR);
  }


  // microZed Open /dev/mem to write to FPGA registers 
  fd = open("/dev/mem",O_RDWR|O_SYNC);
  if (fd < 0) {
	epicsPrintf("Can't Open /dev/mem\n");
	return(ERROR);
  }

  cards[card].base = (unsigned int *) mmap(0,255,PROT_READ|PROT_WRITE,MAP_SHARED,fd,a32base);
  if (cards[card].base == NULL) {
       epicsPrintf("devA32ZedConfig: mmap A32 Address map failed for Card %d\n",card);
  }
  else epicsPrintf("devA32ZedConfig: mmap A32 Address map Successful for Card %d\n",card);

  if(nregs > MAX_ACTIVE_REGS) {
      epicsPrintf("devA32ZedConfig: # of registers (%d) exceeds max\n",nregs);
      return(ERROR);
  }
  else {
      cards[card].nReg = nregs;
      cards[card].lock = epicsMutexMustCreate();
  }
 
/*
  if(iVector) {
      scanIoInit(&cards[card].ioscanpvt);
      if(intConnect(INUM_TO_IVEC(iVector), (VOIDFUNCPTR)devA32_isr, 
                    (int)card) == OK)
      {
          cards[card].base->reg[IVEC_REG] = (unsigned long)iVector;
          write_card(card, IVEC_REG, IVEC_MASK, (unsigned long)iVector);
          write_card(card, IVEC_REENABLE_REG, IVEC_REENABLE_MASK, 
                     IVEC_REENABLE_MASK);
          write_card(card, IVEC_ENA_REG, IVEC_ENA_MASK, IVEC_ENA_MASK);
          sysIntEnable(iLevel);
      }
      else {
          epicsPrintf("devA32ZedConfig: Interrupt connect failed for card %d\n",
                          card);
      }
      rebootHookAdd((FUNCPTR)devA32RebootFunc);
  }
*/
  return(OK);
}

/**************************************************************************
 *
 * BI record interrupt routine
 *
 **************************************************************************/
static long get_bi_int_info(cmd, pbi, ppvt)
int                     cmd;
struct biRecord         *pbi;
IOSCANPVT               *ppvt;
{

   struct vmeio           *pvmeio = (struct vmeio *)(&pbi->inp.value);

   if(cards[pvmeio->card].ioscanpvt != NULL) {
       *ppvt = cards[pvmeio->card].ioscanpvt;
       return(OK);
   }
   else {
       return(ERROR);
   }
}


/**************************************************************************
 *
 * Interrupt service routine
 *
 **************************************************************************/
static void devA32_isr(int card)
{
   scanIoRequest(cards[card].ioscanpvt);
   write_card(card, IVEC_REENABLE_REG, IVEC_REENABLE_MASK, IVEC_REENABLE_MASK);
}


/******************************************************************************
 *
 * A function to disable interrupts in case we get a ^X style reboot.
 *
 ******************************************************************************/
static void devA32RebootFunc(void)
{
  int   card = 0;

  while (card < MAX_NUM_CARDS)
  {
    if (cards[card].ioscanpvt != NULL) {
        write_card(card, IVEC_ENA_REG, IVEC_ENA_MASK, 0);
    }
    card++;
  }
  return;
}


/**************************************************************************
 *
 * BO Initialization (Called one time for each BO MSLT card record)
 *
 **************************************************************************/
static long init_bo(pbo)
struct boRecord *pbo;
{
    long                status = 0;
    int                 card, args, bit;
    int			link, address, parm;
    unsigned long 	rawVal = 0;
    a32ZedDpvt         *pPvt;

    if (devA32ZedDebug >= 20)
    {
	if (pbo->out.type == VME_IO)
         epicsPrintf("init_bo: card %d, regNum %d, mask %s\n", 
	   pbo->out.value.vmeio.card, pbo->out.value.vmeio.signal, pbo->out.value.vmeio.parm );
    }

    /* bo.out must be an VME_IO ??? */
    switch (pbo->out.type) {
    case (VME_IO) :
 
      if(pbo->out.value.vmeio.card > MAX_NUM_CARDS) {
	pbo->pact = 1;		/* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pbo->out.value.vmeio.card , pbo->name);
        return(ERROR);
      }

      card = pbo->out.value.vmeio.card;

      if(cards[card].base == NULL) {
	pbo->pact = 1;		/* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
        	card, pbo->name);
        return(ERROR);
      }

      if (pbo->out.value.vmeio.signal >= cards[card].nReg) {
        pbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pbo->name);
        return(ERROR);
      }

      args = sscanf(pbo->out.value.vmeio.parm, "%d", &bit);
 
      if((args != 1) || (bit >= MAX_ACTIVE_BITS)) {
        pbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Bit # in parm field: ->%s<-\n",
                     pbo->name);
        return(ERROR);
      }

      pbo->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pbo->dpvt;

      pPvt->reg =  pbo->out.value.vmeio.signal;
      pPvt->lbit = bit;
      pPvt->mask = 1 << pPvt->lbit;
      pbo->mask = pPvt->mask;

      if (read_card(card, pPvt->reg, pPvt->mask, &rawVal) == OK)
         {
         pbo->rbv = pbo->rval = rawVal;
         }
      else 
         {
         status = 2;
         }
      break;
         
    default :
	pbo->pact = 1;		/* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal OUT field ->%s<- \n", pbo->name);
        status = ERROR;
    }
    return(status);
}

/**************************************************************************
 *
 * BI Initialization (Called one time for each BI record)
 *
 **************************************************************************/
static long init_bi(pbi)
struct biRecord *pbi;
{
    long                status = 0;
    int                 card, args, bit;
    unsigned long       rawVal = 0;
    a32ZedDpvt         *pPvt;
   

    /* bi.inp must be an VME_IO */
    switch (pbi->inp.type) {
    case (VME_IO) :

      if(pbi->inp.value.vmeio.card > MAX_NUM_CARDS) {
        pbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pbi->inp.value.vmeio.card , pbi->name);
        return(ERROR);
      }

      card = pbi->inp.value.vmeio.card;

      if(cards[card].base == NULL) {
        pbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
        	card, pbi->name);
        return(ERROR);
      }

      if (pbi->inp.value.vmeio.signal >= cards[card].nReg) {
        pbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal #%d exceeds registers: ->%s<-\n",
                     pbi->inp.value.vmeio.signal, pbi->name);
        return(ERROR);
      }

      args = sscanf(pbi->inp.value.vmeio.parm, "%d", &bit);

      if((args != 1) || (bit >= MAX_ACTIVE_BITS)) {
        pbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Bit # in parm field: ->%s<-\n",
                     pbi->name);
        return(ERROR);
      }
      pbi->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pbi->dpvt;

      pPvt->reg =  pbi->inp.value.vmeio.signal;
      pPvt->lbit = bit;
      pPvt->mask = 1 << pPvt->lbit;
      pbi->mask = pPvt->mask;

      if (read_card(card, pPvt->reg, pPvt->mask, &rawVal) == OK)
         {
         pbi->rval = rawVal;
         status = 0;
         }
      else
         {
         status = 2;
         }
      break;

    default :
        pbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal INP field ->%s<- \n", pbi->name);
        status = ERROR;
    }
    return(status);
}


/**************************************************************************
 *
 * MBBO Initialization (Called one time for each MBBO record)
 *
 **************************************************************************/
static long init_mbbo(pmbbo)
struct mbboRecord   *pmbbo;
{
    long                status = 0;
    int                 card, args, bit;
    unsigned long       rawVal = 0;
    a32ZedDpvt         *pPvt;

    /* mbbo.out must be an VME_IO */
    switch (pmbbo->out.type) {
    case (VME_IO) :
      if(pmbbo->out.value.vmeio.card > MAX_NUM_CARDS) {
        pmbbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pmbbo->out.value.vmeio.card , pmbbo->name);
        return(ERROR);
      }

      card = pmbbo->out.value.vmeio.card;

      if(cards[card].base == NULL) {
        pmbbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pmbbo->name);
        return(ERROR);
      }

      if (pmbbo->out.value.vmeio.signal >= cards[card].nReg) {
        pmbbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pmbbo->name);
        return(ERROR);
      }

      args = sscanf(pmbbo->out.value.vmeio.parm, "%d", &bit);

      if((args != 1) || (bit >= MAX_ACTIVE_BITS)) {
        pmbbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Bit # in parm field: ->%s<-\n",
                     pmbbo->name);
        return(ERROR);
      }

      pmbbo->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pmbbo->dpvt;

      pPvt->reg =  pmbbo->out.value.vmeio.signal;
      pPvt->lbit = bit;

      /* record support determined .mask from .nobt, need to adjust */
      pmbbo->shft = pPvt->lbit;
      pmbbo->mask <<= pPvt->lbit;
      pPvt->mask = pmbbo->mask;

      if (read_card(card, pPvt->reg, pPvt->mask, &rawVal) == OK)
         {
         pmbbo->rbv = pmbbo->rval = rawVal;
         status = 0;
         }
      else
         {
         status = 2;
         }
      break;

    default :
        pmbbo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal OUT field ->%s<- \n", pmbbo->name);
        status = ERROR;
    }
    return(status);
}


/**************************************************************************
 *
 * MBBI Initialization (Called one time for each MBBO record)
 *
 **************************************************************************/
static long init_mbbi(pmbbi)
struct mbbiRecord   *pmbbi;
{
    long                status = 0;
    int                 card, args, bit;
    unsigned long       rawVal = 0;
    a32ZedDpvt         *pPvt;

    /* mbbi.inp must be an VME_IO */
    switch (pmbbi->inp.type) {
    case (VME_IO) :
      if(pmbbi->inp.value.vmeio.card > MAX_NUM_CARDS) {
        pmbbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pmbbi->inp.value.vmeio.card , pmbbi->name);
        return(ERROR);
      }

      card = pmbbi->inp.value.vmeio.card;

      if(cards[card].base == NULL) {
        pmbbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pmbbi->name);
        return(ERROR);
      }

      if (pmbbi->inp.value.vmeio.signal >= cards[card].nReg) {
        pmbbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pmbbi->name);
        return(ERROR);
      }

      args = sscanf(pmbbi->inp.value.vmeio.parm, "%d", &bit);

      if((args != 1) || (bit >= MAX_ACTIVE_BITS)) {
        pmbbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Bit # in parm field: ->%s<-\n",
                     pmbbi->name);
        return(ERROR);
      }

      pmbbi->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pmbbi->dpvt;

      pPvt->reg =  pmbbi->inp.value.vmeio.signal;
      pPvt->lbit = bit;

      /* record support determined .mask from .nobt, need to adjust */
      pmbbi->shft = pPvt->lbit;
      pmbbi->mask <<= pPvt->lbit;
      pPvt->mask = pmbbi->mask;

      if (read_card(card, pPvt->reg, pPvt->mask, &rawVal) == OK)
         {
         pmbbi->rval = rawVal;
         status = 0;
         }
      else
         {
         status = 2;
         }
      break;

    default :
        pmbbi->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal INP field ->%s<- \n", pmbbi->name);
        status = ERROR;
    }
    return(status);
}


/**************************************************************************
 *
 * AI Initialization (Called one time for each AI record)
 *
 **************************************************************************/
static long init_ai(pai)
struct aiRecord   *pai;
{
    long                status = 0;
    int                 card, args, bit, numBits, twotype;
    a32ZedDpvt         *pPvt;

    /* ai.inp must be an VME_IO */
    switch (pai->inp.type) {
    case (VME_IO) :
      if(pai->inp.value.vmeio.card > MAX_NUM_CARDS) {
        pai->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pai->inp.value.vmeio.card , pai->name);
        return(ERROR);
      }

      card = pai->inp.value.vmeio.card;

      if(cards[card].base == NULL) {
        pai->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pai->name);
        return(ERROR);
      }

      if (pai->inp.value.vmeio.signal >= cards[card].nReg) {
        pai->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pai->name);
        return(ERROR);
      }

      args = sscanf(pai->inp.value.vmeio.parm, "%d,%d,%d", 
      				&bit, &numBits, &twotype);

      if( (args != 3) || (bit >= MAX_ACTIVE_BITS) || (numBits <= 0) ||
         	(bit + numBits > MAX_ACTIVE_BITS) ||
         	(twotype > 1) || (twotype < 0) ) {
        pai->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf
        ("devA32Zed: Invalid Bit #/Width/Type in parm field: ->%s<-\n",
                     pai->name);
        return(ERROR);
      }

      pai->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pai->dpvt;

      pPvt->reg =  pai->inp.value.vmeio.signal;
      pPvt->lbit = bit;
      pPvt->nbit = numBits;
      pPvt->type = twotype;
      pPvt->mask = ((1 << (numBits)) - 1) << pPvt->lbit;

      pai->eslo = (pai->eguf - pai->egul)/(pow(2,numBits)-1);
      
/*  Shift Raw value if Bi-polar */
      if (pPvt->type ==1) 
         pai->roff = pow(2,(numBits-1));

      status = OK;

      break;
    default :
        pai->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal INP field ->%s<- \n", pai->name);
        status = ERROR;
    }
    return(status);
}



/**************************************************************************
 *
 * AO Initialization (Called one time for each AO record)
 *
 **************************************************************************/
static long init_ao(pao)
struct aoRecord   *pao;
{
    long                status = 0;
    unsigned long       rawVal = 0;
    int                 card, args, bit, numBits, twotype;
    a32ZedDpvt         *pPvt;

    /* ao.out must be an VME_IO */
    switch (pao->out.type) {
    case (VME_IO) :
      if(pao->out.value.vmeio.card > MAX_NUM_CARDS) {
        pao->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pao->out.value.vmeio.card , pao->name);
        return(ERROR);
      }

      card = pao->out.value.vmeio.card;

      if(cards[card].base == NULL) {
        pao->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pao->name);
        return(ERROR);
      }

      if (pao->out.value.vmeio.signal >= cards[card].nReg) {
        pao->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pao->name);
        return(ERROR);
      }

      args = sscanf(pao->out.value.vmeio.parm, "%d,%d,%d", 
      				&bit, &numBits, &twotype);

      if( (args != 3) || (bit >= MAX_ACTIVE_BITS) || (numBits <= 0) ||
         	(bit + numBits > MAX_ACTIVE_BITS) ||
         	(twotype > 1) || (twotype < 0) ) {
        epicsPrintf
        ("devA32Zed: Invalid Bit #/Width/Type in parm field: ->%s<-\n",
                     pao->name);
        return(ERROR);
      }

      pao->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pao->dpvt;

      pPvt->reg =  pao->out.value.vmeio.signal;
      pPvt->lbit = bit;
      pPvt->nbit = numBits;
      pPvt->type = twotype;
      pPvt->mask = ((1 << (numBits)) - 1) << pPvt->lbit;

      pao->eslo = (pao->eguf - pao->egul)/(pow(2,numBits)-1);

/*  Shift Raw value if Bi-polar */
      if (pPvt->type == 1) 
         pao->roff = pow(2,(numBits-1));

      /* Init rval to current setting */ 
      if(read_card(card,pPvt->reg,pPvt->mask,&rawVal) == OK) {
        pao->rbv = rawVal>>pPvt->lbit;

/* here is where we do the sign extensions for Bipolar.... */        
        if (pPvt->type ==1) {
           if (pao->rbv & (2<<(pPvt->nbit-2)))
               pao->rbv |= ((2<<31) - (2<<(pPvt->nbit-2)))*2 ;

	}

        pao->rval = pao->rbv;
      }

      status = OK;

      break;
    default :
        pao->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal OUT field ->%s<- \n", pao->name);
        status = ERROR;
    }
    return(status);
}


/**************************************************************************
 *
 * LI Initialization (Called one time for each LI record)
 *
 **************************************************************************/
static long init_li(pli)
struct longinRecord   *pli;
{
    long                status = 0;
    int                 card, args, bit, numBits;
    a32ZedDpvt         *pPvt;

    /* li.inp must be an VME_IO */
    switch (pli->inp.type) {
    case (VME_IO) :
      if(pli->inp.value.vmeio.card > MAX_NUM_CARDS) {
        pli->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pli->inp.value.vmeio.card , pli->name);
        return(ERROR);
      }

      card = pli->inp.value.vmeio.card;

      if(cards[card].base == NULL) {
        pli->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pli->name);
        return(ERROR);
      }

      if (pli->inp.value.vmeio.signal >= cards[card].nReg) {
        pli->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal #%d exceeds registers: ->%s<-\n",
                     pli->inp.value.vmeio.signal, pli->name);
        return(ERROR);
      }

      args = sscanf(pli->inp.value.vmeio.parm, "%d,%d", &bit, &numBits);

      if((args != 2) || (bit >= MAX_ACTIVE_BITS) || (numBits <= 0) ||
         (bit + numBits > MAX_ACTIVE_BITS)) {
        pli->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Bit #/Width in parm field: ->%s<-\n",
                     pli->name);
        return(ERROR);
      }

      pli->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pli->dpvt;

      pPvt->reg =  pli->inp.value.vmeio.signal;
      pPvt->lbit = bit;
      pPvt->mask = ((1 << (numBits)) - 1) << pPvt->lbit;

      status = OK;


      break;
    default :
        pli->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal INP field ->%s<- \n", pli->name);
        status = ERROR;
    }
    return(status);
}



/**************************************************************************
 *
 * Long Out Initialization (Called one time for each LO record)
 *
 **************************************************************************/
static long init_lo(plo)
struct longoutRecord   *plo;
{
    long                status = 0;
    unsigned long       rawVal = 0;
    int                 card, args, bit, numBits;
    a32ZedDpvt         *pPvt;

    /* lo.out must be an VME_IO */
    switch (plo->out.type) {
    case (VME_IO) :
      if(plo->out.value.vmeio.card > MAX_NUM_CARDS) {
        plo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	plo->out.value.vmeio.card , plo->name);
        return(ERROR);
      }

      card = plo->out.value.vmeio.card;

      if(cards[card].base == NULL) {
        plo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, plo->name);
        return(ERROR);
      }

      if (plo->out.value.vmeio.signal >= cards[card].nReg) {
        plo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal #%d exceeds registers: ->%s<-\n",
                     plo->out.value.vmeio.signal, plo->name);
        return(ERROR);
      }

      args = sscanf(plo->out.value.vmeio.parm, "%d,%d", &bit, &numBits);

      if((args != 2) || (bit >= MAX_ACTIVE_BITS) || (numBits <= 0) ||
         (bit + numBits > MAX_ACTIVE_BITS)) {
        plo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed-init_lo: Invalid Bit #/Width bit=%d, numBits=%d in parm field: ->%s<-\n",
                     bit, numBits, plo->name);
        return(ERROR);
      }


      // are one of these lines causing a segmentation fault???
      plo->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)plo->dpvt;

      //epicsPrintf("devA32Zed-init_lo ok to 1\n");

      pPvt->reg =  plo->out.value.vmeio.signal;
      //pPvt->lbit = 32;		// ?????????
      //pPvt->mask = 0xFFFFFFFF;
      pPvt->lbit = bit;
      pPvt->mask = ((1 << (numBits)) - 1) << pPvt->lbit;


      //epicsPrintf("devA32Zed-init_lo ok to 2 bit=%d, numBits=%d sizeof=%d\n",bit,numBits,sizeof(struct a32ZedDpvt));
      //epicsPrintf("devA32Zed-init_lo ok to 2 plo->dol.type %s\n",plo->dol.type);


      /* if .dol is NOT a constant, initialize .val field to readback value */
      //if ((plo->dol.type == CONSTANT) && 
      //    (strlen(plo->dol.value.constantStr) == 0)) {
      //  epicsPrintf("devA32Zed-init_lo ok to 3\n");
      //    if (read_card(card,pPvt->reg,pPvt->mask,&rawVal) == OK) {
      //	      epicsPrintf("devA32Zed-init_lo ok to 4\n");
      //        plo->val = rawVal>>pPvt->lbit;
      //        plo->udf = 0;
      //    }
      //}

      status = OK;
      epicsPrintf("devA32Zed-init_lo Sucessfull  bit=%d, numBits=%d sizeof=%d\n",bit,numBits,sizeof(struct a32ZedDpvt));

      break;
    default :
        plo->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal OUT field ->%s<- \n", plo->name);
        status = ERROR;
    }
    return(status);
}

/**************************************************************************/
//static long initWfRecord( struct waveformRecord* pRec )
static long initWfRecord( pwf )
struct waveformRecord   *pwf;
{
    long                status = 0;
    unsigned long       rawVal = 0;
    int                 card, args;
    int			pixel;
    a32ZedDpvt         *pPvt;

    if (devA32ZedDebug >= 20)
    {
         epicsPrintf("init_waveform: card %d, signal %d, pixel %s, number elements %lu\n", 
	   pwf->inp.value.vmeio.card, pwf->inp.value.vmeio.signal, pwf->inp.value.vmeio.parm, pwf->nelm );
    }



    /* waveform.inp must be a VME_IO */
    switch (pwf->inp.type) {
    case (VME_IO) :
      //pvmeio = (struct vmeio*)&(pwf->inp.value);

      if(pwf->inp.value.vmeio.card > MAX_NUM_CARDS) {
        pwf->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d exceeds max: ->%s<- \n", 
        	pwf->inp.value.vmeio.card , pwf->name);
        return(ERROR);
      }

      card = pwf->inp.value.vmeio.card;

      if(cards[card].base == NULL) {
        pwf->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Card #%d not initialized: ->%s<-\n",
                     card, pwf->name);
        return(ERROR);
      }

      if (pwf->inp.value.vmeio.signal >= cards[card].nReg) {
        pwf->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Signal # exceeds registers: ->%s<-\n",
                     pwf->name);
        return(ERROR);
      }

      // args = sscanf(pwf->inp.value.vmeio.parm, "%d,%d", &bit, &numBits);
      args = sscanf(pwf->inp.value.vmeio.parm, "%d", &pixel);

      // change args from 2 to 1
      if( (args != 1) || (pixel >= MAX_PIXEL) ) 
      {
        pwf->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Invalid Num Pixels in parm field: ->%s<-\n",
                     pwf->name);
        return(ERROR);
      }

      pwf->dpvt = (void *)calloc(1, sizeof(struct a32ZedDpvt));
      pPvt = (a32ZedDpvt *)pwf->dpvt;

      pPvt->reg =  pwf->inp.value.vmeio.signal;
      pPvt->pixel = pixel;


      epicsPrintf("devA32Zed: waveform config success\n");


      /* if .dol is NOT a constant, initialize .val field to readback value */
      //if ((pwf->dol.type == CONSTANT) && 
      //    (strlen(pwf->dol.value.constantStr) == 0)) {
      //    if (read_card(card,pPvt->reg,pPvt->mask,&rawVal) == OK) {
      //        pwf->val = rawVal>>pPvt->lbit;
      //        pwf->udf = 0;
      //    }
      //}

      status = OK;

      break;
    default :
        pwf->pact = 1;          /* make sure we don't process this thing */
        epicsPrintf("devA32Zed: Illegal OUT field ->%s<- \n", pwf->name);
        status = ERROR;
    }
    return(status);
} /* initWfRecord() */



/**************************************************************************
 *
 * Perform a write operation from a BO record
 *
 **************************************************************************/
static long write_bo(pbo)
struct boRecord *pbo;
{

  unsigned long 	rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pbo->dpvt;

  if (write_card(pbo->out.value.vmeio.card, pPvt->reg, pbo->mask, pbo->rval) 
        == OK)
  {
    if(read_card(pbo->out.value.vmeio.card, pPvt->reg, pbo->mask, &rawVal) 
        == OK)
    {
      pbo->rbv = rawVal;
      return(0);
    }
  }

  /* Set an alarm for the record */
  recGblSetSevr(pbo, WRITE_ALARM, INVALID_ALARM);
  return(2);
}

/**************************************************************************
 *
 * Perform a read operation from a BI record
 *
 **************************************************************************/
static long read_bi(pbi)
struct biRecord *pbi;
{

  unsigned long       rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pbi->dpvt;

  if (read_card(pbi->inp.value.vmeio.card, pPvt->reg, pbi->mask,&rawVal) == OK)
  {
     pbi->rval = rawVal;
     return(0);
  }

  /* Set an alarm for the record */
  recGblSetSevr(pbi, READ_ALARM, INVALID_ALARM);
  return(2);
}


/**************************************************************************
 *
 * Perform a read operation from a MBBI record
 *
 **************************************************************************/
static long read_mbbi(pmbbi)
struct mbbiRecord *pmbbi;
{

  unsigned long       rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pmbbi->dpvt;

  if (read_card(pmbbi->inp.value.vmeio.card,pPvt->reg,pmbbi->mask,&rawVal) 
        == OK)
  {
     pmbbi->rval = rawVal;
     return(0);
  }

  /* Set an alarm for the record */
  recGblSetSevr(pmbbi, READ_ALARM, INVALID_ALARM);
  return(2);
}



/**************************************************************************
 *
 * Perform a write operation from a MBBO record
 *
 **************************************************************************/
static long write_mbbo(pmbbo)
struct mbboRecord *pmbbo;
{

  unsigned long         rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pmbbo->dpvt;

  if (write_card(pmbbo->out.value.vmeio.card,pPvt->reg,
                      pmbbo->mask,pmbbo->rval) == OK)
  {
    if(read_card(pmbbo->out.value.vmeio.card,pPvt->reg,pmbbo->mask,&rawVal) 
       == OK)
    {
      pmbbo->rbv = rawVal;
      return(0);
    }
  }

  /* Set an alarm for the record */
  recGblSetSevr(pmbbo, WRITE_ALARM, INVALID_ALARM);
  return(2);
}


/**************************************************************************
 *
 * Perform a read operation from a AI record
 *
 **************************************************************************/
static long read_ai(pai)
struct aiRecord *pai;
{
  unsigned long         rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pai->dpvt;

  if (read_card(pai->inp.value.vmeio.card,pPvt->reg,pPvt->mask,&rawVal) == OK)
  {
     pai->rval = rawVal>>pPvt->lbit;

/* here is where we do the sign extensions for Bipolar....    */     
        if (pPvt->type ==1) {
           if (pai->rval & (2<<(pPvt->nbit-2))) 
               pai->rval |= ((2<<31) - (2<<(pPvt->nbit-2)))*2; 

	}

     return(0);
  }

  /* Set an alarm for the record */
  recGblSetSevr(pai, READ_ALARM, INVALID_ALARM);
  return(2);

}

/**************************************************************************
 *
 * Perform a write operation from a AO record
 *
 **************************************************************************/
static long write_ao(pao)
struct aoRecord *pao;
{

  unsigned long      rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pao->dpvt;

  if (write_card(pao->out.value.vmeio.card,pPvt->reg,
                 pPvt->mask,pao->rval<<pPvt->lbit) == OK)
  {
    if(read_card(pao->out.value.vmeio.card,pPvt->reg,pPvt->mask,&rawVal)
       == OK)
    {
      pao->rbv = rawVal>>pPvt->lbit;

/* here is where we do the sign extensions for Bipolar.... */        
        if (pPvt->type ==1) {
           if (pao->rbv & (2<<(pPvt->nbit-2)))
               pao->rbv |= ((2<<31) - (2<<(pPvt->nbit-2)))*2;

	}
      
      return(0);
    }
  }

  /* Set an alarm for the record */
  recGblSetSevr(pao, WRITE_ALARM, INVALID_ALARM);
  return(2);
}

/**************************************************************************
 *
 * Perform a read operation from a LI record
 *
 **************************************************************************/
static long read_li(pli)
struct longinRecord *pli;
{

  unsigned long         rawVal = 0;
  a32ZedDpvt           *pPvt = (a32ZedDpvt *)pli->dpvt;

  if (read_card(pli->inp.value.vmeio.card,pPvt->reg,pPvt->mask,&rawVal) == OK)
  {
     pli->val = rawVal>>pPvt->lbit;
     pli->udf = 0;
     return(0);
  }

  /* Set an alarm for the record */
  recGblSetSevr(pli, READ_ALARM, INVALID_ALARM);
  return(2);

}

/**************************************************************************
 *
 * Perform a write operation from a LO record
 *
 **************************************************************************/
static long write_lo(plo)
struct longoutRecord *plo;
{

  a32ZedDpvt           *pPvt = (a32ZedDpvt *)plo->dpvt;

  if (write_card(plo->out.value.vmeio.card,pPvt->reg,
                 pPvt->mask,plo->val<<pPvt->lbit) == OK)
  {
      return(0);
  }

  /* Set an alarm for the record */
  recGblSetSevr(plo, WRITE_ALARM, INVALID_ALARM);
  return(2);
}

/**************************************************************************
 *
 * Raw read a bitfield from the card
 *
 **************************************************************************/
static long read_card(card, reg, mask, value)
short           card;  
unsigned short  reg;
unsigned long   mask;   /* created in init_bo() */
unsigned long  *value; /* the value to return from the card */
{
  if (checkCard(card) == ERROR)
    return(ERROR);

  *value = cards[card].base->reg[reg] & mask;

  if (devA32ZedDebug >= 20)
    epicsPrintf("devA32Zed: read 0x%4.4lX from card %d\n", *value, card);

  return(OK);
}



/**************************************************************************
 *
 * Write a bitfield to the card retaining the states of the other bits
 *
 **************************************************************************/
static long write_card(card, reg, mask, value)
short           card;
unsigned short  reg;
unsigned long   mask;
unsigned long   value;
{
  if (checkCard(card) == ERROR)
    return(ERROR);

  epicsMutexMustLock(cards[card].lock);
  cards[card].base->reg[reg] = ((cards[card].base->reg[reg] & ~mask) | 
                              (value & mask));
  epicsMutexUnlock(cards[card].lock);

  if (devA32ZedDebug >= 15)
    epicsPrintf("devA32Zed: wrote 0x%4.4lX to card %d at reg %d, input value = %lu mask = %lu \n",
            cards[card].base->reg[reg], card, reg, value, mask);

  return(0);
}

/*************************************************************************/
static long readWf( pwf )
struct waveformRecord   *pwf;
{
  //int	pixelToScan;
  int   signal;

  int ii;
  //int	args;
  //unsigned long * wf_buf;

  //int            num;
  //struct vmeio*  pvmeio = (struct vmeio*)&(pRec->inp.value);
  if (checkCard(pwf->inp.value.vmeio.card) == ERROR) return(ERROR);

   signal =  pwf->inp.value.vmeio.signal;
   if( signal == 0 )
   {
      epicsPrintf("Got to readWf signal = 0.  Scan pixel = %d (lo with signal = 10), dacTrim = %d (lo with signal = 11) \n", pixelToScanLO, dacTrim);
      scanPixelN( pixelToScanLO, dacTrim, configPixelCnt, pwf);
      //drvListBuckets( pvmeio->card, pRec->bptr, pRec->nelm, &num );
   }
   else if( signal == 1 )
   {
      epicsPrintf("Got to readWf signal = 1, writes wf for threshold scan to be used for x-axis.\n");
      
     // scan through thresholds
     float thnStart = 1.42;
     float thnStop  = 1.52;
     float thnStep  = 0.0002;
     float thn = 0.0;
     float * wf_buf;
     wf_buf = pwf->bptr;
     for (ii=0; ii<pwf->nelm; ii++)
     {
	thn = thnStart + (ii * thnStep);
	wf_buf[ii] = thn;
     }
      //drvShowPattern( pvmeio->card, &buf, pRec->nelm, &num );
   }

   pwf->nord = pwf->nelm;		// Device support must set this value when it completes, will update waveform

   return( 0 );

} /* readWf() */

/**************************************************************************
* commands used by vipic 
* used to scan through thresholds
*
/**************************************************************************/
void CnfgPixel(int value, volatile unsigned int *fpgabase)
{
   int i, pixclk, pixdata, pixreg;


   pixclk = 0;
   pixdata = 0;
   //printf("pixword = %x\n",value);

   for (i=21;i>=0;i--) {
       //printf("i=%d\n",i);
       pixdata = (value >> i) & 0x1;
       pixreg = (pixdata << 1) | pixclk;
       //printf("   pixdata=%x  pixclk=%d  reg=%d\n",pixdata,pixclk,pixreg);
       fpgabase[8] = pixreg;
       pixclk = 1;
       pixreg = (pixdata << 1) | pixclk;
       //printf("   pixdata=%x  pixclk=%d  reg=%d\n",pixdata,pixclk,pixreg);
       fpgabase[8] = pixreg;
       pixclk = 0;
       pixreg = (pixdata << 1) | pixclk;
       //printf("   pixdata=%x  pixclk=%d  reg=%d\n",pixdata,pixclk,pixreg);
       fpgabase[8] = pixreg;
    //usleep(10);
  }
  fpgabase[8] = 0; 
}

void WriteControlReg(int dev)
{
    char buf[3] = {0};
    int bytesWritten;

    buf[0] = 0x0C;  //Command Access Byte
    buf[1] = 0x04;
    buf[2] = 0x00;
    if (bytesWritten=write(dev,buf,3) != 3)
       printf("Error Writing DAC Control Reg...   Bytes Written: %d\n",bytesWritten);
    //else 
    //   printf("Wrote Control Register...\n");
}

void SetDacVoltage(int dev, int channel, float voltage)
{
    char buf[3] = {0};
    int bytesWritten;
    short int dacWord; 
    
    voltage = voltage; // - 0.024;  //temp kludge to remove offset
    dacWord = (int)(voltage / DACTF);
    if (dacWord > 16383)  dacWord = 16383; 
    if (dacWord < 0)     dacWord = 0;
    //printf("Set Voltage: %f   V\n",voltage);
    //printf("DAC TF: %f\n",DACTF);
    //printf("DAC Word: %d   (0x%x)\n",dacWord,dacWord);
  
    buf[0] = 0x0 + channel; //Command Access Byte
    buf[1] = (char)((dacWord | 0xC000) >> 8);
    buf[2] = (char)(dacWord & 0x00FF) ;
    //printf("MSB: %x    LSB: %x\n",((dacBits & 0xFF00) >> 8),(dacBits & 0x00FF));
    //printf("MSB: %x    LSB: %x\n",buf[1],buf[2]);
    bytesWritten = write(dev,buf,3);
    //printf("DAC Written...  Bytes Written : %d\n",bytesWritten);
}

void TrigReadFrame(volatile unsigned int *fpgabase, int frame2D[][32], int winTime)
{
   int i,j;
   int fifoWordCnt;
   int winStrTime;
   int curFrameNum;
   int frameData[1000];
 

   curFrameNum = fpgabase[FPGAVER];
   //printf("Current Frame Number : %d\n",fpgabase[FRAMENUM]);

   //Check if FIFO is empty, if not reset it
   fifoWordCnt = fpgabase[FIFOWDCNT];
   if (fifoWordCnt != 0) {
      printf("FIFO not empty.  Resetting...\n");
      fpgabase[FIFORST] = 1;
      fpgabase[FIFORST] = 0;
      usleep(1);
      fifoWordCnt = fpgabase[FIFOWDCNT];
    }
    if (fifoWordCnt != 0)  {
       printf("Unable to clear FIFO.  Exiting...\n");
       exit(1);
    }
    else
       //printf("FIFO is ready...\n");

    //Enable the counter by setting WinPad to '1' 
    //printf("Strobing WinPad for %d us\n",winTime);
    fpgabase[WINTIMEPAD] = 0x2;  //Set WinPad  
    //for (j=0; j<40*winTime; j++);  //about 1us delay
    for (j=0; j<800*winTime; j++);  //about 20us delay. If too long counters may overflow
    fpgabase[WINTIMEPAD] = 0x0;  // Clear winPad

    //printf("Triggering Frame...\n");
    //soft trigger a frame
    fpgabase[FRAMETRIG] = 1;
    fpgabase[FRAMETRIG] = 0;

    //Frame Number will increment when Frame is in FIFO
    while (curFrameNum == fpgabase[FRAMENUM]);    
    usleep(1); 
    //printf("Got Frame...   FrameNum = %d\t fifoWdCnt = %d\n",
	  //fpgabase[FRAMENUM],fpgabase[FIFOWDCNT]);

    //read the frame from the fifo
    //reading 33 rows, 16 lwords each row
    for (i=0;i<528;i++) {
        frameData[i] = fpgabase[FIFODATA];
        //row =  (frameData[i] & 0x3F000000) >> 24;       
        //idx =  (frameData[i] & 0x00F00000) >> 20; 
        //cnt0 = frameData[i] & 0x3FF;
        //cnt1 = (frameData[i] & 0xFFC00) >> 10;
        //printf("i: %d\t\trow: %d\t\tidx: %d\t\tCnt0: %d\t\tCnt1: %d\t\tframeData: %x\n",i,row,idx,cnt0,cnt1,frameData[i]);
    } 

    //initialize 2D array to a known number - debug
    for (i=0;i<33;i++)		// i = row
        for (j=0;j<32;j++)	// j = column
           frame2D[i][j] = 100;
    

    //parse the data  i = row=33, j = column=32
    for (i=0;i<33;i++) {	// i = row 33
        //printf("%d\t",i);
        for (j=0;j<16;j++) {	// j = colunm 32   0 to 15 = 16 * 2 = 32
            frame2D[i][2*j] = frameData[(i*16)+j] & 0x3FF;
            //printf("%4d ",frame2D[i][2*j]);
            frame2D[i][(2*j)+1] = (frameData[(i*16)+j] & 0xFFC00) >> 10;
            //printf("%4d ",frame2D[i][2*j+1]); 
        }
        //printf("\n");
    } 
    //printf("Counts = %d\n",frame2D[12][0]);
}

int scanPixelN(pixelToScan, dacTrimSet, ConfigPixelCnt, pwf )
int pixelToScan;
int dacTrimSet;
int ConfigPixelCnt;
struct waveformRecord   *pwf;
{
    int dev;
    char filename[40], buf[10];
    int addr = 0b01010100;        // The I2C address of the ADC
    unsigned int fpgaAddr, fpgaData;
    volatile unsigned int *fpgabase;
    int i, j, fd, dly, ii;
    int pixclk, pixdata, pixreg; 
    //int pixelToScan;
    int frame2D[33][32];
    float thn;
    //FILE *fp;
    int pixelNum, winTime, trimDacVal, trimDacValFixed;
    //int rownum, colnum;
    float thnStart, thnStop, thnStep, thp;
    int row, col;

    unsigned long * wf_buf;
    int wf_buf_ptr = 0;
    wf_buf = pwf->bptr;
    // clear waveform
    for(ii = 0; ii < pwf->nelm; ii++)
    {
    	wf_buf[ii] = 0;    
    }

    // Open i2c device (DAC)
    sprintf(filename,"/dev/i2c-0");
    if ((dev = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the i2c bus.");
        exit(1);
    }
    // Check if i2c device is there (DAC)
    if (ioctl(dev,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    //int pixelword = 0x1400;  //non-counting	
    //int pixelword = 0x181400;  //counting

    // from scott - Did not seem to work?
    //int pixelWordNoncnting = 0x0010C0;	// non-counting
    //int pixelWordCnting    = 0x1830C0; 	// counting with trim dac = 0

    // from scott after I flip ms and ls - Did not seem to work?
    //int pixelWordNoncnting = 0x00C200;	// non-counting
    //int pixelWordCnting    = 0x00C306; 	// counting with trim dac = 0

    // my guess which seems to work??
    // flipped = Direct COMP0, Direct DISCR, CalEn2b, CalEn1b, trim dac = 0110000
    int pixelWordNoncnting = 0x1830C0;	// non-counting
    // meaning of bits if MS and LS are not flipped - DAC = 1000000, CalEn0b, Analog Disable
    // meaning of bits if MS and LS are flipped - Direct COMP0, Direct DISCR, CalEn2b, DAC = 0000000 
    // int pixelWordCnting    = 0x0010C0; 	// counting with trim dac = 0
    //int pixelWordCnting = 0x0010C0 | dacTrimSet;
    int pixelWordCnting = configPixelCnt | dacTrimSet;
    printf("Threshold scan of pixel %d and dacTrimSet = 0x%x, pixelWordCnting = 0x%x\n",pixelToScan, dacTrimSet, pixelWordCnting );

    //volatile unsigned int *pixcnfgptr;
  
    /* Open /dev/mem for writing to FPGA register */
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)  {
      printf("Can't open /dev/mem\n");
      return 1;
   }
   //printf("Opened /dev/mem\r\n");

   fpgabase = (unsigned int *) mmap(0,255,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x43C00000);

   //pixcnfgptr = &fpgabase[8];

   if (fpgabase == NULL) {
      printf("Can't mmap\n");
      return 1;
   }
 
   printf("FPGA Version : %d\n",fpgabase[3]);
   pixclk = 0;
   pixdata = 0;
   
   // enable one pixel
   for (i=0;i<1024;i++) {
	   if (i == pixelToScan)
	   {
              CnfgPixel(pixelWordCnting,fpgabase);
	   }
           else
              CnfgPixel(pixelWordNoncnting,fpgabase);
	   for (dly=0;dly<100;dly++);
   }

  //printf("done configuring\n");

  for (j=0;j<100;j++); 
  //Pulse loadShadowReg
  fpgabase[8] = 0x8;
  for (j=0;j<100;j++);
  fpgabase[8] = 0x0;

  //printf("done loadShadowReg\n");

  // scan through thresholds
  thnStart = 1.42;
  thnStop  = 1.52;
  thnStep  = 0.0002;
  thp	   = 1.45;
  //fprintf(fp,"%f,%f,%f\n",thnStart,thnStop,thnStep);
  printf("Threshold scan from = %f to = %f step = %f of Pixel %d\n",thnStart,thnStop,thnStep, pixelToScan);

  // reset counters when they are read	
  fpgabase[52/4] = 1; 

  // global reset
  //issue a global reset to the miniVIPIC
  fpgabase[GLOBRST] = 1;
  usleep(1);
  fpgabase[GLOBRST] = 0;
  

  // set ThresholdP - Write Control Reg first
  WriteControlReg(dev);
  usleep(10);
  SetDacVoltage(dev,8,thp);
  usleep(10);

  for (thn=thnStart;thn<=thnStop;thn=thn+thnStep) {
     //printf("Setting thN = %5.2f\n",thn);
     // Write DAC Voltage (Channel9 is ThN)

     SetDacVoltage(dev,9,thn);
     //printf("thn = %.4f \n",thn);

     usleep(5000);
     //Toggle and then read the frame   
     winTime = 1000;  //time (us) to assert WinPad (active count time) 
     TrigReadFrame(fpgabase,frame2D,winTime);

     //write results
     for (col=0; col<32; col++) {	// 32 columns 0 to 31
        for (row=1; row<33; row++) {	// skip first row of 57, 32 rows 1->32 
	    if (frame2D[row][col] > 1)
		{
		    printf ("pixel = %d, col = %d, row = %d, threshold = %.6f, wf_buf_ptr=%d, counts = %d\n",
                             pixelToScan, col, row, thn, wf_buf_ptr, frame2D[row][col]);
		    if (wf_buf_ptr < pwf->nelm)	// only write if scan fits into waveform
		    {
			wf_buf[wf_buf_ptr] = (unsigned long) frame2D[row][col];
	        	
                    }
		}
	}
     }
     wf_buf_ptr += 1;	// inc each step of threshold
     //fprintf(fp,"%d,",frame2D[rownum][colnum]);
     
  }
  //fprintf(fp,"\n");
  //printf("Scan Done for pixel %d\n",pixelToScan);
  printf ("number of elements in wf_buf = %d\n",pwf->nelm);

  return 0; 

}


/**************************************************************************
 *
 * Make sure card number is valid
 *
 **************************************************************************/
static long checkCard(card)
short   card;
{
  if ((card >= MAX_NUM_CARDS) || (cards[card].base == NULL))
    return(ERROR);
  else
    return(OK);
}



/*******************************/
/* Information needed by iocsh */
// zed Config arguments
static const iocshArg zedconfigArg0 = {"Card being configured", iocshArgInt};
static const iocshArg zedconfigArg1 = {"zed fpga base address", iocshArgInt};
static const iocshArg zedconfigArg2 = {"number registers", iocshArgInt};
static const iocshArg zedconfigArg3 = {"iVector", iocshArgInt};
static const iocshArg zedconfigArg4 = {"iLevel", iocshArgInt};
static const iocshArg * const zedconfigArgs[5] = 
{
    &zedconfigArg0, 
    &zedconfigArg1,
    &zedconfigArg2,
    &zedconfigArg3,
    &zedconfigArg4
};

static const iocshFuncDef configZedFuncDef = {"devA32ZedConfig", 5, zedconfigArgs};
static const iocshFuncDef reportZedFuncDef = {"devA32ZedReport", 0, NULL};

// Wrappers with args
static void configZedCallFunc(const iocshArgBuf *args) 
{
    devA32ZedConfig(args[0].ival, args[1].ival, args[2].ival, args[3].ival, args[4].ival);
}
static void reportZedCallFunc(const iocshArgBuf *args)
{
    devA32ZedReport();
}

/* Registration routine, runs at startup */
static void ZedRegister(void) 
{
    iocshRegister(&configZedFuncDef, configZedCallFunc);
    iocshRegister(&reportZedFuncDef, reportZedCallFunc);
    // more func?
}
epicsExportRegistrar(ZedRegister);


