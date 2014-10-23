//===== Hercules Plugin ======================================
//= Custom Dummy Commands
//===== By: ==================================================
//= Samuel
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: ===================================== 
//= Hercules
//===== Description: =========================================
//= Complation of Custom Commands for SupremusRO
//===== Additional Comments: =================================  
// 1.0.0 Release
//============================================================

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../common/HPMi.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../map/atcommand.h"
#include "../map/battle.h"
#include "../map/clif.h"
#include "../map/intif.h"
#include "../map/itemdb.h"
#include "../map/mob.h"
#include "../map/npc.h"
#include "../map/pc.h"
#include "../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

HPExport struct hplugin_info pinfo = {
	"dummycommands",		// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

/*=========================================
 * VIP System
 *-----------------------------------------*/
ACMD(vip)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"vip::OnDisplayInfo",0);
	return true;
}

/*=========================================
 * Daily Reward System
 *-----------------------------------------*/
ACMD(loginrewards)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"LOGIN::OnLoginCmnd",0);
	return true;
}

/*=========================================
 * Bank System
 *-----------------------------------------*/
ACMD(bank)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"_BankAtCmd::OnAtcommand",0);
	return true;
}

/*=========================================
 * Hunting Mission System
 *-----------------------------------------*/
ACMD(mission)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"Hunting Missions::OnAtcommand",0);
	return true;
}

/*=========================================
 * Next Level
 *-----------------------------------------*/
ACMD(nextlevel)
{
	if (!sd) return false;

	if( sd->npc_id || sd->vender_id || sd->buyer_id || sd->state.trading || sd->state.storage_flag )
		return false;

	npc->event(sd,"nextlevel::OnAtcommand",0);
	return true;
}

static char custom_atcmd_output[CHAT_SIZE_MAX];

struct Battle_Config battle_config;

/*==========================================
 *
 *------------------------------------------*/
ACMD(monster)
{
	char name[NAME_LENGTH];
	char monster[NAME_LENGTH];
	char eventname[EVENT_NAME_LENGTH] = "";
	int mob_id;
	int number = 0;
	int count;
	int i, k, range;
	short mx, my;
	unsigned int size;
	
	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(custom_atcmd_output, '\0', sizeof(custom_atcmd_output));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(80)); // Please specify a display name or monster name/id.
		return false;
	}
	if (sscanf(message, "\"%23[^\"]\" %23s %d", name, monster, &number) > 1 ||
		sscanf(message, "%23s \"%23[^\"]\" %d", monster, name, &number) > 1) {
		//All data can be left as it is.
	} else if ((count=sscanf(message, "%23s %d %23s", monster, &number, name)) > 1) {
		//Here, it is possible name was not given and we are using monster for it.
		if (count < 3) //Blank mob's name.
			name[0] = '\0';
	} else if (sscanf(message, "%23s %23s %d", name, monster, &number) > 1) {
		//All data can be left as it is.
	} else if (sscanf(message, "%23s", monster) > 0) {
		//As before, name may be already filled.
		name[0] = '\0';
	} else {
		clif->message(fd, msg_txt(80)); // Give a display name and monster name/id please.
		return false;
	}
	
	if ((mob_id = mob->db_searchname(monster)) == 0) // check name first (to avoid possible name beginning by a number)
		mob_id = mob->db_checkid(atoi(monster));
	
	if (mob_id == 0) {
		clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
		return false;
	}
		
	if (number <= 0)
		number = 1;
	
	if( !name[0] )
		strcpy(name, "--ja--");
	
	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;
	
	if (strcmpi(info->command, "monstersmall") == 0)
		size = SZ_MEDIUM;
	else if (strcmpi(info->command, "monsterbig") == 0)
		size = SZ_BIG;
	else
		size = SZ_SMALL;
	
	if (battle_config.etc_log)
		ShowInfo("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, sd->bl.x, sd->bl.y);
	
	count = 0;
	range = (int)sqrt((float)number) +2; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		map->search_freecell(&sd->bl, 0, &mx,  &my, range, range, 0);
		k = mob->once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, eventname, size, AI_NONE|(mob_id == MOBID_EMPERIUM?0x200:0x0));
		count += (k != 0) ? 1 : 0;
	}
	
	if (count != 0)
		if (number == count) {
			if( pc_get_group_level(sd) == 99 ){ // Checks if the GM level is below 99 Announcement is made [Vengeance]
				clif->message(fd, msg_txt(39)); // All monster summoned!
			}
			else {
				sprintf(custom_atcmd_output, "%s summoned %d %s in %s,%d,%d", sd->status.name,number, monster, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y);
				intif->broadcast(custom_atcmd_output, strlen(custom_atcmd_output) + 1, 0);
				clif->message(fd, msg_txt(39)); // All monster summoned!
			}
		}
		else {
			sprintf(custom_atcmd_output, "%s summoned %d %s in %s,%d,%d", sd->status.name,number, monster, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y);
			intif->broadcast(custom_atcmd_output, strlen(custom_atcmd_output) + 1, 0);
			sprintf(custom_atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif->message(fd, custom_atcmd_output);
		}
		else {
			clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
			return false;
		}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(recall) {
	struct map_session_data *pl_sd = NULL;
	
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1018)); // Please enter a player name (usage: @recall <char name/ID>).
		return false;
	}
	
	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_txt(81)); // Your GM level doesn't authorize you to preform this action on the specified player.
		return false;
	}
	
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1019)); // You are not authorized to warp someone to this map.
		return false;
	}
	if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1020)); // You are not authorized to warp this player from their map.
		return false;
	}
	if (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y) {
		return false;
	}
	/* Anti Corrupt GM */
	if ( pc_get_group_level(sd) < 99 && map->list[sd->bl.m].flag.gvg_castle )
	{
		clif->message(fd, "Sorry, you are not allowed to recall in WoE Castles");
		return false;
	}
	pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
	sprintf(custom_atcmd_output, msg_txt(46), pl_sd->status.name); // %s recalled!
	clif->message(fd, custom_atcmd_output);
	
	return true;
}

HPExport void plugin_init (void) {
	iMalloc = GET_SYMBOL("iMalloc");
	atcommand = GET_SYMBOL("atcommand");
	npc = GET_SYMBOL("npc");
	clif = GET_SYMBOL("clif");
	itemdb = GET_SYMBOL("itemdb");
	intif = GET_SYMBOL("intif");
	map = GET_SYMBOL("map");
	mapindex = GET_SYMBOL("mapindex");
	mob = GET_SYMBOL("mob");
	pc = GET_SYMBOL("pc");

	addAtcommand("vip",vip);
	addAtcommand("loginrewards",loginrewards);

	/* Bank System */
	addAtcommand("bank",bank);
	addAtcommand("bankadmin",bank);

	addAtcommand("mission",mission);
	addAtcommand("nextlevel",nextlevel);

	addAtcommand("monster",monster);
	addAtcommand("monstersmall",monster);
	addAtcommand("monsterbig",monster);

	addAtcommand("recall",recall);
}
