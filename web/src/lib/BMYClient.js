function myFetchGet(url) {
	return fetch(url).then(response => response.json());
}

export const BMYClient = {
	get_announce() {
		return myFetchGet("/api/article/list?type=announce");
	},
	get_article_content(boardname_en, aid) {
		return myFetchGet(`/api/article/getHTMLContent?board=${boardname_en}&aid=${aid}`);
	},
	get_article_list_by_board(boardname_en, mode) {
		return myFetchGet(`/api/article/list?type=board&board=${boardname_en}&btype=${mode}`);
	},
	get_board_info(boardname_en) {
		return myFetchGet(`/api/board/info?bname=${boardname_en}`);
	},
	get_boards_by_section(secstr, sortmode) {
		return myFetchGet(`/api/board/list?secstr=${secstr}&sortmode=${sortmode}`);
	},
	get_commend() {
		return myFetchGet("/api/article/list?type=commend");
	},
	get_fav_board_list() {
		return myFetchGet("/api/board/fav/list");
	},
	get_feed(start) {
		return myFetchGet(`/api/subscription/list?start=${start}`);
	},
	get_sectop(secstr) {
		return myFetchGet(`/api/article/list?type=sectop&secstr=${secstr}`);
	},
	get_thread_list(boardname_en, tid) {
		return myFetchGet(`/api/article/list?type=thread&board=${boardname_en}&thread=${tid}`);
	},
	get_user_info(userid) {
		return myFetchGet("/api/user/query?queryid=" + userid);
	}
};

