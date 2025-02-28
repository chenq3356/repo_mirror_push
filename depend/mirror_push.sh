#!/bin/bash

if [ $# -ne 3 ]; then
    echo "fatal: Usage: $0 <arg1> <arg2> <arg3>"
    exit 1
fi

project_path=$1 # 本地仓库的完整路径
git_url=$2		# 远程仓库地址
git_branch=$3

# 进入本地仓库目录
cd "$project_path" || { echo "fatal: Failed to enter local repository directory."; exit 1; }

# 获取当前分支名称，当前分支不存在，则创建master分支
current_branch=$(git rev-parse --abbrev-ref HEAD)
if [[ "$current_branch" == *"HEAD"* ]]; then
    current_branch=master
	git checkout -b $current_branch || { echo "fatal: Failed to checkout to master."; exit 1; }
fi

# 确保本地仓库是最新的
git fetch --all

# 判断是否为浅克隆
if [ -f "$project_path/.git/shallow" ]; then
    echo "The local repository is a shallow clone."
    echo "Converting to a full clone..."

	# 获取所有远程仓库的名称
	remotes=$(git remote)
	
	# 遍历每个远程仓库并获取 URL
	for remote_name in $remotes; do
		# 转换为完整克隆
		git fetch $remote_name --unshallow
	
		if [ ! -f "$project_path/.git/shallow" ]; then
			echo "Converting to a full clone ok"
			break
		fi
	done
fi

# 判断是否为浅克隆, 浅克隆不执行提交操作
if [ ! -f "$project_path/.git/shallow" ]; then

# 判断remote是否存在，如存在则使用set-url
remote_name="newrevision"
remote_url=$(git config --get remote."$remote_name".url)
if [ -z "$remote_url" ]; then
	git remote add "$remote_name" "$git_url"  || { echo "fatal: Failed to add GitLab remote."; exit 1; }
else
	git remote set-url "$remote_name" "$git_url"  || { echo "fatal: Failed to set-url GitLab remote."; exit 1; }
fi

# 推送所有分支到新的 GitLab 仓库
git push ${remote_name} ${current_branch}:${git_branch} || { echo "fatal: Failed to push branches to GitLab."; exit 1; }
# git push --all ${remote_name} || { echo "fatal: Failed to push branches to GitLab."; exit 1; }

# 推送所有标签到新的 GitLab 仓库
git push --tags ${remote_name} || { echo "fatal: Failed to push tags to GitLab."; exit 1; }


else
	echo "fatal: ${project_path} is shallow clone"
fi

