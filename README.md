# Group-25-liar-bars

簡介：
1.	專題題目簡介：
本次我們製作的專題類型是遊戲。根據最近小有名氣的撲克牌吹牛遊戲「騙子酒吧」，重現其玩法在我們的裝置上。
2.	成員分工：
侯孝臻：畫圖、單機版code、伺服器server。
黃柏翔：伺服器client、連機板code
3.	開發與執行環境：Ubuntu 22.04
4.	特殊需求：一局遊戲需要4人方可開始遊玩 + 1伺服器

研究方法與設計： 
1.	Server與Client程式個別功能：
Server： 
接受client端加入、集結4人以fork方式開啟遊戲、處理玩家送來的決定並處理、更新遊戲進度並回傳整體遊戲資訊給其他玩家。
Client： 
加入server並等待4人開啟遊戲，輪到自己回合時，藉由輸入特定按鍵(key bind)決定出牌選擇並傳送特定資訊給server，接受server傳來的資訊以得知目前遊戲的進行清況。
2.	Server與Client程式互動規則與資料傳輸格式：
Server和Client之間主要透過傳送特定格式的字串訊息來交換遊戲資訊，格式為[flag] [information]。flag為0~10每個分別代表不同的訊息(可見下表)，而後面則根據flag是甚麼來提供必要的資訊。如通知遊戲資訊的字串為(1, player index, is death, death count, death count down, is connected, cards)，通過讀取字串便可以處理遊戲所需的資訊。
flag	meaning

0	Initialize a new game

1	Inform player’s information

2	Inform current turn

3	Inform what the player just put

4	Inform doubt

5	Inform death

6	Inform win

7	Inform quit

8	Inform the last player own cards

9	Inform the game start

10	Pass the message to post

以下為遊戲進行流程圖： 

3.	例外狀況之分析與處理：
我們分析遊戲可能遇到的例外狀況有二：Server突然關閉或Client (player) 中途退出
對於Server突然關閉，與Server連接的每個Cient都會自動段開連線並結束執行。
對於Client中途退出，分為ctrl+D與ctrl+C兩種退出方式，ctrl+D退出時Server慧跟退出的Client說Bye!後退出、ctrl+C則無。不管是哪種退出方式，Server皆會重新發牌給其餘的Client並繼續遊戲。
 
成果：
1.	client介面介紹：
 
i.	牌桌（紅色框）：
玩家的面前會有目前擁有的牌，而其面前三個方向分別是其他三位玩家的資訊，四人中間的牌為這局遊戲的table牌。因為是坐成一圈玩的遊戲，所以每名玩家左右的玩家會順序不一樣，如同現實的牌桌。
ii.	通知欄（黃色框）：
會廣播目前遊戲的進展與情況，當消息太多時，會慢慢的把舊的消息刷掉。
iii.	遊戲指示（橘色框）：會提示可以進行的操作。
iv.	狀態爛（綠色框）：記錄玩家資訊。

2.	client出牌功能介紹：
遊戲中輪到自己時會要求輸入按鍵來決定行動和選擇出的牌，行動階段如下：
第一階段：
action： 1（出牌）、2（質疑上一位出牌玩家）鍵。
特例：若該玩家是最後一個持有牌的以及新回合的第一名玩家則只能質疑。
第二階段：
若選擇1（出牌）：透過A、D鍵來移動牌的選項，你的牌會列在正前方，透過C鍵選擇你想出的牌（可選1~3張），最後按F送出。被選中的牌在被送出前會變成紅色醒目標示，而送出之後就不會顯示在玩家的面前。
 
若選擇0張牌或選了大於3張牌並按F送出，會收到Invalid choose的通知並且需要重新選擇正確的張數再送出
 

若選擇2（質疑上一位出牌玩家）：直接進入質疑，玩家不用再做其他動作。Server會判斷質疑是否成功並顯示在通知欄
i.	質疑成功：上一位出牌玩家玩一次俄羅斯輪盤並開始新一局遊戲，發牌後由上一位出牌玩家開始出牌
 
ii.	質疑失敗：質疑者玩一次俄羅斯輪盤並開始新一局遊戲，發牌後由質疑者開始出牌
 
3.	Client退出：
i.	Q鍵：當輪到玩家選擇行動時，他可以按下Q鍵退出遊戲，Server會重新發牌開始新一局遊戲並於通知欄告知其他玩家他退出了
 
ii.	Ctrl+D：當有玩家按下Ctrl+D，Server會跟那位玩家說Bye!並重新發牌開始新一局遊戲以及於通知欄告知其他玩家他退出了
  
iii.	Ctrl+C：當有玩家按下Ctrl+C，Server會直接重新發牌開始新一局遊戲以及於通知欄告知其他玩家他退出了
 

4.	死亡通知：
當有人玩俄羅斯輪盤死亡時，Server會告知其他玩家他死了並發牌開始新一局遊戲，死亡玩家仍會留在遊戲內並且仍可看到遊戲進行時的通知
 

5.	遊戲結束效果：
當遊戲結束時，贏家會看到牌桌上會有閃爍的字樣表示他贏了，通知欄也會顯示玩家勝利的訊息
 
 
結論：
本次的專題是復刻現在網路上的熱門遊戲，完成遊戲功能的過程讓我們覺得相當有趣。我們花了相當的時間在規則和美化遊戲介面上，調整版面的時候是相對麻煩的部分。遊戲性方面為了達到與一般遊戲一樣的直接輸入指示（不用按Enter）。所以我們在該處的function設成了block住的模式。我們覺得這是可以再改進的地方。如果想要讓這遊戲更加貼近原本完整的樣子，畢竟是類似心臟病的遊戲，需要觀察對方是是否在說謊，我們覺得可以讓牠加上語音功能，這樣就可以在線上邊聊天邊去猜對方出的是甚麼牌了。


參考文獻與附錄：
騙子酒吧steam連結： https：//store.steampowered.com/app/3097560/Liars_Bar/

github code:  https://github.com/Pakomu/Group-25-liar-bars


