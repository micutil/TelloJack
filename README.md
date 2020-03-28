# TelloJack
for Tello toy drone controller program.

## TelloJackについて
[TELLOは、RYYZE社](https://www.ryzerobotics.com/jp/tello)の200g以下の軽量トーイドローンです。このドローンは、操作コマンドが公開されているため、多くの方が、操作させるプログラムを作って公開されています。

TelloJackは、MicJackにTelloのコントロールの機能を組み込んだ時に、それをTelloJackと呼んでいました。今回、M5StackやOdroid-GOで動かせるようにしたものを作りましたので、同じ名称で公開しました。MicJackでESP-WROOM-02で動くようなものを作っていたんですが、インターフェースは @hsgucciさん([GitHub](https://github.com/hsgucci404/m5stack_tello), [Quitta](https://qiita.com/hsgucci/items/4571c060ece376f80db3) )のプログラムをモディファイさせて頂いております。

<img src=image/accel.jpg Height =200>
<img src=image/jsUnit.jpg Height=200>
<img src=image/facesJs.jpg Height=314>
<img src=image/odroidGo.jpg Height=314>

#### 同梱内容
- TelloJack.ino (Arduinoのスケッチ)
- binフォルダ
	- TelloJack_Accel.bin (左上写真：Gray, fireの加速度センサーで傾けて操作します）
	- TelloJack_JS.bin（右上写真：[JoyStickユニット](https://docs.m5stack.com/#/en/unit/joystick)をつなげて操作します）
	- TelloJack_FacesJS.bin（左下写真：[Faces](https://docs.m5stack.com/#/en/core/face_kit)の[JoyStickモジュール](https://docs.m5stack.com/#/en/module/joystick)で操作します）
	- TelloJack_GO.bin（右下写真：[Odroid-GO](https://www.hardkernel.com/shop/odroid-go/) arduino版）
	- TelloJack_GO.fw（[Odroid-GO](https://www.hardkernel.com/shop/odroid-go/) 純正ファームウエア版）
- SD-Updaterフォルダ（[SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater)用のjpgやjsonファイルが入っています）
- imageフォルダ（README.md用の画像が入ってます）

1. TelloJack.inoは[ESP32-Chimera-Core by tobozo](ttps://github.com/tobozo/ESP32-Chimera-Core)を使っています。
2. FACESは、最近ボトム単体でも売られています。[FACES BOTTOM](https://docs.m5stack.com/#/en/module/faces)、[FACES II BOTTOM](https://docs.m5stack.com/#/en/module/facesII) と２種類あります。
3. Odroid-GO 純正ファームウエア版は、[FWConverter](https://github.com/micutil/Odroid-GO_FWConverter)を使って作成しています。

## 操作方法
**Telloに接続**

1. Telloのスイッチを入れる。
2. TelloJackを起動すると、Wi-Fiをスキャンしますので、TelloのSSIDが取得できているかもしれません。**左側の"NEXT"ボタン**を押すと取得したSSIDを順に表示します。
2. TelloのSSIDが見つからない場合は、**右側の"SCAN"ボタン**を押して下さい。
3. 接続するTelloが見つかったら、**中央の"CONNECT"ボタン**を押して下さい。うまく接続できると、**"Connect to [TelloのSSID名]"**と表示されます。

<iframe width="260" height="315" src="https://www.youtube-nocookie.com/embed/q9aXnZNxYyE" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

**飛行操作**

1. "TAKE OFF"ボタンで離陸します。
2. "LAND"ボタンで着陸します。
3. **Accelerometor版（加速度センサー版）**はM5Stackを前後左右に傾けると前進後退左右に動きます。また、**中央の"CW/CCW/UP/DOWN”ボタン**を押していると、上昇下降左回転右回転の動作になります。
3. **JoyStick版**は、ジョイスティックを上下左右で前進後退左右に動きます。Accel版と同様に**中央の"CW/CCW/UP/DOWN”ボタン**を押していると、上昇下降左回転右回転の動作になります。また、JoyStick版は、ジョイスティックを押して動かしても上昇下降左回転右回転の動作になります。

## ビルド設定

いろいろな種類のビルドは、TelloJack.inoの最初の方に宣言があります。

**TelloJack_Accel**

	#ifndef ARDUINO_ODROID_ESP32    // Odroid-GOで操作
	#define useJoyStickUnit       // JoyStick Unitで操作
	//#define useGrayAccel            // 加速度センサーで操作
	//#define useFacesJoyStick      // FACESのJoyStick操作
	//#define useFacesKey           //Underconstruction !!!!
	#endif
	
**TelloJack_JS**

	#ifndef ARDUINO_ODROID_ESP32    // Odroid-GOで操作
	//#define useJoyStickUnit       // JoyStick Unitで操作
	#define useGrayAccel            // 加速度センサーで操作
	//#define useFacesJoyStick      // FACESのJoyStick操作
	//#define useFacesKey           //Underconstruction !!!!
	#endif
	
**TelloJack_FacesJS**

	#ifndef ARDUINO_ODROID_ESP32    // Odroid-GOで操作
	//#define useJoyStickUnit       // JoyStick Unitで操作
	//#define useGrayAccel            // 加速度センサーで操作
	#define useFacesJoyStick      // FACESのJoyStick操作
	//#define useFacesKey           //Underconstruction !!!!
	#endif

**TelloJack_GO**

	#ifndef ARDUINO_ODROID_ESP32    // Odroid-GOで操作
	//#define useJoyStickUnit       // JoyStick Unitで操作
	//#define useGrayAccel            // 加速度センサーで操作
	//#define useFacesJoyStick      // FACESのJoyStick操作
	//#define useFacesKey           //Underconstruction !!!!
	#endif
	

## ライセンス
CC 4.0 BY Micono (https://github.com/micutil/TelloJack)
