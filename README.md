# android-lxc-docker

安卓内核 LXC/Docker 支持补丁仓库，由 [LXC-DOCKER-KernelSU_for_k20pro](https://github.com/3032252626/LXC-DOCKER-KernelSU_for_k20pro) 编译工作流引用。

fork 自 [wu17481748/android-lxc-docker](https://github.com/wu17481748/android-lxc-docker)。

> **声明**：本仓库补丁及脚本均源自上游社区，仅做引用整合，未进行自主开发。问题排查与适配由 AI 辅助完成。

---

## 一、仓库结构（当前有效状态）

```
android-lxc-docker/
├── LXC-DOCKER-OPEN-CONFIG.sh    # LXC/Docker 内核配置注入脚本
├── cgroup.patch                 # cgroup 补丁（已弃用，仅作存档）
├── xt_qtaguid.patch             # qtaguid 网络模块补丁
└── scripts-legacy/              # ego-taboo 旧版脚本备份
    ├── runcpatch.sh
    ├── add-lxc-docker-custom3.sh
    └── clangfix2.sh
```

> **注意**：原 `keepalive.patch` 与 `keepalive/keepalive.c` 已于 2026-09-07 提交 `1ac812a8` 删除。keepalive 补丁不再通过本仓库分发，改由编译工作流内嵌 heredoc 注入，详见下文「keepalive 防息屏冻结补丁说明」。

---

## 二、文件信息与用途

### LXC-DOCKER-OPEN-CONFIG.sh

| 项目 | 说明 |
|------|------|
| 类型 | Shell 脚本 |
| 用途 | 向内核 defconfig 注入 LXC/Docker 所需的 Namespaces、Cgroups、Netfilter 等配置项 |
| 来源 | 上游 wu17481748/android-lxc-docker |
| 说明 | 通用 GKI 内核谨慎开启某些配置，可能导致不开机 |

```bash
# 预览（不写入）
./LXC-DOCKER-OPEN-CONFIG.sh <xxx_defconfig>

# 写入
./LXC-DOCKER-OPEN-CONFIG.sh <xxx_defconfig> -w
```

### xt_qtaguid.patch

| 项目 | 说明 |
|------|------|
| 类型 | Git/统一 diff 补丁 |
| 用途 | 修复 `net/netfilter/xt_qtaguid.c` 中一处 rtnl lock 可能导致的内核 panic，容器网络相关 |
| 来源 | 上游社区 |
| 说明 | 有则打，无则跳过，不影响编译 |

```bash
patch -p1 -d <内核源码目录> < xt_qtaguid.patch || echo "patch not applicable"
```

### cgroup.patch（已弃用）

| 项目 | 说明 |
|------|------|
| 类型 | Git/统一 diff 补丁 |
| 用途 | 向 `kernel/cgroup/cgroup.c` 的 `cgroup_add_file()` 函数追加一段前缀跳过逻辑 |
| 状态 | **已弃用，仅作存档** |
| 说明 | 较新内核源码已内置等效逻辑，重复打补丁会导致编译冲突 |

---

## 三、keepalive 防息屏冻结补丁说明

### 背景

解决 Droidspaces 容器息屏后 CPU 被冻结、面板定时任务不触发的问题。该问题分两层：

1. 内核 suspend——由 root 写 `/sys/power/wake_lock` 持锁破除；
2. HyperOS 3 上层应用冻结——无视 wake lock，需关闭『深度睡眠模式 / 息屏断网 / 后台冻结』开关或做冻结豁免。

keepalive 方案从内核层面根治第 1 层问题，不依赖任何具体容器/面板。

### 现状（重要）

- 本仓库中的 `keepalive.patch` 与 `keepalive/keepalive.c` **已删除**（提交 `1ac812a8`，孤儿文件清理）。
- keepalive 源码当前由编译工作流 `build-droidspaces-clang14.yml` 通过 **heredoc 内嵌注入** 方式写入内核源码（含 `device.h` 前置 + `pm_wakeup.h`），读取 `config.env` 中的 `ENABLE_KEEPALIVE` 开关（默认 `true`）控制是否注入，不再引用本仓库任何 keepalive 文件。
- 若需在本地手动注入，可按原补丁内容等价实现：

```bash
cd <内核源码目录>
# 1) 在 drivers/misc/Makefile 末尾追加一行（以 okl4-vipc.o / okl4-panic.o / okl4-link-shbuf.o 三行为锚点）
echo 'obj-y += keepalive.o' >> drivers/misc/Makefile

# 2) 写入内核驱动源码（内容为原 keepalive.c）
#    注册名为 PowerManagerService.noSuspend 的 wakeup source 并保持 active（__pm_stay_awake），
#    使内核永不自动 suspend；等价于 echo PowerManagerService.noSuspend > /sys/power/wake_lock，
#    但运行在内核空间，重启不丢失、用户态无法清除，覆盖所有容器场景。
```

**实现机制**：注册名为 `PowerManagerService.noSuspend` 的 wakeup source 并保持 active（`__pm_stay_awake`），使内核永不自动 suspend。

**影响**：仅关闭整机深度休眠省电；来电/闹钟/通知仍由各自中断唤醒，不受影响。

---

## 四、旧版脚本（scripts-legacy）

备份自 [ego-taboo/LXC-DOCKER-KernelSU_Action](https://github.com/ego-taboo/LXC-DOCKER-KernelSU_Action)，供 `for_k20pro` 仓库中 `-legacy` 后缀工作流使用。补丁体系与主力文件不同，新项目不推荐使用。

| 脚本 | 功能 | 适用场景 |
|------|------|----------|
| `runcpatch.sh` | 定位 `cgroup_add_file()` 函数并注入 runc 兼容代码 | 旧版内核 cgroup 适配 |
| `add-lxc-docker-custom3.sh` | 从 ego-taboo 拉取简化 LXC 配置列表并追加到 defconfig | 简化 LXC 方案（去除约 80 项冗余配置） |
| `clangfix2.sh` | 清理 Makefile 中 `KBUILD_CFLAGS` 赋值以兼容 clang23 | zyc clang23 编译器修复 |

> `add-lxc-docker-custom3.sh` 仍依赖 ego-taboo 的 `lxc-docker-config3.txt`，该外部资源有失效风险。其余两个脚本为独立脚本，不依赖外部下载。

---

## 五、使用说明

本仓库本身不直接编译内核，作为补丁源被 [LXC-DOCKER-KernelSU_for_k20pro](https://github.com/3032252626/LXC-DOCKER-KernelSU_for_k20pro) 的 GitHub Actions 工作流在编译时自动拉取：
- 顶层的 `xt_qtaguid.patch` + `namespace.c` heredoc 被 `build-AB` 系列、`plan2` 工作流使用；
- keepalive 仅由 `build-droidspaces-clang14.yml` 内嵌注入，不引用本仓库。

如需本地调试，直接克隆本仓库后按上述各文件说明执行。
