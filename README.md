# android-lxc-docker

安卓内核 LXC/Docker 支持补丁仓库，由 [LXC-DOCKER-KernelSU_for_k20pro](https://github.com/3032252626/LXC-DOCKER-KernelSU_for_k20pro) 编译工作流引用。

fork 自 [wu17481748/android-lxc-docker](https://github.com/wu17481748/android-lxc-docker)。

## 文件说明

| 文件 | 用途 |
|------|------|
| `LXC-DOCKER-OPEN-CONFIG.sh` | 向 defconfig 注入 LXC/Docker 所需内核配置项 |
| `xt_qtaguid.patch` | qtaguid 网络模块补丁，解决容器网络问题 |
| `cgroup.patch` | cgroup 补丁（**已弃用**，较新内核源码已内置 cgroup 支持，重复打补丁会导致编译冲突） |

## 使用方法

```bash
# 注入 LXC/Docker 内核配置（-w 表示写入）
./LXC-DOCKER-OPEN-CONFIG.sh <xxx_defconfig> -w

# 打 qtaguid 补丁（有则打，无则跳过）
patch -p1 -d <内核源码目录> < xt_qtaguid.patch || echo "patch not applicable"
```

> 通用 GKI 内核谨慎开启某些配置，可能导致不开机。

## scripts-legacy

备份自 [ego-taboo/LXC-DOCKER-KernelSU_Action](https://github.com/ego-taboo/LXC-DOCKER-KernelSU_Action) 的旧版脚本，供 `for_k20pro` 仓库中 `-legacy` 后缀工作流使用：

| 脚本 | 用途 |
|------|------|
| `runcpatch.sh` | 旧版 runc cgroup 补丁 |
| `add-lxc-docker-custom3.sh` | 旧版简化 LXC 配置（去除约 80 项冗余配置） |
| `clangfix2.sh` | zyc clang23 编译修复 |

> 新版工作流不依赖 `scripts-legacy/`，已改用本仓库根目录下的 `LXC-DOCKER-OPEN-CONFIG.sh` + `xt_qtaguid.patch`。
