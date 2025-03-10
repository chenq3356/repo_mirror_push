#!/bin/bash

if [ $# -ne 3 ]; then
    echo "fatal: Usage: $0 <project_path> <remote_url> <remote_branch>"
    exit 1
fi

project_path=$1         # 本地仓库的完整路径
remote_url=$2           # 远程仓库完整地址
remote_branch=$3        # 远程分支名

#截取路径
filepath=$(dirname "$project_path")
filename=$(basename "$project_path" "")

# 创建本地仓库存放的路径
mkdir -p ${filepath}

# 切换到仓库存放的路径
cd ${filepath} || { echo "fatal: Failed to enter local repository directory."; exit 1; }

# 拉取仓库
if test -e "$project_path"; then
  echo "$project_path is already exist"
else
    git clone ${remote_url} -b ${remote_branch} ${filename} || { echo "fatal: Failed to clone from GitLab."; exit 1; }
fi
# 拉取仓库
