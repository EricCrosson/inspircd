/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  Inspire is copyright (C) 2002-2004 ChatSpike-Dev.
 *                       E-mail:
 *                <brain@chatspike.net>
 *           	  <Craig@chatspike.net>
 *     
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include <stdio.h>
#include <string>
#include "users.h"
#include "channels.h"
#include "modules.h"

/* $ModDesc: Provides support for USERIP command */

Server *Srv;
	 
void handle_userip(char **parameters, int pcnt, userrec *user)
{
        char Return[MAXBUF],junk[MAXBUF];
        snprintf(Return,MAXBUF,"302 %s :",user->nick);
        for (int i = 0; i < pcnt; i++)
        {
                userrec *u = Find(parameters[i]);
                if (u)
                {
                        snprintf(junk,MAXBUF,"%s%s=+%s@%s ",u->nick,strchr(u->modes,'o') ? "*" : "",u->ident,u->ip);
                        strlcat(Return,junk,MAXBUF);
                }
        }
        WriteServ(user->fd,Return);
}


class ModuleUserIP : public Module
{
 public:
	ModuleUserIP()
	{
		Srv = new Server;
		Srv->AddCommand("USERIP",handle_UserIP,'o',1,"m_Userip.so");
	}
	
	virtual ~ModuleUserIP()
	{
		delete Srv;
	}
	
	virtual Version GetVersion()
	{
		return Version(1,0,0,1,VF_VENDOR);
	}
	
};

// stuff down here is the module-factory stuff. For basic modules you can ignore this.

class ModuleUserIPFactory : public ModuleFactory
{
 public:
	ModuleUserIPFactory()
	{
	}
	
	~ModuleUserIPFactory()
	{
	}
	
	virtual Module * CreateModule()
	{
		return new ModuleUserIP;
	}
	
};


extern "C" void * init_module( void )
{
	return new ModuleUserIPFactory;
}

