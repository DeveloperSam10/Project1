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

#include <stdio.h>
#include <string.h>
#include "../common/HPMi.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../map/atcommand.h"
#include "../map/clif.h"
#include "../map/itemdb.h"
#include "../map/npc.h"
#include "../map/pc.h"
#include "../map/script.h"
#include "../map/storage.h"
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

static char custom_atcmd_output[CHAT_SIZE_MAX];

#define RESTOCKITEM_SIZE 10

struct restock_data_struct {
	unsigned short restockid[RESTOCKITEM_SIZE]; // [Samuel]
	unsigned int restocking : 1; //performance-saver, restocking state for @restock
};

struct restock_data_struct *get_restock_variable(struct map_session_data *sd) {
	struct restock_data_struct *data;
	
	// Let's check if we already inserted the field
	if ( !(data = getFromMSD(sd, HPMi->pid, 0)) ) {
		
		CREATE(data,struct restock_data_struct, 1);
		
		addToMSD(sd, data, HPMi->pid, 0, true);
	}

	return data;
}

bool pc_isrestocking(struct map_session_data *sd, int nameid)
{
	int i = 0;
	struct restock_data_struct *my = get_restock_variable(sd);

	if (!my->restocking)
		return false;

	ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] == nameid);

	return (i != RESTOCKITEM_SIZE);
}

/*==========================================
 * @restock
 *------------------------------------------*/
ACMD(restock)
{
	struct item_data *item_data = NULL;
	int i;
	int action = 3; // 1=add, 2=remove, 3=help+list (default), 4=reset
	struct restock_data_struct *my = get_restock_variable(sd);
	
	if (message && *message) {
		if (message[0] == '+') {
			message++;
			action = 1;
		}
		else if (message[0] == '-') {
			message++;
			action = 2;
		}
		else if (!strcmp(message,"reset"))
			action = 4;

		if (action < 3) // add or remove
		{
			if ((item_data = itemdb->exists(atoi(message))) == NULL)
				item_data = itemdb->search_name(message);
			if (!item_data) {
				// No items founds in the DB with Id or Name
				clif->message(fd, msg_txt(1189)); // Item not found.
				return false;
			}
		}
	}
	
	switch(action) {
		case 1:
			ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] == item_data->nameid);
			if (i != RESTOCKITEM_SIZE) {
				clif->message(fd, "You're already restocking this item.");
				return false;
			}
			ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] == 0);
			if (i == RESTOCKITEM_SIZE) {
				clif->message(fd, "Your restocking list is full. Remove some items first with @restock -<item name or ID>.");
				return false;
			}
			my->restockid[i] = item_data->nameid; // Restocking Activated
			sprintf(custom_atcmd_output, "Restocking item: '%s'/'%s' {%d}", item_data->name, item_data->jname, item_data->nameid); // Autolooting item: '%s'/'%s' {%d}
			clif->message(fd, custom_atcmd_output);
			my->restocking = 1;
			break;
		case 2:
			ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] == item_data->nameid);
			if (i == RESTOCKITEM_SIZE) {
				clif->message(fd, "You're currently not restocking this item.");
				return false;
			}
			my->restockid[i] = 0;
			sprintf(custom_atcmd_output, "Removed item: '%s'/'%s' {%d} from your restock list.", item_data->name, item_data->jname, item_data->nameid); // Removed item: '%s'/'%s' {%d} from your autolootitem list.
			clif->message(fd, custom_atcmd_output);
			ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] != 0);
			if (i == RESTOCKITEM_SIZE) {
				my->restocking = 0;
			}
			break;
		case 3:
			sprintf(custom_atcmd_output, "You can have %d items on your restock list.", RESTOCKITEM_SIZE); // You can have %d items on your autolootitem list.
			clif->message(fd, custom_atcmd_output);
			clif->message(fd, "To add an item to the list, use '@restock +<item name or ID>'. To remove an item, use '@restock -<item name or ID>'."); // To add an item to the list, use "@alootid +<item name or ID>". To remove an item, use "@alootid -<item name or ID>".
			clif->message(fd, "@restock reset will clear your restock list."); // "@alootid reset" will clear your autolootitem list.
			ARR_FIND(0, RESTOCKITEM_SIZE, i, my->restockid[i] != 0);
			if (i == RESTOCKITEM_SIZE) {
				clif->message(fd, "Your restock list is empty."); // Your autolootitem list is empty.
			} else {
				clif->message(fd, "Items on your restock list:"); // Items on your autolootitem list:
				for(i = 0; i < RESTOCKITEM_SIZE; i++)
				{
					if (my->restockid[i] == 0)
						continue;
					if (!(item_data = itemdb->exists(sd->state.autolootid[i]))) {
						ShowDebug("Non-existant item %d on restock list (account_id: %d, char_id: %d)", sd->state.autolootid[i], sd->status.account_id, sd->status.char_id);
						continue;
					}
					sprintf(custom_atcmd_output, "'%s'/'%s' {%d}", item_data->name, item_data->jname, item_data->nameid);
					clif->message(fd, custom_atcmd_output);
				}
			}
			break;
		case 4:
			memset(my->restockid, 0, sizeof(my->restockid));
			clif->message(fd, "Your restock list has been reset."); // Your autolootitem list has been reset.
			my->restocking = 0;
			break;
	}
	return true;
}

/*==========================================
 * Last checks to use an item.
 * Return:
 *	0 = fail
 *	1 = success
 *------------------------------------------*/
int my_pc_useitem_pre(struct map_session_data *sd,int n) {
	int amount;
	struct restock_data_struct *my = get_restock_variable(sd);

	if (my->restocking) {
	sd->itemid = sd->status.inventory[n].nameid;
	amount = sd->status.inventory[n].amount;
	if (pc_isrestocking(sd, sd->itemid)) {
		if ( amount <=1 )  {
			if (!storage->get(sd, sd->itemid , 10))  {
				snprintf(custom_atcmd_output, sizeof(custom_atcmd_output), "Sorry, you do not have enough `%d` in storage", sd->itemid );
				clif->message(sd->fd, custom_atcmd_output);
				return 0;
				hookStop();
			}
			else {
				storage->get(sd, sd->itemid, 10);
				return 1;
				hookStop();
			}
		}
	}
	else { hookStop(); }
	}
	return 1;
}

HPExport void plugin_init (void) {
	iMalloc = GET_SYMBOL("iMalloc");
	atcommand = GET_SYMBOL("atcommand");
	npc = GET_SYMBOL("npc");
	clif = GET_SYMBOL("clif");
	itemdb = GET_SYMBOL("itemdb");
	pc = GET_SYMBOL("pc");
	storage = GET_SYMBOL("storage");

	addHookPre("pc->useitem",my_pc_useitem_pre);

	addAtcommand("vip",vip);
	addAtcommand("loginrewards",loginrewards);
	addAtcommand("restock",restock);
}
