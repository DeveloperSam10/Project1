//===== Hercules Plugin ======================================
//= *dispbottomcolor Script Command
//===== By: ==================================================
//= AnnieRuru
//===== Current Version: =====================================
//= 1.1
//===== Compatible With: ===================================== 
//= Hercules 2014-09-02
//===== Description: =========================================
//= just dispbottom with colors ... what else ?
//===== Topic ================================================
//= http://hercules.ws/board/topic/7023-dispbottomcolor/
//===== Additional Comments: =================================  
//= dispbottomcolor <message> , <color code>;
//============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../map/pc.h"
#include "../map/script.h"
#include "../common/HPMi.h"
#include "../common/socket.h"
#include "../common/strlib.h"

#include "../common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {
	"dispbottomcolor",	// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",		    // Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

BUILDIN(dispbottomcolor) {
	TBL_PC *sd = script->rid2sd(st);
	if ( sd ) {
		const char *message = script_getstr(st,2);
		unsigned short msg_len = strlen( message ) +1;
		int color = script_getnum(st,3);
		int colorcode = (color & 0x0000FF) << 16 | (color & 0x00FF00) | (color & 0xFF0000) >> 16;
		WFIFOHEAD( sd->fd, msg_len + 12 );
		WFIFOW( sd->fd, 0 ) = 0x2C1;
		WFIFOW( sd->fd, 2 ) = msg_len + 12;
		WFIFOL( sd->fd, 4 ) = 0;
		WFIFOL( sd->fd, 8 ) = colorcode;
		safestrncpy( (char*)WFIFOP( sd->fd,12 ), message, msg_len );
		WFIFOSET( sd->fd, msg_len + 12 );
	}
	return true;
}

HPExport void plugin_init (void) {
	script = GET_SYMBOL("script");
	session = GET_SYMBOL("session");
	strlib = GET_SYMBOL("strlib");
	sockt = GET_SYMBOL("sockt");
	addScriptCommand( "dispbottomcolor", "si", dispbottomcolor );
}
