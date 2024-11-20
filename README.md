# SilexGL

レンダリングの学習用に使用しているレンダラーです。<br>



## 動作確認環境

* Windows 10 / 11
* MSVC - C++ 20
* NVIDIA GForce GTX 1060
* OpenGL 4.5



## ビルド

```bat
git clone --recursive https://github.com/Suuta/SilexGL.git
```

クローン後に ***project.bat*** を実行して *VisualStudio* ソリューションを生成します。<br>
生成後、ソリューションを開いてビルドをするか ***build.bat*** の実行でビルドが行われます。<br>
プロジェクト生成ツールに [Premake](https://premake.github.io/) を使用していますが、別途インストールは必要ありません。<br>



## 操作

| 操作                               | バインド                       |
| ---------------------------------- | ------------------------------ |
| カメラ移動                         | 右クリック + W, A, S, D, Q, E  |
| シーンを開く                       | Ctr + O                        |
| シーンを保存                       | Ctr + S                        |
| 新規シーン                         | Ctr + N                        |
| オブジェクトを選択                 | 左クリック                     |
| 選択オブジェクトにフォーカス       | 選択した状態で F               |
| ギズモのワールド・ローカル切り替え | Q                              |
| ギズモの操作タイプ変更             | W, E, R                        |
| アセット生成                       | アセットブラウザ内で右クリック |

<br>

## スクリーンショット

![SilexGL](Image/SilexGL.jpg?raw=true "SilexGL")

