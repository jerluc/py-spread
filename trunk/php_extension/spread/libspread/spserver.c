/**
  *  filename: spserver.c
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include "sp.h"


#define MAX_MESSLEN     102400
static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;
static  int	Num_sent;

static  char	User[80] = "yanbin";
static  char    Spread_name[80] = "3333@10.55.37.105";
static  char    logfile[] = "server.log";
static  int     To_exit = 0;
static  void	Bye();

int main(int argc, char *argv[])
{
	sp_time test_timeout;
	struct timeval btime, etime, rettime;
	
	test_timeout.sec = 5;
	test_timeout.usec = 0;
	int	ret, i;
	/*
	char meg[MAX_MESSLEN] = "1197993605|addarticle|172.16.175.205|/s/indexbydate_1287294914_20071218.htm %d\n";
	*/
	char meg[MAX_MESSLEN];
	char msg[] = "1197993605|addarticle|172.16.175.205|/s/indexbydate_1287294914_20071218.htm";
	
	char	mess[MAX_MESSLEN];
	unsigned int	mess_len;
	char	groups[10][MAX_GROUP_NAME];
	int	num_groups;
	FILE *fp = NULL;

    setbuf(stdout, NULL);
	   
	ret = SP_connect_timeout( Spread_name, User, 0, 1, &Mbox, Private_group, test_timeout );	
	if( ret != ACCEPT_SESSION ) 	
	{
		SP_error( ret );
		Bye();
	}
	
	printf("User: connected to %s with private group %s\n", Spread_name, Private_group );

    sprintf(groups[0], "%s", "test");
	num_groups = 1;

	
	memset(meg, 0, sizeof(meg));

	for (i = 0; i < 30; ++i)
	{
		strcat(meg, msg);
	}

	strcat(meg, "%d\n");
	
    fp = fopen(logfile, "a+");
    gettimeofday(&btime, 0);
	for (i = 0; i < 30; ++i)
	{
                sprintf(mess, meg, i);
                mess_len = strlen(mess);
		ret= SP_multigroup_multicast( Mbox, RELIABLE_MESS, num_groups, (const char (*)[MAX_GROUP_NAME]) groups, 1, mess_len, mess );
		if( ret < 0 ) 			
		{
			SP_error( ret );
			Bye();
		}
		fwrite(mess,  mess_len + 1, 1, fp);
		 /*printf("send %s ok\n", mess); */
	}
	fclose(fp);
	gettimeofday(&etime, 0);

       rettime. tv_usec = etime.tv_usec - btime.tv_usec;
	 rettime. tv_sec = etime.tv_sec - btime.tv_sec;
	 
	if (rettime. tv_usec  < 0)
	{
	     rettime. tv_usec  += 1000000;
	     rettime. tv_sec -=1;
	}
	
	printf("used time: %d sec, %d usec\n", rettime. tv_sec, rettime. tv_usec);
       SP_disconnect( Mbox );
	return 0;
	
}

static  void Bye()
{
	To_exit = 1;	printf("\nBye.\n");
	#ifdef	_REENTRANT
		#ifdef 		__bsdi__	/* bug in threaded bsdi that cores in SP_disconnect when closing mbox while another thread is reading*/	
			exit( 0 );
		#endif		/* __bsdi__ */
	#endif	/* _REENTRANT */	
	
	SP_disconnect( Mbox );

	#ifdef	_REENTRANT
		#ifndef		ARCH_PC_WIN95		
			pthread_join( Read_pthread, NULL );
		#else		/* ARCH_PC_WIN95 */
		#endif		/* ARCH_PC_WIN95 */
	#endif	/* _REENTRANT */	
	exit( 0 );
}
