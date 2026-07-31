# android-lxc-docker

安卓内核 LXC/Docker 支持补丁仓库，由 [LXC-DOCKER-KernelSU_for_k20pro](https://github.com/3032252626/LXC-DOCKER-KernelSU_for_k20pro) 编译工作流引用。

fork 自 [wu17481748/android-lxc-docker](https://github.com/wu17481748/android-lxc-docker)。

> **声明**：本仓库补丁及脚本均源自上游社区，仅做引用整合，未进行自主开发。问题排查与适配由 AI 辅助完成。

---

## 一、仓库结构

```
android-lxc-docker/
├── LXC-DOCKER-OPEN-CONFIG.sh    # LXC/Docker 内核配置注入脚本
├── xt_qtaguid.patch            # qtaguid 网络模块补丁
├── cgroup.patch                # cgroup 补丁（已弃用，仅作存档）
└── scripts-legacy/             # ego-taboo 旧版脚本备份
    ├── runcpatch.sh
    ├── add-lxc-docker-custom3.sh
    └── clangfix2.sh
```

---

## 二、主力文件

### LXC-DOCKER-OPEN-CONFIG.sh

向内核 defconfig 注入 LXC/Docker 所需的 Namespaces、Cgroups、Netfilter 等配置项。

```bash
# 预览（不写入）
./LXC-DOCKER-OPEN-CONFIG.sh <xxx_defconfig>

# 写入
./LXC-DOCKER-OPEN-CONFIG.sh <xxx_defconfig> -w
```

> 来源：上游 wu17481748/android-lxc-docker。通用 GKI 内核谨慎开启某些配置，可能导致不开机。

### xt_qtaguid.patch

修复 `net/netfilter/xt_qtaguid.c` 中一处 rtnl lock 可能导致的内核 panic，容器网络相关。

```bash
patch -p1 -d <内核源码目录> < xt_qtaguid.patch || echo "patch not applicable"
```

> 来源：上游社区。有则打，无则跳过，不影响编译。

### cgroup.patch（已弃用）

向 `kernel/cgroup/cgroup.c` 的 `cgroup_add_file()` 函数追加一段前缀跳过逻辑。

> **已弃用**：较新内核源码已内置等效逻辑，重复打补丁会导致编译冲突。本文件仅作存档保留。

---

## 三、旧版脚本（scripts-legacy）

备份自 [ego-taboo/LXC-DOCKER-KernelSU_Action](https://github.com/ego-taboo/LXC-DOCKER-KernelSU_Action)，供 `for_k20pro` 仓库中 `-legacy` 后缀工作流使用。补丁体系与主力文件不同，新项目不推荐使用。

| 脚本 | 功能 | 适用场景 |
|------|------|----------|
| `runcpatch.sh` | 定位 `cgroup_add_file()` 函数并注入 runc 兼容代码 | 旧版内核 cgroup 适配 |
| `add-lxc-docker-custom3.sh` | 从 ego-taboo 拉取简化 LXC 配置列表并追加到 defconfig | 简化 LXC 方案（去除约 80 项冗余配置） |
| `clangfix2.sh` | 清理 Makefile 中 `KBUILD_CFLAGS` 赋值以兼容 clang23 | zyc clang23 编译器修复 |

> `add-lxc-docker-custom3.sh` 仍依赖 ego-taboo 的 `lxc-docker-config3.txt`，该外部资源有失效风险。其余两个脚本为独立脚本，不依赖外部下载。

---

## 四、使用说明

本仓库本身不直接编译内核，作为补丁源被 [LXC-DOCKER-KernelSU_for_k20pro](https://github.com/3032252626/LXC-DOCKER-KernelSU_for_k20pro) 的 GitHub Actions 工作流在编译时自动拉取。

如需本地调试，直接克隆本仓库后按上述各文件说明执行。
