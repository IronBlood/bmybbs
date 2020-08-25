#include "ythtbbs.h"
#include "mysql_wrapper.h"

static const unsigned int TOTAL_CAPTCHA_NUMS = 500000;
static const int INTERVAL_TO_REGEN  = 60;      /* 1min */
static const int INTERVAL_TIMEOUT   = 15 * 60; /* 15 min */

static const char *CAPTCHA_FILE     = ".captcha";
static const char *CAPTCHA_LK_FILE  = ".captcha.lock";
static const char *CAPTCHA_TABLE    = "captcha";
static const char *KEY_CAPTCHA      = "captcha";
static const char *KEY_TIMESTAMP    = "timestamp";
static const char *KEY_CREATE_TIME  = "create_time";
static const char *KEY_BOOL_USED    = "used";
static const char *VAL_BOOL_USED    = "true";
static const char *VAL_BOOL_UNUSED  = "false";

static void query_captcha_by_id_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result) {
	if (mysql_stmt_num_rows(stmt) == 1)
		mysql_stmt_fetch(stmt);
}

int query_captcha_by_id(unsigned int cap_id, struct BMYCaptcha *captcha) {
	char sqlbuf[80];
	MYSQL_BIND results[2];

	if (cap_id <= 0 || cap_id > TOTAL_CAPTCHA_NUMS)
		return -1;

	snprintf(sqlbuf, sizeof(sqlbuf), "select value, time from %s where id=%d", CAPTCHA_TABLE, cap_id);
	memset(results, 0, sizeof(results));
	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = captcha->value;
	results[0].buffer_length = 6;
	results[1].buffer_type = MYSQL_TYPE_LONGLONG;
	results[1].buffer = &(captcha->timestamp);
	results[1].buffer_length = sizeof(long long);

	execute_prep_stmt(sqlbuf, NULL, results, NULL, query_captcha_by_id_callback);
	time(&(captcha->create_time));
	return 0;
}

static void store_captcha(const char *filename, struct BMYCaptcha *captcha) {
	char timestamp[32], create_time[32];

	snprintf(timestamp, sizeof(timestamp), "%lld", captcha->timestamp);
	snprintf(create_time, sizeof(create_time), "%ld", captcha->create_time);
	savestrvalue(filename, KEY_CAPTCHA, captcha->value);
	savestrvalue(filename, KEY_TIMESTAMP, timestamp);
	savestrvalue(filename, KEY_CREATE_TIME, create_time);
	savestrvalue(filename, KEY_BOOL_USED, "false");
}

static int allow_to_regenerate_captcha(const char *filename) {
	char used[10], create_time[32];
	time_t now, ct;

	readstrvalue(filename, KEY_BOOL_USED, used, sizeof(used));
	if (strcmp(used, VAL_BOOL_USED) == 0)
		return CAPTCHA_USED;

	readstrvalue(filename, KEY_CREATE_TIME, create_time, sizeof(create_time));
	ct = (time_t) atol(create_time);
	time(&now);
	if (now - ct > INTERVAL_TO_REGEN)
		return CAPTCHA_ALLOW_TO_REGEN;

	return CAPTCHA_NOT_ALLOW_TO_REGEN;
}

int gen_captcha_for_user(const char *userid, struct BMYCaptcha *captcha) {
	char captcha_filename[80], captcha_lock_filename[80];
	struct stat captcha_file_stat;
	unsigned int cap_id;
	int rc, lockfd;

	sethomefile_s(captcha_filename, sizeof(captcha_filename), userid, CAPTCHA_FILE);
	sethomefile_s(captcha_lock_filename, sizeof(captcha_lock_filename), userid, CAPTCHA_LK_FILE);

	lockfd = openlockfile(captcha_lock_filename, O_RDONLY, LOCK_EX);
	if (lockfd <= 0)
		return CAPTCHA_FILE_ERROR;

	f_stat_s(&captcha_file_stat, captcha_filename);
	if(captcha_file_stat.st_mtim.tv_sec == 0)
		goto CREATE;

	rc = allow_to_regenerate_captcha(captcha_filename);
	if (rc == CAPTCHA_NOT_ALLOW_TO_REGEN) {
		close(lockfd);
		return rc;
	}

CREATE:
	getrandomint(&cap_id);
	cap_id = 1 + (cap_id % TOTAL_CAPTCHA_NUMS);
	query_captcha_by_id(cap_id, captcha);
	store_captcha(captcha_filename, captcha);
	close(lockfd);
	return CAPTCHA_OK;
}