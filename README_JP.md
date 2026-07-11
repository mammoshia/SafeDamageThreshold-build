# Safe Damage Threshold 0.1.0

Fallout 4 Anniversary Edition 1.11.221用のF4SEプラグインです。

## 仕組み

- 物理DT = 現在の物理防御値 × INIの物理係数
- エネルギーDT = 現在のエネルギー防御値 × INIのエネルギー係数
- DTを引いた結果は必ず0以上へ制限
- 残ったダメージを既存のDR、DirectHit、Bastionなどの計算へ渡す
- 初期設定では通常防具・パワーアーマーとも2%、プレイヤーのみ対象

物理防御1000のPAなら物理DTは20です。15ダメージの攻撃は0、50ダメージの攻撃は30相当を後段へ渡します。

## 必須

- Fallout 4 1.11.221
- 対応版F4SE
- Address Library for F4SE Plugins

## 導入

配布ZIPをMOD管理ツールで導入してください。ESP/ESMはありません。

古い `Power Armor Damage Threshold`、そのBastionパッチ、`Damage Threshold.esm`を利用したDT付与MODは外してください。併用するとDTが二重適用されます。

設定は `Data/F4SE/Plugins/SafeDamageThreshold.ini` です。T-45以下でも通常グールをさらに確実に止めたい場合は、PA用の2項目だけ `0.03` に変更してください。

## 互換性

このプラグインはF4SEのPreload段階でフックを入れます。通常のLoad段階で同じ処理を使うDirectHit、Bastion、Dynamic Dismember Systemより内側で処理されるため、それらが調整したダメージを受け取ってから、ゲーム本体へ渡す直前にDTと0クランプを適用する構成です。

初回版は実ゲーム検証が必要です。問題が出た場合は `bLogHits=1` にして短時間だけ再現し、`Documents/My Games/Fallout4/F4SE/SafeDamageThreshold.log` を確認してください。

## ソースからのビルド

XMake 3.0以降とVisual Studio 2022が必要です。プロジェクト直下で次を実行してください。

```powershell
git clone --depth 1 https://github.com/libxse/commonlibf4.git lib/commonlibf4
git -C lib/commonlibf4 submodule update --init --recursive --depth 1
xmake f -m releasedbg -y
xmake build -y
```
