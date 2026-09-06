// SPDX-License-Identifier: GPL-2.0
/*
 * keepalive.c - 内核级防息屏冻结（禁止自动 suspend）
 *
 * 目标：无论 Droidspaces 容器里装什么面板/脚本，息屏后 CPU 都不会被
 *       系统自动挂起（auto-suspend），任务照常运行。
 * 机制：注册一个名为 "PowerManagerService.noSuspend" 的 wakeup source
 *       并保持 active（__pm_stay_awake），使内核永不进入 suspend；
 *       与 /sys/power/wake_lock 同名，行为一致且不可被用户态清除。
 *
 * 影响：仅关闭整机深度休眠省电；来电/闹钟/通知仍由各自中断唤醒，不受影响。
 * 编译：编入内核本体（非模块），device_initcall，随内核永久生效。
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/wakeup_source.h>

static struct wakeup_source *no_suspend_ws;

static int __init keepalive_init(void)
{
	no_suspend_ws = wakeup_source_create("PowerManagerService.noSuspend");
	if (!no_suspend_ws)
		return -ENOMEM;

	wakeup_source_add(no_suspend_ws);
	__pm_stay_awake(no_suspend_ws);

	pr_info("keepalive: no-suspend wakeup source active, auto-suspend disabled\n");
	return 0;
}
device_initcall(keepalive_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("3032252626");
MODULE_DESCRIPTION("Keep CPU awake to prevent auto-suspend on containers");
