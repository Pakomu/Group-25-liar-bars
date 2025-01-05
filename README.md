

網程設專題Instruction
===
0. 確認OS為Ubuntu 22.04且有unpv13e所有的檔案
1. 將project.c與project_cli_2.c放入unpv13e/tcpcliserv中
2. 修改Makefile
    ```sh
    include ../Make.defines

    PROGS =	tcpcli01 tcpcli04 tcpcli05 tcpcli06 \
		tcpcli07 tcpcli08 tcpcli09 tcpcli10 \
		tcpserv01 tcpserv02 tcpserv03 tcpserv04 \
		tcpserv08 tcpserv09 tcpservselect01 tcpservpoll01 tsigpipe

    all:	${PROGS}
    ...
    # 加入這兩行
    project:	project.o
		${CC} ${CFLAGS} -o $@ project.o ${LIBS}
    project_cli_2:	project_cli_2.o
		${CC} ${CFLAGS} -o $@ project_cli_2.o ${LIBS}
    ...
    ```
3. 執行make指令
    ```sh
    make project
    make project_cli_2
    ```
4. 先啟動server (project)
    ```sh
    ./project
    ```
5. 依序加入四個client (project_cli_2)，連線到project程式所在的IP address並取好client名字
    ```sh
    ./project_cli_2 [IP_addr] [Name]
    ```
6. 遊戲便可順利開始進行
