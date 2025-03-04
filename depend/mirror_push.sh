#!/bin/bash

if [ $# -ne 4 ]; then
    echo "fatal: Usage: $0 <project_path> <remote_url> <remote_branch> <loss_remote>"
    exit 1
fi

project_path=$1         # 本地仓库的完整路径
remote_url=$2           # 新的远程仓库地址
remote_branch=$3        # 新的远程分支名
loss_remote=$4          # 旧的远程是否已丢失
is_shallow="false"		# 浅克隆标识

# 进入本地仓库目录
cd "$project_path" || { echo "fatal: Failed to enter local repository directory."; exit 1; }


# 判断是否为浅克隆
if [ -f "$project_path/.git/shallow" ]; then
    is_shallow="true"
fi

# 以下执行的是浅克隆情况下，并且无法从源仓库中拉取更新
if [[ "$is_shallow" == "true" && "$loss_remote" == "true" ]]; then
    echo "The local repository is a shallow clone and loss remote."
    echo "Recreate git repository..."
    rm -rf .git/

    git init || { echo "fatal: Failed to init repository"; exit 1; }
    git remote add newrevision "$remote_url"  || { echo "fatal: Failed to add GitLab remote."; exit 1; }
    git add . ||  { echo "fatal: Failed to add git file"; exit 1; }
    git commit -m "recreate, because the local repository is a shallow clone" > /dev/null

    git push -u newrevision master || { echo "fatal: Failed to push branches to GitLab."; exit 1; }

    exit 0;
fi

# 以下执行的是其他情况

# 获取当前分支名称，当前分支不存在，则创建master分支
current_branch=$(git rev-parse --abbrev-ref HEAD)
if [[ "$current_branch" == *"HEAD"* ]]; then
    current_branch=master
    git checkout -b $current_branch || { echo "fatal: Failed to checkout to master."; exit 1; }
fi

# 确保本地仓库是最新的
if [[ "$loss_remote" == "false" ]]; then
    git fetch --all
fi

# 判断是否为浅克隆
if [[ "$is_shallow" == "true" && "$loss_remote" == "false" ]]; then
    echo "The local repository is a shallow clone."
    echo "Converting to a full clone..."

    # 获取所有远程仓库的名称
    remotes=$(git remote)

    # 遍历每个远程仓库并获取 URL
    for remote_name in $remotes; do
        # 转换为完整克隆
        git fetch $remote_name --unshallow
        # 再次判断是否为浅克隆，否-退出
        if [ ! -f "$project_path/.git/shallow" ]; then
            echo "Converting to a full clone ok"
            break
        fi
    done
fi

# 再次判断是否为浅克隆
if [ -f "$project_path/.git/shallow" ]; then
    echo "fatal: Failed to Converting a full clone"
    exit 1;
fi

# 判断remote是否存在，如存在则使用set-url
remote_info=$(git config --get remote."newrevision".url)
if [ -z "$remote_info" ]; then
    git remote add newrevision "$remote_url"  || { echo "fatal: Failed to add GitLab remote."; exit 1; }
else
    git remote set-url newrevision "$remote_url"  || { echo "fatal: Failed to set-url GitLab remote."; exit 1; }
fi

# 推送所有分支到新的 GitLab 仓库
git push newrevision ${current_branch}:${remote_branch} || { echo "fatal: Failed to push branches to GitLab."; exit 1; }
# git push --all ${remote_name} || { echo "fatal: Failed to push branches to GitLab."; exit 1; }

# 推送所有标签到新的 GitLab 仓库
git push --tags newrevision || { echo "fatal: Failed to push tags to GitLab."; exit 1; }

