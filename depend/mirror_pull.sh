#!/bin/bash

if [ $# -ne 2 ]; then
    echo "fatal: Usage: $0 <project_path> <remote_url>"
    exit 1
fi

project_path=$1         # 本地仓库的完整路径
remote_url=$2           # 新的远程仓库地址

#截取路径
filepath=$(dirname "$project_path")
filename=$(basename "$project_path" "")

# 进入本地仓库目录
mkdir -p ${filepath}

cd ${filepath} || { echo "fatal: Failed to enter local repository directory."; exit 1; }

# 拉取仓库
if test -e "${project_path}.git"; then
  echo "${project_path}.git is already exist"
else
    git clone --mirror ${remote_url} ${filename}.git || { echo "fatal: Failed to clone to GitLab."; exit 1; }
fi


