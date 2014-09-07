// VisaExec.cpp
//

/*
VisaExec <VISA_ADDRESS or ALIAS> [SCPI_CMD] [WAIT ms] [LOG_FILE]

VisaId: Visa Address or alias name of instrument.
SCPI_COMMAND: Command to send to instrument, use " " to enclose command string. Default "*IDN?"
WAIT ms: milliseconds to wait after command completion, default 100 ms. (for chaining commands in batch file)
LogFile: LogFile name, default: no log, console only. If file don't exist it is created, if exists data is appended.


*/


#include "stdafx.h"
#include "visa.h"
#include "string.h"

#define MAX_CNT 1024

FILE *logFile;
SYSTEMTIME lt;

int sendSCPI(ViSession instr,char *strCmd, char *buffer);

int main(int argc, char* argv[])
{
ViStatus status; /* For checking errors */
ViSession defaultRM, instr; /* Communication channels */

int  retcode,waitms;
char buffer[MAX_CNT]; /* Buffer for string I/O */
char strCmd[MAX_CNT] = "*IDN?"; // Command String, Default: Identify Instrument.

	fprintf(stderr,"\n~ VisaExec.exe ~ Copyright 2014 by Guillermo F. Molina ~\n");

	if(argc < 2 || argc > 5) {
		/* Error Wrong number of parameters*/
		fprintf(stderr,"\nERROR: Wrong number of parameters.\n");
		fprintf(stderr,"\nUSE:\n VisaExec <VISA_ADDRESS or ALIAS> [SCPI_CMD] [WAIT ms] [LOG_FILE]\n");
		return -1;
		}


	if(argc > 2)
	{
		/* Copy second parameter to command string */
		sprintf(strCmd,"%s\n", argv[2]);
	}

	if (argc > 3)
	{
		sscanf(argv[3],"%d",&waitms);
	}else // Default parameter, 100 millisecond wait.
	{	
		waitms=100;
	}

	if (argc > 4)
	{
			if((logFile=fopen(argv[4],"a+")) == NULL)
			{
				fprintf(stderr,"\nError: Can't create %s\n",argv[4]);
				return -1;
			}
	}else //Default parameter, no logfile, console only
	{
		logFile=NULL;
	}
	
	
	/* Begin by initializing the system*/
	status = viOpenDefaultRM(&defaultRM);
	if (status < VI_SUCCESS) {
		/* Error Initializing VISA...exiting*/
		fprintf(stderr,"\nERROR: viOpenDefault\n VISA Interface can't be initialized.\n");
		return -1;
		}
	
	/* Open communication */
	status = viOpen(defaultRM,argv[1], VI_NULL, VI_NULL, &instr);
	if (status < VI_SUCCESS) {
		/* Cant open Instrument...exiting*/
		fprintf(stderr,"\nERROR: viOpen\n No VISA Instrument at %s\n",argv[1]);
		return -1;
		}

	/* Set the timeout for message-based communication*/
	status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
		if (status < VI_SUCCESS) {
		fprintf(stderr,"\nERROR: viSetAttribute\n");
		return -1;
		}
		
		GetLocalTime(&lt);
		retcode=sendSCPI(instr,strCmd,buffer);
		
		if (retcode<0) 
		{
			fprintf(stderr,"\nERROR in SCPI_CMD: %s\n",strCmd);
			return -1;
		}

		if(retcode>0) // query command issued, log result
		{
			buffer[strlen(buffer)-1]=0; //strip last character, <CR>.
			if(strCmd[(strlen(strCmd)-1)]=='\n') strCmd[(strlen(strCmd)-1)]= 0;//strip <CR>
			fprintf(stderr,"\"%s\"\t%s",strCmd,buffer);
			if (logFile != NULL) fprintf(logFile,"%04d-%02d-%02d::%02d:%02d:%02d.%03d\t\"%s\"\t%s\n",lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, strCmd,buffer);
		}

		Sleep(waitms);

	/* Close down the system */
	status = viClose(instr);
		if (status < VI_SUCCESS) {
		fprintf(stderr,"\nERROR: viClose\n");
		return -1;
		}

	status = viClose(defaultRM);
		if (status < VI_SUCCESS) {
		fprintf(stderr,"\nERROR: viClose\n");
		return -1;
		}

	return 0;
}
/*
	int sendSCPI(ViSession instr,char *strCmd, char *buffer)

	Sends strCmd command string to VISA instr and leaves result (if any) at buffer
	Returns number of characters read (greater than 0 when the command is a query, 0 when it is a cmd without reply)
	Returns -1 if error
*/

int sendSCPI(ViSession instr,char *strCmd, char *buffer)
{
	ViStatus status;
	ViUInt32 retCount; /* Return count from string I/O */

	/* Send SCPI Command*/
	
	Sleep(50); // Magic! To avoid timeout errors

	status = viWrite(instr, (ViBuf) strCmd, strlen(strCmd), &retCount);
		if (status < VI_SUCCESS) 
		{
		fprintf(stderr,"\nERROR: viWrite\n");
		return(-1);
		}

	retCount=0;

	if (strchr(strCmd,'?') != NULL) 
	{ // if it is a Query command, issue the read to get result and put it in the buffer
		
	Sleep(50); // Magic! To avoid timeout errors
		
	status = viRead(instr,(ViBuf) buffer, MAX_CNT, &retCount);
		if (status < VI_SUCCESS) 
		{
		fprintf(stderr,"\nERROR: viRead\n");
		return(-1);
		}
	buffer[retCount]=0; // insert NULL at buffer end
		
	}
	return (int)retCount;
}