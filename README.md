# 一、使用说明

### 1、执行 `build.sh`
```
./build.sh
```
### 2、进入 `output/bin` 目录，目录文件如下
```
output
└── bin
    ├── mirror_push.sh
    ├── repo_mirror_push
    └── setting.ini

```
# 3、配置 `setting.ini`
```
# 根据repo的XML文件拉取代码到本地，依赖于XMLConfig配置,XML->remote->fetch 需要填写完整的路径
[RepoDownload]
enable=false                    ;是否执行
mirror=false                    ;是否以镜像的方式下载
projectsPath=/demo              ;下载后存放的本地根路径

# 根据repo的XML文件上传代码到服务，依赖于XMLConfig配置
[RepoUpload]
enable=false                    ;是否执行
mirror=false                    ;是否以镜像的方式提交，本地的源码也需要是镜像拉取
repeate=false                   ;允许重复提交操作
to_unshallow=false              ;是否将浅克隆转为完整克隆，否的话则重新创建git
projectsPath=/demo              ;本地代码的根路径
targetHost=https://gitlab.example.com ;目标gitlab地址,域名
targetToken=your_access_token     ;目标gitlab授权的API个人访问令牌
targetGroupId=0                   ;目标远程仓库的根路径的id

# 根据repo的XML文件查看Hash,Changed，依赖于XMLConfig配置
[RepoStatus]
enable=false                    ;是否执行
projectsPath=/demo              ;本地代码的根路径

# REPO仓库的XML文件
[XMLConfig]
outFile=example.xml             ;将指定的XML以单文件的形式输出
basePath=/demo                  ;XML文件的路径
baseFile=xxx.xml                ;预提交仓库的XML文件
```

> `repeate`: 如果 `true`，即使远程分支已存在也会重复执行`git push`操作(如果远程仓库没有新的提交，重复执行`git push`通常不会有问题)； 如果`false`，远程分支已存在时跳过`git push`操作。

> `XMLInfo.baseFile`: 预提交仓库的XML文件推荐使用命令`.repo/repo/repo manifest -m xxx.xml -o xxx.xml`生成的文件，而不是直接使用manifests下的文件。

> `apiToken`: gitlab个人账户授权的`apitoken`,通过点击右上角【用户头像】->选择【settings】 ->菜单中选择【Access Tokens】，右侧 进行创建，建议给予全部权限

![apiToken1](png/apiToken1.png)
![apiToken2](png/apiToken2.png)

> `namespaceId`: 远程库根路径的ID,仓库源码将提交到此路径下

![groupid](png/groupid.png)

### 4、执行 `repo_mirror_push`

支持重复执行该指令，已经提交过的仓库会根据`commitAgain`来决定是否重复执行
```
./repo_mirror_push
```

执行完成后，远程仓库目录结构如下：

```
repotest
├── manifest
├── repo
├── remote1
├── remote2
└── remoten
```

# 二、拉取代码
### 1、安装 repo命令
```
cd work
git clone https://gerrit.rock-chips.com:8443/repo-release/tools/repo
```
### 2、下载 manifest

```
# 通过 --repo-url 参数指定新的repo工具
./repo/repo init --repo-url  git@example.com:repotest/repo.git -u git@example.com:repotest/manifests.git -m demo.xml 

# 删除1步骤中安装的repo
rm -rf repo
```
### 3、同步代码
```
.repo/repo/repo sync -c
```
