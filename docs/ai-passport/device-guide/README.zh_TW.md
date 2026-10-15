<p align="right"><a href="../../../README.zh_CN.md">简体中文</a> · <strong>繁體中文</strong> · <a href="../../../README.md">English</a></p>

# 額度星盤 · AI Passport

**把 Codex 額度與公開重置消息放在桌邊的小螢幕上，減少切換視窗查詢。** 這是安裝在 AI Passport 硬體上的玩法，不是 Mac 桌面應用程式。

- **看公告：** 裝置連上 Wi-Fi，就能獨立查看預告與完成公告；可開啟不同提示音。不需要 Mac，也不需要登入 Codex。
- **看個人額度：** 選裝 Mac 同步助手，角色頁會顯示剩餘額度，月夜檔案員手上的書燈隨額度改變。
- **輕鬆互動：** 按確認點亮星星、聽祈願音；來信頁使用微信掃碼進入「星岸重置助手」。

| 公開公告 | 個人額度與書燈 |
|---|---|
| ![公告示意](images/home.png) | ![額度示意](images/quota-week-only.png) |

圖片由介面程式碼渲染，資料為示例，非實機照片；部分圖片保留較早文案。公開公告不代表你的個人額度已恢復，點星不會增加額度。

[查看角色額度與祈願互動示意](images/role-prayer.jpg)

## 要下載哪一個？

先準備 AI Passport。只看公告安裝第一項；想看個人額度，再加第二項。

| 用途 | 安裝位置 | 入口 |
|---|---|---|
| 必裝：裝置韌體 | AI Passport | [社群作品 1250 → 安裝最新版](https://ai-passport.folotoy.cn/plays/1250/) |
| 選裝：Passport Mac 同步助手 0.3.0 | Apple 晶片 Mac | [繁體安裝說明](../mac-helper/README.zh_TW.md) · [下載附件](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0) |

助手安裝包是 `StarshorePassport-AppleSilicon-0.3.0.zip`，不必安裝 Python。Source code 是原始碼，不是安裝包。助手仍是預發行版本，尚未通過 Apple 公證；若 macOS 阻止正常開啟，請停止並回報提示，不要關閉系統安全防護。Intel Mac 與 Windows 尚無經過驗證的助手安裝包。

## 第一次使用

1. 用電腦 Chrome 或 Edge 開啟社群作品頁，以 USB 資料線連接開機裝置，按安裝最新版並選擇裝置，完成前保持連接。這會取代目前玩法，可能清除舊設定。手機連上裝置顯示的 `Starshore-…` 熱點，使用螢幕密碼；在手機瀏覽器開啟 [http://192.168.4.1](http://192.168.4.1)，輸入家中的 **2.4 GHz Wi-Fi**，按連線並儲存。出現已同步公開消息即成功。失敗可重填；逾時或換網時，長按確認 → 設定 → 重新配網，再次確認。
2. 上下鍵切換首頁、來信與角色；來信按確認顯示小程式碼，用微信掃碼，再按確認返回。長按確認進設定，可開提示音或切換簡體／繁體／English。
3. 個人額度為選用功能。Mac 先登入 Codex CLI，或使用「應用程式」中的 Codex 桌面應用；與裝置連同一 Wi-Fi。安裝並開啟[同步助手](../mac-helper/README.zh_TW.md)，用 USB 資料線連接裝置，先按電腦的連接裝置。等硬體啟動後，在硬體設定選連接 Mac，按確認允許。
4. 配對後會自動同步，角色頁出現個人額度即成功，可拔掉 USB。日常保持 Mac 開機喚醒、助手運行；若曾手動暫停，按恢復同步。角色頁短按確認點星；不再使用時，可在硬體設定確認解除 Mac 配對。

公開公告約每 5 分鐘檢查，個人額度約每 30 秒同步。「上次同步」指最近成功取得公開消息的時間，不是時鐘。Mac 休眠或兩分鐘未更新時，保留舊額度並標示舊資料，書燈變灰；未提供的額度視窗不會補為零。

## 版本與限制

社群 REV-2886 起含三語介面、重要操作二次確認與裝置位址探索；以社群公開更新記錄為準，原始碼更新不會自動更新社群韌體。助手 0.3.0 在一台裝置驗證配對、拔線同步與裝置重啟恢復；Mac 真正重啟後的登入啟動、實際 DHCP 換址、長時間休眠恢復仍待測試。Codex 憑證留在 Mac，裝置只接收加密額度摘要。

[完整頁面與流程圖（簡體）](README.zh_CN.md) · [助手繁體指南](../mac-helper/README.zh_TW.md) · [English](../../../README.md)

## 與 Mac 主程式的差別

[Quota Orbit Mac 主程式](https://github.com/koi-lee/quota-orbit)提供桌寵與選單列介面，是獨立產品；使用 Passport 不需要安裝它。

韌體沿用根目錄 MIT 及原檔授權；助手採 [AGPL-3.0](../../../tools/lan-quota/LICENSE)。角色與小程式碼僅授權作為本專案的一部分散布，見[角色說明](../../../assets/character/README.md)與[小程式碼說明](../../../assets/wechat/README.md)。本專案與 OpenAI、FoloToy 無官方背書關係。
