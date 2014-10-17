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

HPExport void plugin_init (void) {
	iMalloc = GET_SYMBOL("iMalloc");
	atcommand = GET_SYMBOL("atcommand");
	npc = GET_SYMBOL("npc");
	clif = GET_SYMBOL("clif");
	itemdb = GET_SYMBOL("itemdb");
	pc = GET_SYMBOL("pc");

	addHookPre("pc->useitem",my_pc_useitem_pre);

	addAtcommand("vip",vip);
	addAtcommand("loginrewards",loginrewards);
}
