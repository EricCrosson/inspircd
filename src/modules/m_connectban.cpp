/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2010 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"
#include "xline.h"

/* $ModDesc: Throttles the connections of any users who try connect flood */

class ModuleConnectBan : public Module
{
 private:
	clonemap connects;
	unsigned int threshold;
	unsigned int banduration;
	unsigned int ipv4_cidr;
	unsigned int ipv6_cidr;
 public:
	void init()
	{
		Implementation eventlist[] = { I_OnUserConnect, I_OnGarbageCollect };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
	}

	virtual ~ModuleConnectBan()
	{
	}

	virtual Version GetVersion()
	{
		return Version("Throttles the connections of any users who try connect flood", VF_VENDOR);
	}

	void ReadConfig(ConfigReadStatus&)
	{
		std::string duration;

		ipv4_cidr = ServerInstance->Config->GetTag("connectban")->getInt("ipv4cidr");
		if (ipv4_cidr == 0)
			ipv4_cidr = 32;

		ipv6_cidr = ServerInstance->Config->GetTag("connectban")->getInt("ipv6cidr");
		if (ipv6_cidr == 0)
			ipv6_cidr = 128;

		threshold = ServerInstance->Config->GetTag("connectban")->getInt("threshold");

		if (threshold == 0)
			threshold = 10;

		duration = ServerInstance->Config->GetTag("connectban")->getString("duration");

		if (duration.empty())
			duration = "10m";

		banduration = ServerInstance->Duration(duration);
	}

	virtual void OnUserConnect(LocalUser *u)
	{
		int range = 32;
		clonemap::iterator i;

		switch (u->client_sa.sa.sa_family)
		{
			case AF_INET6:
				range = ipv6_cidr;
			break;
			case AF_INET:
				range = ipv4_cidr;
			break;
		}

		irc::sockets::cidr_mask mask(u->client_sa, range);
		i = connects.find(mask);

		if (i != connects.end())
		{
			i->second++;

			if (i->second >= threshold)
			{
				// Create zline for set duration.
				ZLine* zl = new ZLine(ServerInstance->Time(), banduration, ServerInstance->Config->ServerName, "Your IP range has been attempting to connect too many times in too short a duration. Wait a while, and you will be able to connect.", mask.str());
				if (ServerInstance->XLines->AddLine(zl,NULL))
					ServerInstance->XLines->ApplyLines();
				else
					delete zl;

				ServerInstance->SNO->WriteGlobalSno('x',"Module m_connectban added Z:line on *@%s to expire on %s: Connect flooding", 
					mask.str().c_str(), ServerInstance->TimeString(zl->expiry).c_str());
				ServerInstance->SNO->WriteGlobalSno('a', "Connect flooding from IP range %s (%d)", mask.str().c_str(), threshold);
				connects.erase(i);
			}
		}
		else
		{
			connects[mask] = 1;
		}
	}

	virtual void OnGarbageCollect()
	{
		ServerInstance->Logs->Log("m_connectban",DEBUG, "Clearing map.");
		connects.clear();
	}
};

MODULE_INIT(ModuleConnectBan)
