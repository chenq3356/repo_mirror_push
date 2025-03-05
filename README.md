# 写在前面

> 在低版本的ubuntu编译过程中，cpp-httplib的兼容性不是很好，即使降低库版本后，编译能通过，但是程序运行起来，regex又一直抛出异常，所以干脆用libcurl改了一版。编译前请确保libcurl的环境，或者也可自行替换其他http库。


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
[UploadInfo]
upload=true                            	;是否执行仓库迁移
commitAgain=false                       ;是否重复提交，否的话跳过已经提交过的仓库
basePath=/home/img/code                 ;本地代码的路径
manifestsPath=/home/img/code/.repo/manifests    ;本地要提交的manifests仓库路径
repoPath=/home/img/code/.repo/repo      ;本地要提交的repo仓库路径
projectUrl=https://gitlab.example.com   ;目标gitlab地址,域名
apiToken=your_access_token              ;目标gitlab授权的API个人访问令牌
namespaceName=repotest                  ;远程仓库的根路径名称(无实际作用)
namespaceId=1635                        ;远程仓库的根路径id（仓库源码将提交到该路径下）

[XMLInfo]
outFile=example.xml                     ;将指定的XML以单文件的形式输出
basePath=/demo                          ;XML文件的路径
baseFile=xxx.xml                        ;预提交仓库的XML文件
```

> `commitAgain`: 如果 `true`，即使远程分支已存在也会重复执行`git push`操作(如果远程仓库没有新的提交，重复执行`git push`通常不会有问题)； 如果`false`，远程分支已存在时跳过`git push`操作。

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
