# GitHubでDLLを作る手順

1. GitHubの `SafeDamageThreshold-build` リポジトリを開く。
2. **Add file → Upload files** を押す。
3. このZIPを展開し、中身をすべてリポジトリ直下へアップロードする。
   - `.github` は隠しフォルダなので、ZIPのままではなく展開後のフォルダ構造を維持すること。
4. **Commit changes** を押す。
5. 上部の **Actions** を開き、`Build SafeDamageThreshold` を選ぶ。
6. 自動実行されない場合は **Run workflow** を押す。
7. 完了後、実行結果ページ下部の **Artifacts** から `SafeDamageThreshold-0.1.0` をダウンロードする。
8. ダウンロードしたArtifactを一度展開すると、その中に導入用 `SafeDamageThreshold-0.1.0.zip` がある。

ビルドが赤く失敗した場合は、Actionsの赤い実行結果を開き、失敗した工程のログを共有してください。
