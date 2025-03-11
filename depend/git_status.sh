#!/bin/bash

if [ $# -ne 3 ]; then
    echo "fatal: Usage: $0 <project_path> <remote_revision> <remote_branch>"
    exit 1
fi

project_path=$1         # 本地仓库的完整路径
remote_revision=$2      # 远程revision值
remote_branch=$3        # 远程分支名


# 进入本地仓库目录
cd "$project_path" || { echo "fatal: Failed to enter local repository directory."; exit 1; }

# 获取本地最新一次提交的哈希值
local_latest_commit=$(git log -1 --pretty=format:'%H')

echo "local_latest_commit: $local_latest_commit"

# 判断XML中的revision是否是 hash， 然后与本地的哈希作比较
if [[ "$remote_revision" =~ ^[0-9a-f]{40}$ ]]; then
    if [ ! "$remote_revision" = "$local_latest_commit" ]; then
        echo "revision: $remote_revision"
    fi
fi

# 获取所有远程仓库的名称
remotes=$(git remote)

# 遍历每个远程仓库
for remote_name in $remotes; do
    # 获取远程最新的提交 hash， 然后与本地的哈希作比较
    remote_latest_commit=$(git ls-remote "$remote_name" "$remote_branch" | awk '{print $1}')
    if [ ! "$remote_latest_commit" = "$local_latest_commit" ]; then
        echo "remote_latest_commit: $remote_latest_commit $remote_name $remote_branch"
    fi
done

# 获取本地分支的个数（排除空行）
branch_count=$(git branch --format='%(refname:short)' | wc -l)
if (( branch_count > 1 )); then
    echo "branch_count: $branch_count"
fi

# 查看是否有未提交的修改
status_info=$(git status)
search_str="nothing to commit, working tree clean"
if [[ ! "$status_info" == *"$search_str"* ]]; then
  echo "Changes not staged for commit"
fi
