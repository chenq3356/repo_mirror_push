#!/bin/bash

if [ $# -ne 2 ]; then
    echo "fatal: Usage: $0 <project_path> <remote_url>"
    exit 1
fi

project_path=$1         # 本地仓库的完整路径
remote_url=$2           # 新的远程仓库地址

# 进入本地仓库目录
cd "$project_path" || { echo "fatal: Failed to enter local repository directory."; exit 1; }

# 提交仓库
git push --mirror "$remote_url" || { echo "fatal: Failed to push to GitLab."; exit 1; }
