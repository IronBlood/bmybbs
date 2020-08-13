#include "bbslib.h"

#define ONEFACEPATH "/face"

#define NFACE 3
struct wwwface {
	char *bgcolor;
	char *color;
	char *figure;
	char *stamp;
	char *logo;
};
static struct wwwface *pface;
static struct wwwface oneface = {
	NULL, NULL, NULL, NULL, NULL
};
static struct wwwface bbsface[NFACE] = {
	{"#000000", "#99ccff", "/ytht2men.jpg", NULL, NULL},
	{"white", "#99ccff", "/ythtBlkRedGry.gif", NULL, NULL},
	{"white", "gray", "/cai.jpg", "/stamp.gif", "/logo.gif"}
};

/**
 * �ж��Ƿ�Ϊ�Ϸ��� guest ����
 * ��ʽ��׺����Ϊ _[A-H]
 * @param url
 * @return 0 ���Ϸ� 1 �Ϸ�
 */
static int is_valid_guest_url(const char* url);

char *get_login_pic_link (char *picname, char *linkback);

int
checkfile(char *fn, int maxsz)
{
	char path[456];
	int sz;
	sprintf(path, HTMPATH "%s", fn);
	sz = file_size(path);
	if (sz < 100 || sz > maxsz)
		return -1;
	return 0;
}

int
loadoneface()
{
	FILE *fp;
	static char buf[256], figure[256 + 100], stamp[356], logo[356];
	char *ptr;

	fp = fopen(HTMPATH ONEFACEPATH "/config", "r");
	if (!fp)
		return -1;
	if (fgets(buf, sizeof (buf), fp) == NULL) {
		fclose(fp);
		return -1;
	}
	fclose(fp);
	ptr = buf;
	oneface.bgcolor = strsep(&ptr, " \t\r\n");
	oneface.color = strsep(&ptr, " \t\r\n");
	oneface.figure = strsep(&ptr, " \t\r\n");
	oneface.stamp = strsep(&ptr, " \t\r\n");
	oneface.logo = strsep(&ptr, " \t\r\n");
	if (!oneface.logo)
		return -2;
	if (strstr(oneface.figure, "..") ||
	    strstr(oneface.stamp, "..") || strstr(oneface.logo, ".."))
		return -3;
	sprintf(figure, ONEFACEPATH "/%s", oneface.figure);
	oneface.figure = figure;
	if (checkfile(figure, 15000))
		return -4;
	if (!strcasecmp(oneface.stamp, "NULL"))
		oneface.stamp = NULL;
	else {
		sprintf(stamp, ONEFACEPATH "/%s", oneface.stamp);
		oneface.stamp = stamp;
		if (checkfile(stamp, 4000))
			return -5;
	}
	if (!strcasecmp(oneface.logo, "NULL"))
		oneface.logo = NULL;
	else {
		sprintf(logo, ONEFACEPATH "/%s", oneface.logo);
		oneface.logo = logo;
		if (checkfile(logo, 6500))
			return -6;
	}
	return 0;
}

int
showannounce()
{
	static struct mmapfile mf = { ptr:NULL };
	if (mmapfile("0Announce/announce", &mf) < 0 || mf.size <= 10)
		return -1;
	printf("<table width=85%% border=1><tr><td>");
	fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</table>");
	return 0;
}


void loginwindow()
{
	int n = 2;
	if (!loadoneface())
		pface = &oneface;
	else
		pface = &(bbsface[n]);
	html_header(4);

	char *fourpics=get_no_more_than_four_login_pics();
	printf("<script>function openreg(){open('" SMAGIC
	     "/bbsreg', 'winREG', 'width=600,height=460,resizable=yes,scrollbars=yes');}\n"
	     "function sf(){document.l.id.focus();}\n"
	     "function st(){document.l.t.value=(new Date()).valueOf();}\n"
	     "function lg(){self.location.href='/" SMAGIC
	     "/bbslogin?id=guest&ipmask=8&t='+(new Date()).valueOf();}\n"
	     "</script>\n");
	printf("<link href=\"/images/oras.css\" rel=stylesheet type=text/css>\n");
	printf("<title>��ӭ���� "MY_BBS_NAME"</title>");
	printf("<script type=\"text/javascript\" src=\"jquery-1.6.4.min.js\"></script>");
	printf("<script type=\"text/javascript\" src=\"showloginpics.min.js\"></script>");
	printf("<link type=\"text/css\" rel=\"Stylesheet\" href=\"showloginpics.css\" />");
	printf("<body bgcolor=#efefef leftmargin=0 topmargin=0 onload='document.l.id.focus();'>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n <td rowspan=4>&nbsp;</td>\n <td width=650 height=47> </td>\n"
		"<td rowspan=4>&nbsp;</td>\n  </tr>\n  <tr> \n"
		"<td height=264 valign=top bgcolor=#FFFFFF>\n"
		"<table width=780 border=0 align=center cellpadding=0 cellspacing=0 bgcolor=#FFFFFF>\n"
	        "<tr>\n<td height=35 align=right class=0004><a href='javascript:lg();' class=linkindex>������¼</a>"
		"/ <a href='telnet://" MY_BBS_DOMAIN "' class=linkindex>telnet��¼</a> /"
		"<a href='CTerm.rar' class=linkindex>CTerm��������</a> /"
		"<a href='javascript: openreg();' class=linkindex>���û�ע��</a><font color=#FFFFFF> \n"
		"/</font></font> </td>\n"
		"</tr>\n <tr> \n"
		"<td width=770 height=400><div id=\"container\"></div></td>" /* modified by IronBlood 11.09.08 */
		//"<td width=770 height=400 bgcolor=\"#ff6600\"><a href=%s><img src=%s alt=\"bmybbs\" border=0 width=770 height=400></img></a></td>"  /* modified by linux 05.9.11 */
        "</tr>\n"
        "<tr>\n"
          "<td height=20><div id=\"nav\"></div></td>\n" /* modified by IronBlood 11.12.25 */
	"</tr>\n"
        "<tr>\n"
          "<td height=35> <table border=0 align=center cellpadding=0 cellspacing=0>\n"
              "<tr>\n"
		"<form name=l action=/" SMAGIC "/bbslogin method=post><td>\n"
                "<td>�ʺ�&nbsp&nbsp&nbsp</td>\n"
                "<td><input name=id type=text id=usrname style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
                "<td>&nbsp&nbsp&nbsp����&nbsp&nbsp&nbsp</td>\n"
                "<td><input name=pw type=password id=pwd style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
                "<td>&nbsp&nbsp&nbsp<input name=login_btn type=image id=login_btn src=\"images/index_log.gif\" width=40 height=18 border=0 onMouseOver=\"this.src='images/index_log2.gif'\" onMouseOut=\"this.src='images/index_log.gif'\"></td>\n"
	//add by liuche@BMY 20120205
		"<td>&nbsp&nbsp&nbsp<a href='/" SMAGIC "/bbsfindpass' target='_blank' class=linkindex>�һ��û��������룿</a></td>\n"
		"</form></tr></table>\n"
	//"<table border=0 align=center cellpadding=0 cellspacing=0>"
	//"<tr><td><class=1004>��֤�ͻ� IP ��Χ��<select name=ipmask>\n"
	//	"<option value=0 selected class=1004>��IP</option>\n"
	//	"<option value=1 class=1004>2 IP</option>\n"
	//	"<option value=2 class=1004>4 IP</option>\n"
	//	"<option value=3 class=1004>8 IP</option>\n"
	//	"<option value=4 class=1004>16 IP</option>\n"
	//	"<option value=5 class=1004>32 IP</option>\n"
	//	"<option value=6 class=1004>64 IP</option>\n"
	//	"<option value=7 class=1004>128 IP</option>\n"
	//	"<option value=8 class=1004>256 IP</option>\n"
	//	"</select><a href=/ipmask.html target=_blank>����ʲô?</a></font></td></tr></table></form>\n"
	"</tr>\n </td>\n"
       "</tr>\n"
      "</table></td>\n"
  "</tr>\n"
    "<tr>\n"
    "<td align=center bgcolor=#FFFFFF><img src=\"images/index_line.gif\" name=Image1 width=650 height=20 id=Image1></td>\n"
  "<tr><td align=center>��ICP�� 05001571��<br />"
  //"��BBS�����ڣ�������ͨ��ѧ�������ģ��й�������������������<br />"
  "����ά����������ͨ��ѧ��������  BBS������</td></tr>"
	"</tr>\n"
"</table>");/* modified by linux 05.9.11 */
	showannounce();
	printf("<script>showloginpics(\"%s\")</script>",fourpics);
	free(fourpics);		// �ͷ���Դ by IronBlood 2014.10.22 ��ʵ�˴����˳��ˣ����Զ��ͷŵ� :-)
	printf("</body>\n</html>");
}

/*
void
loginwindow()
{

//int n=random()%NFACE;
	int n = 2;
	if (!loadoneface())
		pface = &oneface;
	else
		pface = &(bbsface[n]);
	html_header(4);
	printf("<script language='JavaScript'> window.open('http://www.xjtu.edu.cn/xjtucert/','62','height=400,width=800,left=50,top=50,status=no,directories=no,scrollbars=no,toolbar=no,menubar=no,location=no')");
	printf("</script>");
	//printf("<script>document.location.href='main';</script>");
	printf
	    ("<STYLE type=text/css>A{COLOR: %s; text-decoration: none;}</STYLE>\n"
	     "<script>function openreg(){open('" SMAGIC
	     "/bbsreg', 'winREG', 'width=600,height=460,resizable=yes,scrollbars=yes');}\n"
	     "function sf(){document.l.id.focus();}\n"
	     "function st(){document.l.t.value=(new Date()).valueOf();}\n"
	     "function lg(){self.location.href='/" SMAGIC
	     "/bbslogin?id=guest&ipmask=8&t='+(new Date()).valueOf();}\n"
	     "</script>\n"
	     "</head>\n<BODY text=%s bgColor=%s leftmargin=1 MARGINWIDTH=1 topmargin=0>\n<br>"
	     "<CENTER>", pface->color, pface->color, pface->bgcolor);
	printf("<IMG src=%s border=0 alt='' width=85%%>\n", pface->figure);
	if (pface->stamp)
		printf
		    ("<table width=75%%><tr><td align=right><IMG src=%s border=0 alt=''></td></tr></table>",
		     pface->stamp);
//by bjgyt
	printf("<table><tr><!--td><IMG src=/ythtnet.gif border=0 alt=''></td-->"
	       "<form name=l action=/" SMAGIC "/bbslogin method=post><td>");

	if (pface->logo)

	else
		printf("<font size=+1>B M Y</font>");
//by bjgyt
	printf("<table><tr><td>�ʺ�<INPUT maxLength=12 size=8 name=id>"
	       " ����<INPUT type=password maxLength=12 size=8 name=pw>"
	       "<INPUT type=submit value=��¼>"
	       "<INPUT type=hidden name=t value=''></td>"
	       "<script>sf();st();</script>"
	       "<td><a href='javascript: openreg();'><font size=-1>���û�ע��</font></a>&nbsp;"
	       "<a href='javascript: lg();'><font size=-1>������¼</font></a></TD></tr>"
	       "<tr><td><font size=-1>��֤�ͻ� IP ��Χ��<select name=ipmask>"
	       "<option value=0 selected>��IP</option>"
	       "<option value=1>2 IP</option>"
	       "<option value=2>4 IP</option>"
	       "<option value=3>8 IP</option>"
	       "<option value=4>16 IP</option>"
	       "<option value=5>32 IP</option>"
	       "<option value=6>64 IP</option>"
	       "<option value=7>128 IP</option>"
	       "<option value=8>256 IP</option>"
	       "</select><a href=/ipmask.html target=_blank>����ʲô?</a></font></td></tr>"
	       "<tr><td></td></tr></table></form>"
	       "</TR></TABLE>\n");
	printf("<table><tr><td><br>"
	       "<a href='telnet://" MY_BBS_DOMAIN "'><i><u>Telnet��¼" MY_BBS_ID
	       "</u></i></a>"
	       " &nbsp;<a href=\"javascript:window.external.AddFavorite('http://"
	       MY_BBS_DOMAIN "/','��" MY_BBS_LOC MY_BBS_NAME "��')\">"
	       "<i><u>����վ�����ղؼ�</u></i></a>"
	       " &nbsp;<a href=\"mailto:bohan@xjtu.edu.cn\"><i><u>��ϵվ����</u></i></a>"
	       "</td></tr></table>");
	showannounce();
	printf("</CENTER></BODY></HTML>");
}
*/
void
shownologin()
{
	int n = 0;
	static struct mmapfile mf = {
		ptr:NULL
	};
	html_header(4);
	printf
	    ("<STYLE type=text/css>A{COLOR: #99ccff; text-decoration: none;}</STYLE>"
	     "</head><BODY text=#99ccff bgColor=%s leftmargin=1 MARGINWIDTH=1><br>"
	     "<CENTER>", bbsface[n].bgcolor);
	printf("<IMG src=%s border=0 alt='' width=70%%><BR>",
	       bbsface[n].figure);
	printf("<b>ͣվ֪ͨ</b><br>");
	if (!mmapfile("NOLOGIN", &mf))
		fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</CENTER></BODY></HTML>");
	return;
}

int
bbsindex_main()
{
	char str[20], redbuf[50]; //, main_page[STRLEN];
	if (nologin) {
		shownologin();
		http_quit();
		return 0;
	}
	if (!(loginok || is_valid_guest_url(getsenv("SCRIPT_URL"))) && (rframe[0] == 0)) {
		if (strcasecmp(FIRST_PAGE, getsenv("SCRIPT_URL"))) {
			html_header(3);
			redirect(FIRST_PAGE);
			http_quit();
		}
		wwwcache->home_visit++;
		loginwindow();
		http_quit();
	}
	if (!(loginok || is_valid_guest_url(getsenv("SCRIPT_URL")))) {
		sprintf(redbuf, "/" SMAGIC "/bbslogin?id=guest&t=%d", (int) now_t);
		html_header(3);
		redirect(redbuf);
		http_quit();
	}
	if (cache_header(1000000000, 86400))
		return 0;
	html_header(1);
#if 0
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Frameset//EN\"\n"
		"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"gb2312\">\n"
		"<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n"
		"<meta http-equiv=\"Content-Language\" content=\"gb2312\" />\n"
		"<meta content=\"all\" name=\"robots\" />\n"
		"<meta name=\"author\" content=\"flyinsea@bmy,Inversun@bmy, minbtaggio@bmy\" />\n"
		"<meta name=\"Copyright\" content=\"bbs.xjtu.edu.cn,��������,����ٸBBS\" />\n"
		"<meta name=\"description\" content=\"bbs.xjtu.edu.cn,��������,����ٸBBS\" />\n"
		"<meta content=\"����ٸBBS,bmy,bbs.xjtu.edu.cn,��������\" name=\"keywords\" />\n"
		"<!--<link rel=\"icon\" href=\"/images/favicon.ico\" type=\"/image/x-icon\" />\n"
		"<link rel=\"shortcut icon\" href=\"/images/favicon.ico\" type=\"/image/x-icon\" />-->\n"
		"<link href=\"/images/oras.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
#endif
	if (!isguest
	    && (readuservalue(currentuser.userid, "wwwstyle", str, sizeof (str))
		|| atoi(str) != wwwstylenum)) {
		sprintf(str, "%d", wwwstylenum);
		saveuservalue(currentuser.userid, "wwwstyle", str);
	}
	//add by mintbaggio 040411 for new www
	printf("<title>��ӭ���� %s</title>"
		"<frameset cols=135,* frameSpacing=0 frameborder=no id=fs0>\n"
		"<frame src=bbsleft?t=%ld name=f2 frameborder=no scrolling=auto>\n"
		"<frameset id=fs1 rows=0,*,20 frameSpacing=0 frameborder=no border=0>\n"
			"<frame scrolling=no name=fmsg src=bbsgetmsg>\n"
			"<frame name=f3 src=%s>\n"
			"<frame scrolling=no name=f5 src=bbsfoot>\n"
		"</frameset>\n"
		"</frameset>\n"
		"<noframes>\n"
		"<body>\n"
		"</body>\n"
		"</noframes>\n", MY_BBS_NAME, now_t, bbsred(rframe));
	http_quit();
	return 0;
}

static int is_valid_guest_url(const char* url) {
	const char* patt = "/" SMAGIC "_?/";
	int i, l;
	l = strlen(patt);

	if (strcmp("/" SMAGIC "/", url) == 0)
		return 1;

	if (strlen(url) != l)
		return 0;

	for(i=0; i<l; i++) {
		if (patt[i] == '?') {
			if(url[i] < 'A' || url[i] > 'H')
				return 0;
		} else if (patt[i] != url[i])
			return 0;
	}

	return 1;
}