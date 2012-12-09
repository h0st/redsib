/*

  Copyright (c) 2009, Nokia Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.
    * Neither the name of Nokia nor the names of its contributors
    may be used to endorse or promote products derived from this
    software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <glib.h>

#include <whiteboard_log.h>

////BOOLEAN CONST
#define true  1
#define false 0

#define TRUE 1
#define FALSE 0

//#include <mcheck.h>


#include "dbushandler.h"
#include "sib_control.h"
#include "sib_operations.h"

#define MAJOR_VERSION 	"0"
#define MINOR_VERSION 	"3"
#define BUILD 		"12"

GMainLoop *mainloop = NULL;
SibControl *sib_control  = NULL;

void main_signal_handler(int sig)
{
  static volatile sig_atomic_t signalled = 0;

  whiteboard_log_debug_fb();

  if ( 1 == signalled )
    {
      signal(sig, SIG_DFL);
      raise(sig);
    }
  else
    {
      signalled = 1;
      sib_control_stop_all(sib_control);
      g_main_loop_quit(mainloop);
    }

  whiteboard_log_debug_fe();
}

int main(int argc, char **argv)
{

	  /*AD-ARCES*/
	  printf("/* * * * * * * * * * * * * * * * * * * * * *\\\n");
	  printf("  Redland SIB, version %s.%s.%s\n", MAJOR_VERSION, MINOR_VERSION, BUILD);
	  printf("  Developed By ARCES's Team from Nokia C-SIB\n");
	  printf("  It exports SYNCHRONIZATION mechanism\n");
	  printf("\\* * * * * * * * * * * * * * * * * * * * * */\n\n");

  //mtrace();
  gchar* ss_name;
  gchar *dbus_path=NULL;
  whiteboard_log_debug("SIB version %s.%s.%s\n", MAJOR_VERSION, MINOR_VERSION, BUILD);
  whiteboard_log_debug_fb();

  int i;

  GSList *parameter_list = NULL;

  int name_inserted= FALSE;


  if (argc > 1) {
    for (i = 2; i <= argc; i = i+1)
	{
		char* temp;
		temp=((char*)(argv[i-1]));
		if (strncmp(temp, ((char*)"--"), 2)   ==0)
		{
			//parameter
			if (strcmp(temp,"--help")!=0)
			{
				printf ("REQUESTED PARAMETER: %s\n",temp );
				parameter_list=g_slist_prepend (parameter_list, temp);
			}
			else
			{
				printf("\nUSAGE:\n\n");
				printf("./redsibd [--option1] [--option2] [--option..] [ss_name]\n");
				printf("\n\nOPTIONS AVALIABLE:\n\n");
				printf("--enable-rdf++			Enable RDF++ Reasoning Plugin\n\n");
				printf("--disable-protections		Disable protections ARCES plugin\n\n");
				printf("--storage-volatile		Disable persistent BDB and run on volatile\n");
				printf("--storage-sqlite		Use Sqlite as storage db (slow..)\n");
				printf("--storage-subs-persistent	Use persistent BDB for handling subscriptions\n"); 				        printf("                                (just in case subscriptions work with high number of statements)\n\n");

				return 0;
			}
		}
		else
		{	
			if (!name_inserted)
			{			
				//SS name
				printf ("SS name set from parameters: %s\n",argv[i-1]);
	  			ss_name = g_strdup(argv[i-1]);
				name_inserted= TRUE;
			}
		}

	}
     if (!name_inserted)
	{
		printf ("SS name: default\n");
		ss_name = g_strdup("X");
	}
	
  }
  else {
    ss_name = g_strdup("X");
  }

  DBusHandler *dbushandler = NULL;
  //SibHandler *sib_handler = NULL;


  if (!g_thread_supported ()) g_thread_init (NULL);
  g_type_init();
  dbus_g_thread_init();
  /* Set signal handlers */
  signal(SIGINT, main_signal_handler);
  signal(SIGTERM, main_signal_handler);

  /* Create new main loop */
  mainloop = g_main_loop_new(NULL, FALSE);
  g_main_loop_ref(mainloop);

  /* Initialize SIB data structures */

  whiteboard_log_debug("Initializing SIB.\n");
  sib_data_structure* sib_data = sib_initialize(ss_name, parameter_list);
  whiteboard_log_debug("Done\n");

  /* TODO: remove hardcoded values */
  /* Create a new DBus connection handler */
  whiteboard_log_debug("Creating dbus handler.\n");
  dbus_path = getenv("SIB_DBUS_PATH");
  if(dbus_path==NULL)
    {
      dbus_path=g_strdup("/tmp/dbus-sib");
    }

  dbushandler = dbushandler_new(dbus_path,
				ss_name,
				mainloop, sib_data);
  whiteboard_log_debug("Done\n");

  /* Create the node access component */
  //	whiteboard_log_debug("Creating sib access handler.\n");
  //sib_sib_handler = sib_sib_handler_new(dbushandler);
  //whiteboard_log_debug("Done\n");

  /* Create new control object and start all sinks/sources */
  whiteboard_log_debug("Creating control object and starting sibaccess/sib modules.\n");
  sib_control = sib_control_new();
  //sib_control_start_all_from(sib_control, SIB_LIBEXECDIR);
  whiteboard_log_debug("Done\n");


  /* Enter main loop and block */

  g_main_loop_run(mainloop);

  whiteboard_log_debug("Finished, cleaning up.\n");


  sib_control_stop_all(sib_control);

  sib_control_destroy(sib_control);
  //	sib_sib_handler_destroy(sib_sib_handler);
  dbushandler_destroy(dbushandler);

  whiteboard_log_debug("Normal exit.\n");

  whiteboard_log_debug_fe();

  //END TS MODEL (if DB is persistent triples will be saved)
  librdf_free_model(sib_data->RDF_model);
  librdf_free_storage(sib_data->RDF_storage);
  //////////////////////////////////////////////////////////

  printf("\n\nBye.\n\n");
  return 0;
}
