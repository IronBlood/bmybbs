/* timeop.h */
#ifndef __TIMEOP_H
#define __TIMEOP_H
char *Ctime(time_t);

/** 比较当前时间和目标时间的差异，并返回适当的字符串。
 * 例如 5秒钟前，10分钟后
 * @param compared_time 需要对比的时间。
 * @return 更容易理解的文字
 * @warning 该方法不是线程安全的！并且会强制转换为 int 类型。
 */
char *Difftime(time_t compared_time);
#endif
