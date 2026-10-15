# 額度星盤 · Passport Mac 同步助手 0.3.0（公開試用版）

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="README.zh_TW.md">繁體中文</a> · <a href="README.md">English</a></p>

將已登入 Codex 的 Mac 上可讀取的個人剩餘額度，加密同步到同一 Wi-Fi 下已配對的 AI Passport。只看公開重置訊息不需要此助手。

## 下載

[下載 Apple 晶片 Mac 安裝套件](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/StarshorePassport-AppleSilicon-0.3.0.zip) · [校驗檔案](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/SHA256SUMS.txt) · [版本與全部附件](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0)

這是 GitHub Pre-release，目前使用本機 ad-hoc 簽章，尚未使用 Developer ID 簽章或 Apple 公證。若 macOS 阻止正常開啟，請停止安裝並回報系統提示，不要關閉系統安全保護。

## 準備

需要 Apple 晶片 Mac；安裝並登入 Codex CLI，或使用安裝在「應用程式」中的 Codex 桌面應用程式。Mac 與裝置連接同一 Wi-Fi，Mac 保持開機、喚醒與連網。首次配對使用 USB 傳輸線。

## 首次使用

1. 解壓縮 ZIP，將 `StarshorePassport.app` 拖入「應用程式」。舊助手執行中時先結束再替換。首次開啟後，瀏覽器顯示「隨身星盤 · Mac 同步助手」；若未開啟，可前往 http://127.0.0.1:8942/ 。
2. USB 連接已開機的裝置，先點電腦的「連接裝置」。裝置可能重新啟動；等它回到首頁後，再長按硬體確定→設定→連線 Mac，出現允許提示後按確定。
3. 電腦顯示配對成功後自動同步。角色頁出現個人額度，即可拔掉 USB。之後裝置開機，助手會自動繼續同步；若曾手動暫停，在電腦點「恢復同步」。
4. 助手首次正常執行時登記目前使用者的登入啟動項目，可在「系統設定→一般→登入項目」關閉。關閉助手不會解除配對；需要撤銷授權時，在裝置設定選擇「解除 Mac 配對」，依裝置提示確認。

## 範圍與限制

- 約每 30 秒更新一次。Codex 未提供的額度時段不會被補成零；離線或 Mac 休眠時，裝置保留舊資料並標記更新時間。
- 裝置重新啟動後的週期同步已在一台裝置驗證；Mac 實際登出／登入或重新啟動後的自動啟動，以及真實 DHCP 換址恢復仍未完成實測。
- 自動換址需要裝置韌體提供配對裝置探索介面。較舊的社群 REV-2840 不包含該介面，仍可使用已儲存位址同步；新候選韌體包含介面，是否已公開請核對[社群作品](https://ai-passport.folotoy.cn/plays/1250/)的更新日誌。某些較大網段會跳過自動掃描，換址後可能需要重新連接。
- Codex 登入憑證與配對金鑰留在 Mac，裝置只接收加密額度摘要。公開重置公告不能證明個人額度已恢復。
- 回報問題請提供助手版本、韌體版本、macOS 版本及操作步驟；不要傳送登入憑證、配對檔案或 Wi-Fi 密碼。

這是額度星盤 AI Passport 的配套工具；Quota Orbit Mac 桌面主程式另有獨立安裝套件。本專案與 OpenAI、FoloToy 沒有官方背書關係。
