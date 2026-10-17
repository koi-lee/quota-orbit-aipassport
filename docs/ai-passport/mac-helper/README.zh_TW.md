# 額度星盤 · Passport Mac 同步助手 0.3.2（公開試用版）

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="README.zh_TW.md">繁體中文</a> · <a href="README.md">English</a></p>

將已登入 Codex 的 Mac 上可讀取的個人剩餘額度，加密同步到同一 Wi-Fi 下已配對的 AI Passport。只看公開重置訊息不需要此助手。

## 下載

[下載 Apple 晶片 Mac 安裝套件](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.2/StarshorePassport-AppleSilicon-0.3.2-notarized.zip) · [校驗檔案](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.2/SHA256SUMS.txt) · [版本與全部附件](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.2)

安裝包已完成 Developer ID 簽章與 Apple 公證，已附加公證票據並通過 Gatekeeper 檢查。仍作為公開試用版發行；功能測試範圍見下方限制。

## 準備

**請先核對系統版本：** 目前 0.3.2 安裝包僅在 macOS 27.0.1 / Apple 晶片 Mac 驗證；較舊系統暫不列為已支援。這是目前安裝包的相容限制，簽章及公證通過不代表舊系統相容；裝置獨立查看公開公告不受影響。

需要 Apple 晶片 Mac；安裝並登入 Codex CLI，或使用安裝在「應用程式」中的 Codex 桌面應用程式。Mac 與裝置連接同一 Wi-Fi，Mac 保持開機、喚醒與連網。首次配對使用 USB 傳輸線。

## 首次使用

1. 解壓縮 ZIP，將 `StarshorePassport.app` 拖入「應用程式」。舊助手執行中時先結束再替換。首次開啟後，瀏覽器顯示「隨身星盤 · Mac 同步助手」；若未開啟，可前往 http://127.0.0.1:8942/ 。
2. USB 連接已開機的裝置，先點電腦的「連接裝置」。裝置可能重新啟動；等它回到首頁後，再長按硬體確定→設定→連線 Mac，出現允許提示後按確定。
3. 電腦顯示配對成功後自動同步。角色頁出現個人額度，即可拔掉 USB。之後裝置開機，助手會自動繼續同步；若曾手動暫停，在電腦點「恢復同步」。
4. 助手首次正常執行時登記目前使用者的登入啟動項目，可在「系統設定→一般→登入項目」關閉。關閉助手不會解除配對；需要撤銷授權時，在裝置設定選擇「解除 Mac 配對」，依裝置提示確認。

## 遇到問題時

| 看到什麼 | 接下來怎麼做 |
| --- | --- |
| 等待連接裝置／第一次同步失敗 | 先完成上面的 USB 配對步驟。不要先解除既有配對。 |
| 配對未完成 | 檢查傳輸線與裝置是否開機，只連接一台 Passport；電腦點選連接後，等裝置啟動，再於硬體設定選「連接 Mac」並確認允許。 |
| 個人額度同步未完成 | 先確認 Codex 已登入，再檢查 Mac 與裝置是否在同一 Wi-Fi、裝置是否開機。助手會自動重試；若曾手動暫停，請點「恢復同步」。 |
| 瀏覽器頁面無法開啟，或顯示其他程式 | 先確認助手正在執行，再重新開啟助手。若本機 8942 連接埠仍被其他程式占用，請回報，不要在陌生頁面輸入憑證。 |
| 點「結束助手」後 | 結束成功後顯示「助手已結束」並停止更新。關閉該頁；再次開啟應用程式即可恢復執行。 |
| macOS 阻止開啟 | 確認下載的是 `-notarized.zip`；記錄系統原文並回報，不關閉系統安全保護。 |

助手會分別提示尚未配對、配對記錄無法讀取、裝置無法連接或 Codex 額度讀取失敗；讀取失敗時請檢查安裝與登入，不代表一定已登出。關閉瀏覽器分頁不會結束助手；要停止執行，請點「結束助手」。

## 回報問題

[提交使用問題](https://github.com/koi-lee/quota-orbit-aipassport/issues/new?template=passport_helper.yml)。可複製以下資訊，不知道的版本填「不確定」：

```text
助手版本／安裝包檔名：
macOS 版本／Apple 晶片型號：
社群韌體版本：
操作步驟：
預期結果：
實際結果／完整錯誤提示：
是否每次發生：
```

截圖請先遮住個人額度與帳號資訊。不要上傳配對檔案、登入憑證、Wi-Fi 密碼或未經檢查的完整記錄。

公開社群頁面無法讀取時，可參考[純文字版本查詢入口](../public-release.zh_CN.md)。

## 範圍與限制

- 約每 30 秒更新一次。Codex 未提供的額度時段不會被補成零；離線或 Mac 休眠時，裝置保留舊資料並標記更新時間。
- 裝置重新啟動後的同步及 0.3.2 登入服務恢復同步已在本機驗證；修復後再次完整重新啟動 Mac、真實 DHCP 換址及全新電腦首次配對仍待實測。
- 社群 REV-2886 起包含裝置位址探索；舊 REV-2840 不含此功能。請核對[社群更新記錄](https://ai-passport.folotoy.cn/plays/1250/)。實際 DHCP 換址恢復仍待測試；部分較大網段會跳過掃描，找不到時可重新連接裝置，不必先解除配對。
- Codex 登入憑證與配對金鑰留在 Mac，裝置只接收加密額度摘要。公開重置公告不能證明個人額度已恢復。
- 回報問題請提供助手版本、韌體版本、macOS 版本及操作步驟；不要傳送登入憑證、配對檔案或 Wi-Fi 密碼。

這是額度星盤 AI Passport 的配套工具；Quota Orbit Mac 桌面主程式另有獨立安裝套件。本專案與 OpenAI、FoloToy 沒有官方背書關係。
