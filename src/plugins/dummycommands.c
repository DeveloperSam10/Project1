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
#include "../common/timer.h"
#include "../map/atcommand.h"
#include "../map/battle.h"
#include "../map/clif.h"
#include "../map/elemental.h"
#include "../map/intif.h"
#include "../map/itemdb.h"
#include "../map/mob.h"
#include "../map/npc.h"
#include "../map/pc.h"
#include "../map/pet.h"
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

struct godelay_data_struct {
	unsigned int warpgodelay_tick;
};

struct godelay_data_struct *get_godelay_variable(struct map_session_data *sd) {
	struct godelay_data_struct *data;
	
	// Let's check if we already inserted the field
	if ( !(data = getFromMSD(sd,0)) ) {
		
		CREATE(data,struct godelay_data_struct, 1);
		
		addToMSD(sd, data,0, true);
	}

	return data;
}

bool pc_authok_pre(struct map_session_data *sd, int login_id2, time_t expiration_time, int group_id, struct mmo_charstatus *st, bool changing_mapservers) {
	int64 tick = timer->gettick();
	struct godelay_data_struct *data = get_godelay_variable(sd);


	data->warpgodelay_tick = tick;
	return true;
}

int go_delay = 1;
void go_delay_setting(const char *val) {
	go_delay = atoi(val) * 1000;
	ShowDebug("Received 'go_delay:%s'\n",val);
	/* do anything with the var e.g. config_switch(val) */
}

/*==========================================
 * Invoked when a player has received damage
 *------------------------------------------*/
void my_pc_damage_pre(struct map_session_data *sd,struct block_list *src,unsigned int hp, unsigned int sp)
{
	struct godelay_data_struct *data = get_godelay_variable(sd);
	int64 tick = timer->gettick();

	data->warpgodelay_tick = tick+go_delay; //This is the timer
	return;
	hookStop();
}

int unit_skilluse_id2_pre(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel) {
	struct map_session_data *sd = NULL;
	struct godelay_data_struct *data = get_godelay_variable(sd);
	int64 tick = timer->gettick();

	if (sd) data->warpgodelay_tick= tick+go_delay ;//The delay when they use skill
	hookStop();
}

/**
 * retrieves the help string associated with a given command.
 */
static inline const char* atcommand_help_string(AtCommandInfo *info) {
	return info->help;
}

/*==========================================
 * @go [city_number or city_name] - Updated by Harbin
 * Added 5 secs delay when hit by monsters and skill use
 *------------------------------------------*/
ACMD(go) {
	struct godelay_data_struct *my = get_godelay_variable(sd);
	int64 tick = timer->gettick();
	int i;
	int town = INT_MAX; // Initialized to INT_MAX instead of -1 to avoid conflicts with those who map [-3:-1] to @memo locations.
	char map_name[MAP_NAME_LENGTH];
	
	const struct {
		char map[MAP_NAME_LENGTH];
		int x, y;
		int min_match; ///< Minimum string length to match
	} data[] = {
		{ MAP_PRONTERA,    156, 191, 3 }, //  0 = Prontera
		{ MAP_MORROC,      156,  93, 4 }, //  1 = Morroc
		{ MAP_GEFFEN,      119,  59, 3 }, //  2 = Geffen
		{ MAP_PAYON,       162, 233, 3 }, //  3 = Payon
		{ MAP_ALBERTA,     192, 147, 3 }, //  4 = Alberta
#ifdef RENEWAL
		{ MAP_IZLUDE,      128, 146, 3 }, //  5 = Izlude (Renewal)
#else
		{ MAP_IZLUDE,      128, 114, 3 }, //  5 = Izlude
#endif
		{ MAP_ALDEBARAN,   140, 131, 3 }, //  6 = Aldebaran
		{ MAP_LUTIE,       147, 134, 3 }, //  7 = Lutie
		{ MAP_COMODO,      209, 143, 3 }, //  8 = Comodo
		{ MAP_YUNO,        157,  51, 3 }, //  9 = Juno
		{ MAP_AMATSU,      198,  84, 3 }, // 10 = Amatsu
		{ MAP_GONRYUN,     160, 120, 3 }, // 11 = Kunlun
		{ MAP_UMBALA,       89, 157, 3 }, // 12 = Umbala
		{ MAP_NIFLHEIM,     21, 153, 3 }, // 13 = Niflheim
		{ MAP_LOUYANG,     217,  40, 3 }, // 14 = Luoyang
		{ MAP_NOVICE,       53, 111, 3 }, // 15 = Training Grounds
		{ MAP_JAIL,         23,  61, 3 }, // 16 = Prison
		{ MAP_JAWAII,      249, 127, 3 }, // 17 = Jawaii
		{ MAP_AYOTHAYA,    151, 117, 3 }, // 18 = Ayothaya
		{ MAP_EINBROCH,     64, 200, 5 }, // 19 = Einbroch
		{ MAP_LIGHTHALZEN, 158,  92, 3 }, // 20 = Lighthalzen
		{ MAP_EINBECH,      70,  95, 5 }, // 21 = Einbech
		{ MAP_HUGEL,        96, 145, 3 }, // 22 = Hugel
		{ MAP_RACHEL,      130, 110, 3 }, // 23 = Rachel
		{ MAP_VEINS,       216, 123, 3 }, // 24 = Veins
		{ MAP_MOSCOVIA,    223, 184, 3 }, // 25 = Moscovia
		{ MAP_MIDCAMP,     180, 240, 3 }, // 26 = Midgard Camp
		{ MAP_MANUK,       282, 138, 3 }, // 27 = Manuk
		{ MAP_SPLENDIDE,   197, 176, 3 }, // 28 = Splendide
		{ MAP_BRASILIS,    182, 239, 3 }, // 29 = Brasilis
		{ MAP_DICASTES,    198, 187, 3 }, // 30 = El Dicastes
		{ MAP_MORA,         44, 151, 4 }, // 31 = Mora
		{ MAP_DEWATA,      200, 180, 3 }, // 32 = Dewata
		{ MAP_MALANGDO,    140, 114, 5 }, // 33 = Malangdo Island
		{ MAP_MALAYA,      242, 211, 5 }, // 34 = Malaya Port
		{ MAP_ECLAGE,      110,  39, 3 }, // 35 = Eclage
	};
			
	memset(map_name, '\0', sizeof(map_name));
	memset(custom_atcmd_output, '\0', sizeof(custom_atcmd_output));
	
	if (!message || !*message || sscanf(message, "%11s", map_name) < 1) {
		// no value matched so send the list of locations
		const char* text;
		
		// attempt to find the text help string
		text = atcommand_help_string( info );
		
		clif->message(fd, msg_txt(38)); // Invalid location number, or name.
		
		if( text ) {// send the text to the client
			clif->messageln( fd, text );
		}
		
		return false;
	}

	// Numeric entry
	if (ISDIGIT(*message) || (message[0] == '-' && ISDIGIT(message[1]))) {
		town = atoi(message);
	}

	if (town < 0 || town >= ARRAYLENGTH(data)) {
		map_name[MAP_NAME_LENGTH-1] = '\0';

		// Match maps on the list
		for (i = 0; i < ARRAYLENGTH(data); i++) {
			if (strncmpi(map_name, data[i].map, data[i].min_match) == 0) {
				town = i;
				break;
			}
		}
	}

	if (town < 0 || town >= ARRAYLENGTH(data)) {
		// Alternate spellings
		if (strncmpi(map_name, "morroc", 4) == 0) { // Correct town name for 'morocc'
			town = 1;
		} else if (strncmpi(map_name, "lutie", 3) == 0) { // Correct town name for 'xmas'
			town = 7;
		} else if (strncmpi(map_name, "juno", 3) == 0) { // Correct town name for 'yuno'
			town = 9;
		} else if (strncmpi(map_name, "kunlun", 3) == 0) { // Original town name for 'gonryun'
			town = 11;
		} else if (strncmpi(map_name, "luoyang", 3) == 0) { // Original town name for 'louyang'
			town = 14;
		} else if (strncmpi(map_name, "startpoint", 3) == 0 // Easy to remember alternatives to 'new_1-1'
		        || strncmpi(map_name, "beginning", 3) == 0) {
			town = 15;
		} else if (strncmpi(map_name, "prison", 3) == 0 // Easy to remember alternatives to 'sec_pri'
		        || strncmpi(map_name, "jail", 3) == 0) {
			town = 16;
		} else if (strncmpi(map_name, "rael", 3) == 0) { // Original town name for 'rachel'
			town = 23;
		}
	}

	if(DIFF_TICK(my->warpgodelay_tick,tick)>0) {
		sprintf(custom_atcmd_output, "You need to wait %.02f seconds to use @go command", (DIFF_TICK(my->warpgodelay_tick,tick)) / (double)1000.0);
		clif->message(fd,custom_atcmd_output);
		return 0;
	}

	if (town >= 0 && town < ARRAYLENGTH(data)) {
		int16 m = map->mapname2mapid(data[town].map);
		if (m >= 0 && map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_txt(247));
			return false;
		}
		if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_txt(248));
			return false;
		}
		if (pc->setpos(sd, mapindex->name2id(data[town].map), data[town].x, data[town].y, CLR_TELEPORT) == 0) {
			clif->message(fd, msg_txt(0)); // Warped.
		} else {
			clif->message(fd, msg_txt(1)); // Map not found.
			return false;
		}
	} else {
		clif->message(fd, msg_txt(38)); // Invalid location number or name.
		return false;
	}
	
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
	pet = GET_SYMBOL("pet");
	elemental = GET_SYMBOL("elemental");
	skill = GET_SYMBOL("skill");
	timer = GET_SYMBOL("timer");

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

	addHookPre("pc->authok",pc_authok_pre);
	addHookPre("pc->damage",my_pc_damage_pre);

	addAtcommand("go",go);
}

HPExport void server_preinit (void) {
	/* makes map server listen to mysetting:value in any "battleconf" file (including imported or custom ones) */
	/* value is not limited to numbers, its passed to our plugins handler (parse_my_setting) as const char *,
	 * and thus can be manipulated at will */
	addBattleConf("go_delay",go_delay_setting);
}