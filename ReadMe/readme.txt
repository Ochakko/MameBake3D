﻿
このソフトの著作権はおちゃっこにあります。
オープンソースのライセンスはLGPLです。

このソフトにより問題が起きたとしてもその責任は持ちません。
自己責任でお願いします。




#### 動作環境 　ここから　####

	Mayaは趣味の人？に対しては安価で？提供されている
	（インディー契約の条件については以下のページ）
	https://area.autodesk.jp/product/maya-3ds-max-indie/


開発環境(2023/07/07時点)を書きます。
開発環境(ソフトウェア)を書きます。
OS Windows11 Pro
VisualStudio2022Pro
FBXSDKは2020.3.4(VC2022)
Microsoft MultiThreadLibrary PPL.
DirectX11 (Windows10 SDK or VisualStudioからインストール)
githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
githubのMicrosoftのDirectXTex(DirectX11でtgaが読める)を利用
githubのbullet physics ver2.89(http://bulletphysics.org/wordpress/)
VisualStudio2022Pro(VisualStudioSubscription)
MayaIndie(Subscription FBXの確認はMayaでする)
Unity3D 2022LTS(プロが作ったアセットをUnityAssetStoreでゲットする)
ゲームパッド操作用のライブラリはGitHubのヒロッチさんのオープンソースを編集して使いました。
(ヒロッチさんのゲームパッドライブラリのライセンスはReadMe/DS4HidInput_LICENSE.txtに置きました。)




Testディレクトリのモデルデータ作成にはVRoidStudioを使用しています
https://vroid.com/studio





開発環境(2021/03/19_2時点)を書きます。
FBXSDKは2020.2(VC2019)
Microsoft MultiThreadLibrary PPL.
DirectX11 (Windows10 SDK or VisualStudioからインストール)
githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
githubのbullet physics ver2.89(http://bulletphysics.org/wordpress/)
VisualStudio2019Pro(VisualStudioSubscription)
MayaIndie(Subscription FBXの確認はMayaでする)



開発環境(2021/03/04時点)を書きます。
FBXSDKは2020.2(VC2019)
Intel OneAPI Base Toolkit v.2021のtbb(Intelのサイトに登録してダウンロードインストール)
DirectX11 (Windows10 SDK or VisualStudioからインストール)
githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
VisualStudio2019Pro(VisualStudioSubscription)
MayaIndie(Subscription FBXの確認はMayaでする)

2017/12/17コミット分から
bullet physicsのバージョンが2.86になりました。
http://bulletphysics.org/wordpress/



開発環境(2021/02/16時点)を書きます。
FBXSDKは2020.0.1(Debugビルドも出来た)
DirectX11 (Windows10 SDK or VisualStudioからインストール)
githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
VisualStudio2015Pro(VisualStudioSubscription)
MayaIndie(Subscription FBXの確認はMayaでする)


開発環境(2021/01/26時点)を書きます。
FBXSDKを2018.1.1に戻しました


開発環境(2020/12/17時点)を書きます。
なんちゃってワークステーション（ホワイトワークステーション）つまりWWSにて開発
Microsoft VisualStudio Subscription   VisualStudio2015Pro(2015 Comuでは開けなかったです)
FBXSDKは2020.0.1
  サンプルとして付けているFBXファイルはまだ2013のままであり
  MameBake3Dで保存しなおさないと修正は適用されない
Maya2013とAutodesk Maya Indie


開発環境(2020/09/24時点)を書きます。
なんちゃってワークステーション（ホワイトワークステーション）つまりWWSにて開発
VisualStudio2015Pro(2015 Comuでは開けなかったです)
FBXSDKは2018.1　読み込みはFBX2018.1も可だが書き出しはFBX2013(手持ちのMayaが2013だから)
Maya2013(Autodeskが日本人にも売上制限付き格安インディー契約を認めたニュースがありました)


開発環境(2018/11/22時点)を書きます。
Mac(ちくわ)とVM。
BootCampではなくて、またVMでやることにしました。
バージョンアップしたところVMでのバックバッファのずれが直っていました。
垂直同期もほぼ出来ているようです。
Ctrl + 1キーを押せば物理シミュレーション時にも60fpsで動きます。


過去の環境
開発環境(2018/03/04時点)を書きます。
Mac(ちくわ)とBootCampとWindows10 Pro で開発しています。

#### 動作環境 　ここまで　####



このプログラムは外部のライブラリを使用しています。
Microsoft DirectX SDK (June 2010) (マイクロソフトの3D描画ライブラリ)
OpenGL(３Ｄ描画ライブラリ(このプログラムではbulletからリンクされている))
bullet physics(AMDのオープンソースの物理演算ライブラリ http://bulletphysics.org/wordpress/)
FBX SDK 2020.0.1(Autodeskの互換用ファイル操作SDK  http://www.autodesk.com/fbx)
Intel TBB(Intelのスレッド操作用のライブラリ　https://www.threadingbuildingblocks.org/)

VisualStudioのバージョンは2015 (VC 14)です。

（
参考までに
Visual Studioの過去のバージョンは
MicrosoftのサイトでMSDNサブスクリプションの契約することにより使用可能です。
個別に買うより安いです。
）


MameBake3D.exeと同じディレクトリに以下の４つのDLLを置く必要があります。
libfbxsdk.dll (FBX SDK)
tbb.dll (Intel TBB)
tbbmalloc.dll (Intel TBB)
tbbmalloc_proxy.dll (Intel TBB)


2017/12/17コミット分から
bullet physicsのバージョンが2.86になりました。
http://bulletphysics.org/wordpress/

bullet physicsを少しだけ編集しましたので明記します。
btSimulationIslandManageMt.cppの
btSimulationIslandManagerMt::Island* btSimulationIslandManagerMt::getIsland( int id )
{
関数の先頭に以下の3行を加えました。

if (id < 0){

	return NULL;//!!!!!!!!!!

}


CommonRigidBodyMTBase.cppも少し変えました。

static int gNumIslands = 0;
を

int gNumIslands = 0;
にしました。

static bool gMultithreadedWorld = false;
を
static bool gMultithreadedWorld = true;
にしました。

static btScalar gSliderNumThreads = 1.0f; 
を
btScalar gSliderNumThreads = 64.0f; 
にしました。

コンストラクタの
m_multithreadedWorld = false;
m_multithreadCapable = false;
を
m_multithreadedWorld = true;m_multithreadCapable = true;
にしました。


createEmptyDynamicsWorld関数に
gTaskMgr.setNumThreads(int(gSliderNumThreads));
を加えました。

createDefaultParameters()関数の中身を全てコメントアウトしました。



（2017/06/10コミット分から2017/12/17まではbullet physicsのバージョンは2.85でした。）
（2017/06/10以前のコミットで使用していたbullet physicsのバージョンは2.80です。）


2017/12/17コミット分からIntel TBBを使用しました。
https://www.threadingbuildingblocks.org/
(何か問題が起きたとしてもIntelは責任を負いません。)
TBB2018のVC 14(Visual Studio 2015)用のビルド済みライブラリを使用しました。



######メタセコイアのFBX読み込み########
メタセコイアでFBX出力する場合は
出力する前に面の三角化を実行してください。

メタセコイアでボーンを入れたらボーンダイアログでポーズ出力メニューを実行してください。

メタセコイアのFBX出力オプションで
スムージンググループをオフにしてください。


＃＃＃　外部ライブラリのソースの場所

まめばけ３Dでは
外部ライブラリのソースをプロジェクトに追加しています。

その部分について少しメモしておきます。

まずBullet Physicsのソース
AMDのサイトからBullet Physics 2.86をダウンロードします。
解凍します。
data, docs, examples, Extras, src, testのフォルダをまめばけ３Dのdisp4のフォルダの中にコピーします。
そしてsrcのフォルダの名前をBTSRCにリネームします。




DXUTについて　(2023/02/22)
	DXUTはGitHubのMicrosoftのDXUT11をほぼそのまま使用できるようになりました

	変更部分はDXUT.cppのDXUTRender3DEnvironment()
		fps制御のための記述を追加

		    if(g_VSync == true)
		    {
		        double oneframetime = 0.010;
		        if (g_fpskind == 0)
		        {
		            oneframetime = 0.0020;
		        }
		        else if (g_fpskind == 1)
		        {
		            oneframetime = 0.010;
		        }
		        else if (g_fpskind == 2)
		        {
		            oneframetime = 1.0 / 60.0;
		        }
		        else if (g_fpskind == 3)
		        {
		            oneframetime = 1.0 / 30.0;
		        }
		        else if (g_fpskind == 4)
		        {
		            oneframetime = 1.0 / 15.0;
		        }

		        GetDXUTState().SetOverrideConstantFrameTime(true);
		        GetDXUTState().SetOverrideConstantTimePerFrame(oneframetime);
		        DXUTSetConstantFrameTime(false, oneframetime);
		    }
		    else
		    {
		        GetDXUTState().SetOverrideConstantFrameTime(false);
		        GetDXUTState().SetOverrideConstantTimePerFrame(fElapsedTime);
		        DXUTSetConstantFrameTime(false, fElapsedTime);
		    }


		Present部分は次のように

		    if (g_VSync) {
		        hr = pSwapChain->Present(SyncInterval, dwFlags);
		    }
		    else {
		        hr = pSwapChain->Present(0, 0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		    }



DXUTについて　(古いバージョンにおける記述)
	次にDXUTのソースについて
	DXUTのソースはDirectXのSDK (Microsoft DirectX SDK (June 2010))に入っています。
	DXUTのフォルダ（中にCoreフォルダとOptionalフォルダが入っている）をまめばけ３Dのdisp4のフォルダにコピーします。

	DXUTのD3DXの部分をChaVecCalcで置き換えます。
	[ DXUTの中の必要なヘッダファイルでChaVecCalc.hをインクルードします。 ]

	手順としては
	以下の文字列置き換えをします。
	D3DXVECTOR3 --> ChaVector3
	D3DXMATRIX --> ChaMatrix
	D3DXVec3 --> ChaVector3
	ChaMatrixA16 --> ChaMatrix
	D3DXQUATERNION --> CQuaternion
	LH( --> RH(

	ChaMatrixMultiplyの部分を　A = B * C;形式に書き換えます。
	ChaVector3Subtractの部分を　A = B - C;形式に書き換えます。

	CQuaternionRotationMatrixの部分は
	quaternion.RotationMatrix(matrix);形式に書き換えます。

	DXUTmisc.cpp のDXUTTrace関数のreturn文のところをreturn hr;にします。

	DXUT.cppのCreate関数の
	m_state.m_OverrideForceVsyncの値を= 1にします。


	そしてどうしてもD3DXが必要な部分はそのようにキャストします。
	例えばSetMatrix((D3DXMATRIX*)&srcchamatrix);のようにキャストします。
	(const D3DXMATRIX*)などのようにconstが必要な場所もありますがそれはVisual Studioのエラーを読んで対応してください。
	メンバ変数が同じなのでキャストしてもちゃんと動きます。


	2018/01/22にDXUTgui.cppを調整しました。
	メインう移動のスライダーとチェックボックスをクリックするとその範囲外をクリックするまで
	GUIが表示されない不具合を解消するための編集をしました。
	DXUTgui.cppのCDXUTSlider::HandleMouseとCDXUTCheckBox::HandleMouseの
	以下の2行をコメントアウトしました。
	if(!m_HasFocus)
		m_pDialog->RequestFocus(this);
	上記２行をコメントアウトすることでGUIが消える症状がなくなりました。



	2018/03/05
	DXUT.cppのDXUTRender3DEnvironment10()のPresentの箇所を
	hr = pSwapChain->Present(1, dwFlags);
	にしました。





＃＃＃
newer log is at UpdateLog.txt


2021/06/07
	bvh2FBX Batch Menu.
	MotionCacheFile Batch Menu.
	Debug ReuseOfBlockAlocMem.
	Add FbxComment to Lmt File.

2021/05/11_1
	Loading time is about three times faster than prev version.
	Add fbx animation loading cache files named *.fbx.anim*.egp.
	If laoding fbx file's rev is rev. 2.3 and comment of fbxinfo is match with cache file, cachefile will be loaded. 
	For fbx file's rev becomming rev.2.3, it is needed that project file is saved from file menu.
	For creating cache file, it is needed that rev2.3 fbx file is loaded once at least.
	If fbx file is changed, it is needed that cachefile is remaked. 

	読み込みが以前と比べて約３倍速くなった。
	FBXアニメーション読み込みのキャッシュファイルを追加。キャッシュファイルの名前はFBXファイルの名前の後に.anim*.egpを付けたもの
	rev 2.3のFBXファイルが読み込まれFBXINFOのコメントがキャッシュファイルと一致した時、キャッシュファイルが読み込まれる
	FBXのrevをrev. 2.3にするためにはfileメニューでプロジェクトファイルを保存する必要がある
	キャッシュファイルを作成するためには、rev. 2.3のfbxファイルを少なくとも一度読み込む必要がある
	FBXファイルの内容が更新された場合、キャッシュファイルも作り直す必要がある


2021/05/10_1
	FBXのマルチスレッド読み込みはSDKに任せることに（そのままでも複数CPUが動いていました）。
	アニメーション読み込み時、メッシュの行列とスケルトンの行列読み込み関数を別にした。
	EvaluateGlobalTransformはEvaluatorから呼び出すように。

	MeshTransform at GetMeshAnimReq, SkeletonTransform at GetFBXAnimReq.
	Use FbxAnimEvaluator.


2021/05/08_1
	マルチスレッド読み込みの修正（エラーにならないように）
	一番重い関数がわかったが、その関数のマルチスレッド化は難しいこともわかった。
	FBX SDKのMDとリンクした。

	Modify multiThread loading of FBX animation.
	Link Fbx md library.

2021/05/07_1
	FBXファイルのアニメーション読み込み部分を４スレッドのマルチスレッド化
	同期をとったので速くはならなかったが処理の負荷が分散した
	
	Four threads loading of FBX Animation.
	It is not faster than prev version, but Load distributed.

2021/05/06_1
	fbxファイルの読み込みをFbxStreamを使ってバッファから読み込むように修正
	しかし、読み込み速度のボトルネックではなかった（ほとんど速くならなかった）
	ブロックアロケート修正（前回最後尾データをリリースした場合の考慮）

	Use FbxStream on load FBX file.
	Modify block alloc of CMotionPoint, CBone, CRigidElem and EulKey.


2021/05/04_1
	file->bvh2fbxメニュー実行の際に落ちるようになっていたのを修正
	bvhファイルはMotionBuilder friendlyのものに対応することにした
	カーネギーメロン大学のbvh file : MotionBuilder friendlyで変換再生テストOK！！
	１つの表示オブジェクトに対する最大クラスター数を７０から２００に変更、ビデオカードのシェーダー定数の数の問題になり得るがしょうがない。

	Debug function of file->bvh2fbx menu.
	bvh2fbx function is tested ok using Motionbuilder-friendly BVH conversion release of 
	the Carnegie-Mellon University (CMU) Graphics Lab Motion Capture Database.
	Max Num of Clustor for a object is changed from 70 to 200, it may cause a problem of num of videocard's shader constants.


2021/05/03_1
	オープンプロジェクトファイル履歴を改良
	ウインドウタイトルにキャラクターのインデックスも表示

	Improve OpenProjectFileHistory.
	Display charactor index at WindowTitle too.

2021/05/01_3
　　　　動画【物理シミュのベイク】MameBake3Dで物理をベイク【フレーム範囲指定OK】
　　　　https://youtu.be/E-w6tpksu4I

2021/05/01_2
	物理シミュレーション中にアンドゥをすると形が崩れるので
	アンドゥーの際に物理シミュレーションを自動で止めるようにしました。

	Call StopBt OnFrameUndo.

2021/05/01_1
	物理シミュレーションをモーションに書き出せるようにしました
	メインウインドウのBullet PhysicsプレートをクリックしてGUIを表示し、BT RECボタンを押すと再生保存します。保存には少し時間がかかります。
	
	動画のお知らせ
　　　　【物理IK】MameBake3Dの物理IKが保存可能に【位置コンストレイント】
　　　　https://youtu.be/EOqRf35o7u8

	物理シミュレーションの保存の動画は後ほど撮る予定

	Simulation result of bullet physics is saved on click BT REC button.
	new movie : saving of Physics IK  https://youtu.be/EOqRf35o7u8

	moview about Simulation result of bullet physics will be up later.
	
2021/04/30_2
	物理IKのデバッグ。いつもの式を使うようにした。
	物理IKで手が変にならないように
	手（名前にHandあるいはElbow_branchが含まれるジョイントより子供のジョイント）自動でキネマティックフラグをセット。
	Debug CModel::ApplyPhysIkRecReq using ordinary formula. Auto Hand KinematicFlag.

2021/04/30_1
	メインウインドウのタイトルにカレントのRigidElemファイル(*.ref)名も表示しました
	設定したはずの物理の動きと異なる場合にはカレントのrefファイルが異なることを疑います。
	refファイルはメインメニューの剛体設定切り替え(select rigid)メニューで選択します。

	Current RigidElemFileName(*.ref) is displayed at MainWindowTitle.
	If behavior of physics simulation is diffrent form what you expected, you will doubt current ref selection.
	For selecting current ref file, there is the "select rigid" Menu at MainWindowMenu.

2021/04/29_1
	物理IKの数式にコメントをたくさんつけました。
	Comment politely at PhysicsIK formula of CModel::ApplyPhysIkRecReq.

2021/04/27_1
	日記のページで、物理IKの操作手順や注意点を図解しました
	https://moo-ochakkolab.ssl-lolipop.jp/diary_win.html#PhysicsIK_20210426

2021/04/26_2
	Debug a problem that first time of PhysRot and PhysMv is fail.
	For keep hand figure, it is needed to set Hand KinematicONtoLower with RightClickMenu.

	初回の物理IKが失敗する問題をデバッグ。
	物理IKの間、手の形を保つためには、手首を右クリックしてメニューを出しKinematicONtoLowerJoitsを設定する必要有り。

2021/04/26_1
	Result of PhysicsRot and PhysicsMv is applied to selected frame range.
	Editiing data of PhysicsIK is started to record when PhysicsRot or PhysicsMv button is pushed and Joint is dragged.
	Recording of PhysicsIK is finished and Apply result to selected frame range when mouse LButtonUp.
	Apply rate is enable when PhysicsIK too.

	物理IKの結果が選択フレームに適用されるようになった。
	PhysRotまたはPhysMvボタンを押してからジョイントをドラッグすると編集内容が記録され始める。
	編集内容の記録はマウス左ボタンをアップした時に終了し、結果が選択フレームに適用される。
	選択範囲のブラシの山の頂点の位置を決めるためのApplyRateスライダーは物理IKのときも有効に働く。

 
2021/04/25_2
	ApplyFrameNumber of PhysicsRot and PhysicsMv is free selectable, not only Zero Frame.
	(Currently result of PhysicsIK is applied to a frame, but futurely result will be applied to selected range.)

	Set Visible of Manipulator False when Visible of Bone is FALSE.

2021/04/25_1
	Current CharactorFileName and MotionName is written at MainWindowTitle.

2021/04/24_3
	When (L2 or R2) and R1 button is pushed, change Motion to other loaded one.

2021/04/24_2
	If (L2 or R2) and L1 buttons is pushed, change charactor to other loaded one.

2021/04/24_1
	disp-->model panel menu is enable.
	disp-->object panel menu is enable.
	ModelPanel and ObjectPanel with GamePad OK.

2021/04/23_1
	Fix Reuse bone problem on deletion models and loading models.

2021/04/22_1
	GamePad is enable on SHGetSpacaialFolderLocation too.
	Title of MessageBox is one of error!!! or warning!!! or check!!!.
 
2021/04/20_2
	Replace GetSaveFileName to GetOpenFileName, GamePad OK!


2021/04/20_1
	Operation with GamePad is enable during MessageBox and GetOpenFileName and GetSaveFileName too.

	
2021/04/18_2
	Fix problems that timelineWnd close if click timelineWnd before loading model.
	Fix problems that timelineWnd is resized Unintentionally.

	Enable all buttons of PlayerButtons of LongTimeLineWnd.
	(history buttons of selected frames)

	If GamePad is not connected, mouse cursor pos don't jump to GUI pos.

		
2021/04/18_1
	Adjust GUI placement of 3DWND.
	
	Set default value of PhysEditRate from 0.1 to 1.0.
	(PhysRot and PhysMv works effectively.)
		
	Add PhysicsIkStop button.

2021/04/17_1
	Option button at joint pos of 3dwnd as Right Click Menu.
	
	Can push buttons in a group box.

	All? of Dialogs exclude GetFileNameDialog operation can be enable with GamePad.(perhaps)

2021/04/16_3
	Refactoring source file.
		Use common source at AngleLimitDlg and other WindowsDialogs.

2021/04/16_2
	pragma lib at cpp not at h.
	MouseMoveMessage of AngleLimitDlg. 	

2021/04/16_1
	Fix problem that cursor moves to 3DWND on drop down Main Menu.

2021/04/15_1
	Operation with GamePad
		HiLite Window on SelChangeGUIPlate.		
		Put cursor pos to ctrl pos of HiLite window on SelChangePlate.
			
2021/04/13_2
	Explanation of operation with Sony Game Pad DualSence.
		MameBake3D/ReadMe/MameBake3D_OperationUsingSonyGamePadDualSence.png

2021/04/13_1
	Ring GUI selecting using CrossButtons.

	Move Mouse Cursor using Analog L stick.

	Num of open history is changed from 3 to 5.
 

2021/04/12
	Operation with GamePad
		Can Select DropDownMenuItem from GUI buttons with GamePad.
		Can push Windows Dialogs form GUI buttons with GamePad.
		Can push OrgWindows from GUI buttons with GamePad.

2021/04/11_3
	MainMenu with GamePad
		AimBar moves under Main Menu Bar :  R stick.
		And then DropDown Menu is displayed : O button.
		And then select menu item : L stick and 0 button.
	
2021/04/11_2
	補足：Undo, Redoモーション編集に関するUndo, Redo。

	FileのOpenメニューのプロジェクトオープン履歴に重複するものを表示しないように。

	Undo and Redo is concerned with editting of motions.
	Exclude same project entries of Open Project Dialog.
	

2021/04/11_1
	ゲームパッド対応の続き
		Undo, Redoが動かなくなっていたのを修正
	
		Keyboard
			Ctrl + Z : Undo
			Ctrl + Shift + Z : Redo
		GamePad
			L2 + X button : Undo
			R3 + X button : Redo

		画面外へドラッグするとドラッグが中断する問題を再び修正


2021/04/10
	ゲームパッド対応の続き
		プロジェクト読み込みに成功するたびにTempフォルダにプロジェクトパスを記録
			FileのOpenメニューから出すダイアログをゲームパッドで押せるようにした
			ゲームパッドからプロジェクトを読み込めるようにFileのOpenメニューに履歴パスのラジオボタンを配置

		Rスティック上下で現在のメインメニューバー項目、左右でひとつ左または一つ右のメインメニューバー項目


		L2,R2のアクセル機能はLスティックのマウス移動を加速。L2とR2の両方を押しているとさらに加速。


		Open Project History is saved at temp directory.
		It is enable that buttons of open file dialog is pushed using a GamePad.
		Current menu is selected with up and down of R stick.
		Accel function of L2 or R2 is double effective by both is pushed. 	

2021/04/07_1
	ゲームパッド対応の続き
		Rスティック左右でメインウインドウのメニューバー選択
			スティック左右でドロップダウンを出して、Xボタンでキャンセルしてを繰り返して左右へと進む
			ドロップダウンが出たらLスティックでマウス位置移動し〇ボタンで決定

		つまり
		メインメニューバードロップダウン：Rスティック
		ドロップダウンキャンセル：Xボタン
		マウスカーソル移動：Lスティック
		メニュー選択決定：〇ボタン

	DropDown main menu : R stick.
	Cancel DropDown : X button.
	Move Mouse Cursor : L stick.
	Enter button : 〇 button.


2021/04/05_3
	動画【MameBake3D】モーションツールをゲームパッドで動かしてみた【DualSence】
　	https://youtu.be/AdJPZ3Uv6-c

2021/04/05_2
	L3, R3ボタンを押したときの機能の改良
		設定ウインドウ上でもボタンを押している間半透明アニメをするようにした
		設定ウインドウ上でも背景色を透過して描画した

	UFO Transparent and AlphaBlendAnimation at mouse pos while L3 or R3 is pushed. 

2021/04/05_1
	L3, R3ボタン(アナログスティックを)を押したときにマウス位置にUFOを表示しました
	設定用ウインドウにも３Dウインドウにも出ます

	For not lose sight of mouse pos, UFO appear at window on L3 or R3 button.
	L3(R3) button is pushed when L(R) analog stick is pushed.
 

2021/04/04
	R1ボタンを１回押すと３Dウインドウのジョイント位置へジャンプ、
	もう一回押すとジョイント位置へジャンプする前の位置にジャンプ

	L3ボタン、R3ボタンを押している間、マウス位置にウェイティングカーソルを表示
	マウスカーソルを見失わないための機能

	L1ボタンを何回か押して階層エディタ選択した後の十字ボタンによるジョイント階層選択時にマウスも追従

	Mouse cursor is jumped to joint pos at 3dwindow on push R1 button,
	it jumped to prev pos on one more push R1 button.

	If L3 or R3 button is push, waiting cursor is displayed, for lose sight of mouse position.

	Mouse position is moved to selecting tree elem too.(by L1 button and corss buttons)

2021/04/03_2
	ゲームパッドR1ボタンのジョイント位置修正
	Fix mouse cursor pos problem on R1 button.

2021/04/03_1
	ゲームパッド対応続き
		R1ボタンで３Dウインドウを選択して選択中のジョイントの位置にマウス位置ジャンプ

	On R1 button, select 3d window and jump to selected joint pos.


2021/04/02
	カメラスプライトのドラッグ処理の修正
		真上と真下でくるくる回る問題を修正

	ゲームパッドをつないだ状態でマウスでドラッグすると
	マウスが３Dウインドウの外に出たときにキャプチャーが解除される問題を修正

	ゲームパッドのL1ボタンで３Dウインドウを選択し（ウインドウ右上に黄色ランプ点灯）
	矢印ボタンを押してカメラスプライトにもジャンプできるように修正
	カメラスプライトからGUIへのジャンプは十字左右ボタン
	GUIプレートの種類をまたぐ場合のマウスジャンプも十字左右ボタン
	ただし□ボタンと〇ボタンでGUIを表示している状態である必要有り。


2021/04/01_1
	ゲームパッド（Sony DualSence, Sony DualShock4）対応
		L1でウインドウを選択後
			十字ボタンでコントロール選択
			〇ボタンで決定
			〇ボタン＋Lスティックでドラッグ

		After selecting a window by L1 button,
			select control by cross button,
			enter by 〇 button,
			drag by 〇 button + L stick.

	
2021/03/30_1
	マウスキャプチャーに関する修正

	十字キーでスライダーを選択した際のマウスポインタの位置の修正

	リファクタリング
		スプライトの配列アクセスにかかわる部分をenumに置き換え

	Modify operation of mouse capturing.
	Fix problem of mouse position on selectiong a slider by cross button.
	Refactoring source code concerned with access of sprite array.


2021/03/29_2
	マウスドラッグ改善
		ゲームパッド使用時においても
		ドラッグ中にマウスがウインドウ外に出た場合にドラッグ効果が続くようにしました

	Improve mouse drag operations.
	In case of using GamePad, effect of mouse drag is continued even if mouse is out of window.
 

2021/03/29_1
	Sony PlayStation用のゲームパッドに対応(中)
		PS5用のDualSenceとPS4用のDualShock4に対応(中)
		
		カエルボタン押下：△ボタン
		プレートメニュー移動；□ボタン
		プレートメニュー選択決定：〇ボタン
		アクセル：L2,R2		

		決定：〇ボタン		
			GUIを押すことが出来ます

		子ウインドウ選択：L1ボタン
			子ウインドウを選択した状態でGUIコントロールを選択：十字キーで移動して〇ボタンで決定

		マウスカーソル移動：Lアナログスティック

		階層エディタの操作(L1ボタンで階層エディタ選択後)
			十字ボタンでジョイントを選択
			親ジョイント選択：上ボタン
			子供ジョイント選択：下ボタン
			シスタージョイント選択：右ボタン
			ブラザージョイント選択：左ボタン
			親階層のブランチジョイント選択：L2あるいはR2＋上ボタン
			子階層のブランチジョイント選択：L2あるいはR2＋下ボタン
			
		ドラッグ：〇ボタンを押しながらLアナログスティック
			ジョイントのドラッグ
			スプライトのドラッグ
			スライダーのドラッグ
			コンボボックスのドラッグ
			複数フレーム選択
			などが出来ます


		注意：ゲームパッドはUSB接続して使用する必要があります。

		注意：ゲームパッド操作用のライブラリはGitHubのヒロッチさんのオープンソースを編集して使いました。
		　　　ヒロッチさんのゲームパッドライブラリのライセンスはReadMe/DS4HidInput_LICENSE.txtに置きました。

		注意：現在はまだ対応中の段階です。

		注意：ポップアップメニューの選択は出来ますが決定はまだできません。
		　　　ポップアップメニュー選択決定はキーボードのエンターキーを押してください。		
				
		注意：〇ボタンを一度押してもGUIを押せない場合があります。その場合にはもう一回〇ボタンを押してください。

		注意：ウインドウ右上の３つのボタン（最小化、最大化、終了）はマウスで押してください。

		注意：終了できなくなった場合にはタスクマネージャでFBXアニメーションツールを選択して終了してください。

		
2021/03/24_1
	Change number of LogLines of CInfoWindow from 500,000 to 6,000.
	Change LogLines of CInfoWindow to ring buffer.
	CInfoWindowのログ用メモリの行数を500,000行から6,000行のリングバッファに変更

	Change PICKRANGE from 6 to 16.
	Change pickmode to relative mode.
	ピックの誤差許容範囲を６ピクセルから１６ピクセルに変更。
 	ピックモードをマウス位置相対モードに変更。

2021/03/23_2
	Use memory blocks on allocating CRigidElem for cache hit.

2021/03/23_1
	Use memory blocks on allocating CBone for cache hit.

2021/03/22_2
	Fix loading ref files and CreateRigidElem.

2021/03/22_1
	Fix CModel::CalcBoneEul(-1).

	Modified loading ref files and CreateRigidElem.

				
2021/03/21
	Use memory blocks on allocating CMotionPoint and EulKey.

	Fix EulerGraph problem that euler is not displayed at cases of loading multi characters multi motions.

2021/03/19_3
	Bullet Physics ver2.89のメモリリーク修正
	Fix Memory Leak of Bullet Physics ver 2.89.

		//btDiscreteDynamicsWorldMt.cpp
		btDiscreteDynamicsWorldMt::~btDiscreteDynamicsWorldMt()
		{
			//2021/03/19 add
			if (m_constraintSolverMt) {
				delete m_constraintSolverMt;
				m_constraintSolverMt = 0;
			}
		}


		//CommonRigidBodyMTBase.cpp
		//2021/03/19 add
		virtual ~btTaskSchedulerManager() { shutdown(); };



2021/03/19_2
	Bullet Physics Update ver2.86 to ver 2.89.
	Bullet Physics with PPL Micosoft MultiTheadLibrary.
		Becaouse bullet physics 2 series (old version) is not treated with new Intel oneAPI.

	Develop Env(2021/03/19_2)
		FBXSDKは2020.2(VC2019)
		Microsoft MultiThreadLibrary PPL.
		DirectX11 (Windows10 SDK or VisualStudioからインストール)
		githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
		githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
		githubのbullet physics ver2.89(http://bulletphysics.org/wordpress/)
		VisualStudio2019Pro(VisualStudioSubscription)
		MayaIndie(Subscription FBXの確認はMayaでする)

2021/03/19_1
	Replace std::tr1::function to std::function.

2021/03/18
	Each physics paramster file each character, select using rigid menu and rgd menu after select model.

	Replace Elasticity to Restitution as in bullet physics.


2021/03/16_3
	Init m_motionkey[0] On Constructing.

	Select ref by rigid menu of MainWindowMenuBar before Simulating Bullet Physics.

2021/03/16_2
	Result of checkking about loading FBX having multiple AnimStack, 
	read FBX file and open same FBX by Animation Time Editor using Synthetic option.

	It is success to read FBX sample of today by Maya2020.

2021/03/16_1
	Fix problem about saving FBX file which is made from bvh file.
	
	Update Sample File in Media Directory.
		Multible models, multiple motions.
	
	Select editable model by model menu, select editable motion by motion menu of MameBake3D.

	Now chekking about operations of CharacterSet and AnimationLayer of Maya2020.
		Now ?????.

2021/03/15_1
	Fix problem about loading FBX File which include Multiple motions.
		Fix motid problem, motid range is form 1 to motnum.

2021/03/12
	Fix problem about Normal of FBX2020 saved with Maya2020.
		Treatment of Normal Mapping Mode of FBX 2020.

	Before Loading by Maya FBX File which is saved by MameBake3D, run next MEL script.
		FBXProperty "Import|IncludeGrp|Geometry|OverrideNormalsLock" -v 1
 
2021/03/10_1
	Keep Rig target On Click othoer. Rig Mode is OFF when Red Rig Sprite is clicked.	

2021/03/09_3
	On Selecting RightClickRigMenu, RigSettingWindow is set to SideWindow.
	At First, PlaceFolderWindow is set to SideWindow.	

2021/03/09_2
	Rig Sprite is displayed when RightClickRigExecutionMenu is selected too.

2021/03/09_1
	Display GUIText in English.


2021/03/08_1
	Mediaフォルダの中のサンプルデータの修正
		リグ設定ファイル　yuri_12_1_walk.fbx.rig　の修正
		Rigのプログラムには問題が無かったのだが、なぜか設定ファイルが間違っていた
			手首、足首に操作用のリグ有
			リグ実行モードから抜けるには、メインウインドウの向かって右上の水色の円のマークをクリック

2021/03/06_1
	プレートメニューのコードをRefactoring

	プレートメニューのRetargetと同じ階層にLimitEulerを追加

		
2021/03/05_1
	メモリリークに関する修正
		blendstateのリリースし忘れ修正

		DirectX11のDXUT(MicrosoftのOSSを使用)に関して（D3D11のデバイスなどに関して）は、
		differred destroyという遅延解放処理のためにリファレンスカウンタが残っているようにみえることがあるとのこと

2021/03/04_2
	VisualStudio2019Pro対応
		開発環境(2021/03/04時点)を書きます。
		FBXSDKは2020.2(VC2019)
		Intel OneAPI Base Toolkit v.2021のtbb(Intelのサイトに登録してダウンロードインストール)
		DirectX11 (Windows10 SDK or VisualStudioからインストール)
		githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
		githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
		VisualStudio2019Pro(VisualStudioSubscription)
		MayaIndie(Subscription FBXの確認はMayaでする)
	
2021/03/04_1
	リターゲットウインドウをサイドウインドウに配置
		カエルボタンを１回押すと物理パラメータメニュー
		カエルボタンをさらにもう一回押すとリターゲットメニュー
		カエルボタンをもう一回押すとメインGUIオンオフメニュー

2021/02/26_1
	サンプルの物理が以前と同じように動くように修正
		DXUTのfps制御をVSYNCに設定
		previewflag == 4のときに決め打ちしていたパラメータを設定ファイルに適用して保存
			Media/TestData_20210226_1

2021/02/24_2
	動画【カエルボタン】プレートメニューでGUIを切り替え【プレートメニュー】
	https://youtu.be/GW0OVv7_0SA

2021/02/24_1
	カエル(Frog)のスプライトをクリックすると
		メインウインドウのGUIコントロールメニューとサイドウインドウ選択メニューが切り替わります
		サイドウインドウ選択メニューには以下の４種類があります
			Rigid, Impulse, Ground, Damp
		サイドウインドウ選択メニューはモデルデータを読み込んでボーンをクリックして選択しているときのみ機能します
		サイドウインドウの上部のボタンから表示ウインドウを切り替えることも可能


	Bullet Physicsシミュレーション時のパラメータの決め打ち解除について
		開発の途中から
		適切なパラメータ値を探すために
		previewflag 5, 6の再生時にはパラメータを決め打ちして再生していた

		そのためRigidParamdDialgで設定した値は
		物理シミュレーション時には上書きされていた

		今回、RigidParamsDialogを前面に出すことになって
		previewflag 5 の再生時にはパラメータを決め打ちを止めた
		よって挙動が変わるかもしれない
	
		決め打ちの部分は
		該当プログラム個所に
		//決め打ち
		というコメントをしておいたので
		決め打ちパラメータ値を知りたい場合にはみてください

2021/02/22_2
	動画【MameBake3D】煩雑なGUIをスイッチでオンオフ【プレートスイッチ】
	https://youtu.be/DhcikN88D5A

2021/02/22
	３Dウインドウの下部にSpritePlateSwitchを５枚表示
		SpritePlateSwitchをクリックすることにより３Dウインドウ内のGUIのオンオフ

2021/02/21_1
	Save-->Projectメニューにおいて参照ボタンを押すと固まる不具合に対応
		新しいUIを指定するフラグを取り除くと動くようになった

2021/02/17_2
	ddsファイルに対応

2021/02/17_1
	DirectX11対応版にファイルをaddし忘れていたのを追加
		Media/MameMedia/bonecircle.png
	補足説明
		DXUTのUIディレクトリは実行時にはexeのある場所へコピーする必要有
		ControlFlowGuardはリリースビルドのみ

2021/02/16
	DirectX11対応
	FBX SDK 2020.0.1対応

	開発環境(2021/02/16時点)を書きます。
	FBXSDKは2020.0.1(Debugビルドも出来た)
	DirectX11 (Windows10 SDK or VisualStudioからインストール)
	githubのMicrosoftのFX11(DirectX11でEffectを使う)を利用
	githubのMicrosoftのDXUT(DXUTのDirectX11版のオープンソース)を利用
	VisualStudio2015Pro(VisualStudioSubscription)
	MayaIndie(Subscription FBXの確認はMayaでする)

2021/02/11
	ControlFlowGuardをGuardに設定してビルドしました

2021/02/06
	動画【MameBake3D】モーションにアクセントを【モーションブラシ】
	https://youtu.be/WqB03hMqbgc

2021/02/03_1
	オイラーグラフのペアレントオイラーのチェックボックスをデフォルトでオンにしました
	一階層のIK操作が一番多い操作だと思うので、チェックボックスをチェックする手間と思い違いが減ると思います。

2021/02/02_2
	動画　
	【MameBake3D】モーションをブラシで盛る！！【モーションブラシ】
	https://youtu.be/rZFTNVLYXPU
	（公開終了しました）


2021/02/02_1
	Rename EditScale to MotinBrush.
	エディットスケールという機能名をモーションブラシという名前に変更しました。
	モーションブラシという名前にすることで
	MameBake3Dというソフトの仕組みを直感的に理解可能だと思われます。
	相対的なモーション編集というよりもブラシで盛るという方が直感的。
	
	分かりやすい言葉がみつかったので
	後ほど、この言葉でMameBake3Dの使い方の動画を作ってみます。


2021/01/30_1
	EditScaleの修正
		オイラー角がすべて０度の場合にEditScale形状がフラットになってしまう不具合を修正

2021/01/28_1
	EditScaleの修正

	EditScaleの種類に"cos(x^2+PI)山"と"矩形山"を追加

2021/01/27_3
	【MameBake3D】相対的編集とはどういうことかについて【PseudoLocal Edit】 

	複数フレームに対する編集だから
	モデルの向きが変わったら
	編集結果がおかしくなるんじゃないかという
	疑問に応える動画

	PseudoLocalと呼ぶ技術で解決しています


	https://youtu.be/iyQS7ZPEQLk
	（公開終了しました）


2021/01/27_2
	エディットスケールの動画　https://youtu.be/uqMUoQo7egE
	（公開終了しました）


2021/01/27_1
	EditScaleのバージョンアップ
		姿勢適用位置スライダーに対応
		EditScaleの形状をコンボボックスで"線形山"(Mountain of Linear)と"Cos(x+PI)山"(Mountain of Cos)から選べるようにしました

	IKの効果をみるにはドラッグしているジョイントの親ジョイントのオイラー角をみます
		オイラーグラフの"ParentEuler"チェックボックスにチェックを入れると
		ジョイントの選択はそのままで選択したジョイントの親のオイラー角を表示します

2021/01/26
	FBXSDKを2018.1.1に戻しました(Debugビルドでリンクエラーが出るので)

	複数フレーム選択時にオイラーグラフ部分に編集倍率(EditScale)を表示しました
	
	EditScaleとはメインウインドウにおけるジョイントのドラッグアンドドロップの効果を
	どのフレームにどれだけ適用するかをあらわします
	まずは一番単純な線形の山の形のEditScxaleにしました
	山が高いフレーム部分にはたくさん効果を与え、山が小さい部分に対しては効果を小さくします。

	EditScaleの効果が０の部分は元の姿勢になります。
	つまり相対的な編集効果です。

	EditScaleの形状に関しては
	スプライン曲線や矩形など複数用意して選べるようにする予定です。

2020/12/24
	GitHub 2021/08/13にパスワード廃止のお知らせを受けて
	sshでpushのテスト

2020/12/17その１
	FBXSDKのバージョンアップ
		Maya2020を使用することになったのでFBXSDKも2020.0.1にバージョンアップ
		

2020/11/08その１
	FBX出力の修正
	影響度出力の修正
		Maya2018で読み込んだ時に余計なJointSetが出来ないようにした
		ダミーメッシュの頂点数をきちんと計算してセットした
	AppleのRealityConverterでusdzに変換しても形が崩れなくなったしモーションも正常
	AppleのRealityConverterでusdzに変換するとテクスチャ情報が無くなったようにみえた

2020/10/31その２
	FBX出力の修正
	LinkMeshToSkeletonReqの修正


2020/10/31その１
	FBX出力の修正
	RokDeBone2-->MameBake3D-->MameBake3Dという経路と
	RokDeBone2-->MameBake3D-->Maya2018という経路で確認
	Maya2018でエラーも警告も出ずに正常に読み込めることを確認

	ただし現在、(Joint名)JointSetという名前のオブジェクトが多数出来る症状が出ている
	AppleのReality Converter(Beta)でusdzに変換するためには
	Maya2018で(Joint名)JointSetをすべて削除する必要があった

	XYZローテーションオーダーにおけるCBone::ModifyEulerXYZを新規

	サンプルとして付けているFBXファイルはまだ2013のままであり
	MameBake3Dで保存しなおさないと修正は適用されない

2020/10/21
	FBX入出力テストの続き
	2020/10/19でFBX出力時にRootNodeを新規には作成しないようにしたが
	同梱のyuriのデータの入出力で不具合が生じたので
	RootNodeの扱いを元に戻した


2020/10/20
	FBXの受け渡し順として
	RokDeBone2-->Maya2018-->MameBake3D-->MameBake3D
	という経路と
	RokDeBone2-->Maya2018-->MameBake3D-->Maya2018
	という経路において
	FBXが正しく受け渡せるようにデバッグした
	
	RokDeBone2とMameBake3Dと両方の修正が必要だった
	RokDeBone2では無駄な頂点バッファが出力されていて悪影響を与えることがあったのを修正
	MameBake3DではインデックスとUVの読み込み時と書き出し時の処理を見直した
	MameBake3Dでは影響度出力の際の参照と書き出しのインデックスがorgなのかoptなのかをはっきりさせた
	
	
	上記の２つの経路で正常にFBXの受け渡しが出来ることを確認
	ただし
	RokDeBone2-->Maya2018-->MameBake3D-->Maya2018
	の経路においては
	Maya上で法線の平均メニューを実行するなどしてスムージングをすることが必要だった
	
	

2020/10/19
	書き出したFBXをMayaでみると動かない部分が生じることがあった
	原因は頂点数を２倍にしていた時の無駄を取り除こうとして頂点数を最適化する部分
	
	つまりPolyMesh4::GetOrgPointNum()の不具合だった
	
	頂点数を２倍して増えた方の頂点に何かで影響度を付けてしまった後で最適化すると動かない
	という解釈に落ち着いた
	
	修正したがMayaで影響度を設定しなおさないと何かがおかしい場合は多いようだ
	FBXを入手したらまずはMayaのスクリプトか何かで影響度をバックアップするべき
	影響度に不具合が出たらバックアップから影響度を復元する
	
	（Mayaがあることが前提）
	
	現在のFBX書き出しのバージョンは再び２０１８に戻っています
	
	
	Mayaは趣味の人？に対しては安価で？提供されている
	（インディー契約の条件については以下のページ）
	https://area.autodesk.jp/product/maya-3ds-max-indie/
	
	

2020/10/18
	2020/07/18にFBX書き出しをAPI201300にした件をロールバックしてFBX2018にした


	忘れていたが2013と2018とでは形状データのインデックスの使い方が異なっていたのを思い出した
	バージョン指定だけではうまくいかないデータがあった
	（１つのオブジェクトに複数のマテリアルがある場合かもしれないが検証できていない）
	
	今後の方針としては
	書き出しはFBX2018に戻して
	Mayaのバージョンも2018以降を使用していく
	

2020/09/24
	剛体の表示について修正
		剛体設定ダイアログにて無効にした剛体は表示しないようにしました
		一番最初にFBXデータ読み込みから始める場合には
		剛体設定ダイアログで、すべての剛体を有効にするボタンを押してから、必要ない剛体の有効チェックをはずします

	ビルド時の注意
		現在、静的リンクライブラリー（MameBake3DLib.lib）とMameBake3D本体（MameBake3D.exe）のプロジェクトが１つのソリューション内に存在します。
		古いMameBake3DLib.libがアプリケーションパス（MameBake3Dフォルダー）に存在すると
		ソースを更新してリビルドしても古いMameBake3DLib.libとリンクしてしまう現象を確認しました。
		開発時にはアプリケーションパス（MameBake3Dフォルダー）にMameBake3DLib.libを置かないようにしましょう。

2020/07/18
	出力したFBXが正しいFBXかどうかはMayaで読み込まないと分からない！！
	持っているのはMaya2013のパーソナルCM永久ライセンス

	よって
	出力するFBXは2013にしました

	FBXSDKは2018.1を使用しているので2018.1までの読み込みは可能
	出力はFBX201300

	VisualStudioは2015

2020/07/15
	なんちゃってなワークステーションにて更新

	ウインドウサイズとGUI配置の調整中不具合を修正
		ライトスライダーをクリックすると落ちる不具合を修正

2018/11/22
	開発環境(2018/11/22時点)を書きます。
	Mac(ちくわ)とVM。
	BootCampではなくて、またVMでやることにしました。
	バージョンアップしたところVMでのバックバッファのずれが直っていました。
	垂直同期もほぼ出来ているようです。
	Ctrl + 1キーを押せば物理シミュレーション時にも60fpsで動きます。
	
	物理IKの検証と修正のつづき
		「複数の」位置コンストレイントを設定した状態で
		無理な姿勢にドラッグしようとすると破綻する不具合があることが判明
		今後修正していく予定		

2018/11/07_1
	物理IKの検証と修正のつづき
		Kinematic設定時の物理IKのデバッグ
		質量ゼロ（Mass0）設定時の物理IKのデバッグ
		質量ゼロとKinematicを両方同じ部分に設定した場合には質量ゼロを優先（フラグレベルで）

		Mass0による位置コンストレイントの調整中。

		物理IKは実験段階なのでワンドラッグお試し状態です
			破綻しても余白をクリックすることで元に戻ります
	引き続き検証する。

2018/10/30_1
	物理IKの検証と修正のつづき
		PhysicsMV（物理IKの移動機能）はそーっと動かさないと破綻する場合が多いことが分かった
		Edit Physics Rateスライダーの初期値を1.0から0.1に修正
		位置コンストレイントはまだ課題が多い状態

		物理IKは実験段階なのでワンドラッグお試し状態です
			破綻しても余白をクリックすることで元に戻ります
	引き続き検証する。
2018/10/27_1
　　動画　【物理IK】質量０とポスト処理による位置コンストレイント
　　　（1分くらいのテスト動画）
　　　https://youtu.be/q9z_DhzZQYU
2018/10/26_1
	物理IKの検証と修正のつづき
		物理IK時の位置補正計算の修正
		物理IK時に向きの補正計算も実装
		物理IK時に右クリックメニューのMass0のオンをすることで位置コンストレイントを実現
		物理なのでゆっくり動かさないと乱れやすい
		Kinematic指定しないでも指などの形が崩れなくなってきた
		未実装なのはMass0とKinematicの両方を同じボーンに設定した場合
	引き続き検証する。
2018/10/25_1
	オイラー角による角度制限の検証と修正のつづき
		物理IK時に関節に隙間が出来過ぎないようにしました。
			操作ボーンから下層方向と上層方向そしてトップダウンでPost処理することにより実現
		物理IK時に操作していないボーンが不自然にゆらゆらする現象が無くなりました。
		Mass0効果の修正が必要な状態
	引き続き検証する。
2018/10/23_1
	オイラー角による角度制限の検証と修正のつづき
		物理IK時のKinematic指定（右クリックメニューで指定）部分の動作の修正
			Kinematic指定部分は形が崩れないようにしました
			例えば、手の甲より下層に対してKinematicを指定してから腕を物理IKすればてのひらの形が崩れずに物理IK可能
	引き続き検証する。
2018/10/16_1
	オイラー角による角度制限の検証と修正のつづき
		Physics MVの効果を元の物理的な効果に戻しました
			体の中心をドラッグして手を固定して体を動かしたり出来ます
		Kinematicのオンオフのメニューを追加
			物理のオンオフとも言えます。
			操作ボーン以外のKinematicをオンにすると
				Physics Rotの際には動きません
				Physics MVの際にはドラッグしただけ「ゆらゆらせずに」一緒に動きます
			
		姿勢によっては物理IKがゆらゆらする現象というを軽減
		物理IK時に腕を動かすと指が大きくゆらゆらすることがありますが
			少しずつ動かしてドラッグ終了地点でマウスボタンを押したまま待っていると元の形状に戻ります

	引き続き検証する。
2018/10/15_1
	オイラー角による角度制限の検証と修正のつづき
		物理IKの開始時に姿勢が乱れないように修正。
	
	引き続き検証する。
2018/10/10_1
	オイラー角による角度制限の検証と修正のつづき
		位置コンストレイントの実装方法の変更の検討と試験中。
			ボーンの剛体のMass0(質量ゼロ)のオンとオフによって位置コンストレイント効果を得る。
			右クリックメニューにMass0関連メニュー多数追加。

			例えば、まず「すべてのジョイントのMass0 ON」メニューを選びます。
			次に操作したいジョイントを選択してから「このジョイントより下層のMass0 OFF」メニューを選びます。
			この状態で物理IKをするとMass0がオフのボーンだけが動きます。
			元に戻す場合は「すべてのジョイントのMass0 OFF」メニューを選びます。

			ボーンがぶれずにコンストレイント可能である。

	引き続き検証する。
2018/10/09_1
	オイラー角による角度制限の検証と修正のつづき
		検証中
		IK時にマニピュレータにマウスを当てた際にマニピュレータがハイライトしなくなっていたのを修正

	引き続き検証する。
2018/10/07
	オイラー角による角度制限の検証と修正のつづき
		CalcAxisMatXの修正（指の剛体が暴れにくいように修正）
			この修正は初期姿勢の修正と同じ意味です。
			古いデータは読み込み後、「編集・変換」->「ボーン軸をXに再計算」メニューで初期姿勢を再計算後
			Fileメニューからプロジェクトごと保存しなおしてください。
		InfoWindowの出力をマルチスレッド化
			通常IK時に出力が激しくて重くなっていたのを改善
		物理IKの修正
			ゼロフレーム以外での動作を改善
		位置コンストレイントの修正
			ゼロフレーム以外での動作を改善するために修正中。途中。
			右クリックメニューに多数追加
	引き続き検証する。
2018/09/29_1
	オイラー角による角度制限の検証と修正のつづき
		PhysicsRotAxisDeltaのデバッグ
2018/09/25_1
	オイラー角による角度制限の検証と修正のつづき
		オイラー角は通常IK時のものを利用して物理IKをした。
		通常IKと物理IKとでオイラー角の制限角度が統一された。
		InfoWindowは制限行数を50,000行に。
		通常IK時にもInfoWindowに出力。
		引き続き検証する。
2018/09/24_1
	オイラー角による角度制限の検証と修正のつづき
		InfoWindowアップデート
			古いデータ-->上。新しいデータ-->下。
			５０００行制限。
			ホイールでスクロール。
			最新行を表示している時にオートスクロール。
			終了時info_(date).txtに保存。	

2018/09/22_1
	オイラー角による角度制限の検証と修正のつづき
		InfoWindowを追加。
		物理IKのときの姿勢への適用候補オイラー角と可動かどうかをInfoWindowにテキスト表示。	

2018/09/16_1
	オイラー角による角度制限の検証と修正のつづき
		PhysicsRotAxisDeltaのとき、姿勢編集のときの座標変換と制限角度チェックの時の座標変換が異なっていたのを修正。

		さらに検証が必要。

2018/09/12_1
	オイラー角による角度制限の検証と修正のつづき
		物理IKのとき（PhysicsRot()とPhysicsRotAxisDelta()）の検証と修正

		さらに検証が必要。

	[追記]
	　　　GitHubでソースをゲットしてVisual Studioから実行すると
	　　　VisualStudioの出力タブに物理IKをしているときのオイラー角が表示されます。
	　　　なんで動かないの？という場合の答えになっているでしょう。たぶん。
	　　　制限角度は.lmtファイルです。
	　　　ちなみに出力タブの文字列の中の「eul」は「オイル」と読みます。
	　　　あ、それと念のため。物理IKは実験中の機能なので勇気がある方用です。
	
2018/09/09_1
	統一オイラー角による角度制限の検証と修正
		物理IKのとき（PhysicsRot()とPhysicsRotAxisDelta()）の検証と修正

		まだ途中。

2018/09/01
　　　まめばけ３Dのお知らせ
　　　　あれからほとんど検証作業はしていませんが、
　　　　ツインテールがマニピュレータのXYZそれぞれの軸で回転することくらいは確認済です。
　　　　FBXファイル名.lmtという名前のファイルに制限角度を記述するのですが
　　　　現在ツインテール部分は制限角度のY軸が±15度で他の軸が±５度になっています。
　　　　よってツインテールはキャラクターの正面に対して主に前後に揺れるようになっています。
　　　　Z軸を±１５度にして他の軸を±５度に変更すると横に揺れます。
　　　　通常IKのときと物理IKのときと物理シミュのときに同じように機能するようにしたつもりです。
　　　　物理シミュはbullet physicsの角度制限を利用したので
　　　　fpsと計算回数が同じ環境ならほぼ同様に角度制限が働くと思われます。
　　　　という状況。

2018/08/19_1
	物理のときと物理でないときのオイラー角を統一。
		内部のオイラー角データをXYZ観点順序に統一。
		マニピュレータの軸基準のオイラー角で角度制限（物理と物理でないとき両方）。
		*.lmtファイル（制限角度設定ファイル）も対応。

		ざっとテストしたところ、動かしたくないときでも-5から5度くらい（もうちょっと少なくてもいけるかもしれない）の制限角度にしないと物理が破綻しやすかった。

		とりあえず実装して軽く流してテストした段階です。検証はこれから。
		人体のそれっぽい制限角度設定をしたlmtファイルを用意するのも今後の課題。

2018/08/11_2
	複数キャラ読み込み時の処理を修正。
		剛体の表示を修正。
		物理シミュがエラーになっていたのを修正。

2018/08/11_1
	FBXの読み込み後にFBXのSceneを削除するようにしました。今までの約半分のメモリ量で動きます。

	既知のバグ
		前回？の仕様変更以降、通常IK時の剛体の表示及び複数キャラクター読み込み時の剛体の処理に不具合があることが分かりました。
		次回以降で直していきます、しばらくお待ちください。

2018/07/22_1
	物理シミュの開始直後に剛体が大きく揺れるのを防止しました。
	物理シミュの選択範囲での再生が出来なくなっていたのを修正しました。

2018/07/21
	物理シミュ、物理IKのときの基準の姿勢をシミュ開始時の姿勢からバインドポーズに変更しました。
		通常IKの基準姿勢もバインドポーズです。
		これにより通常IKのときと物理のときとで同じオイラー角を利用できるようになる予定です。
		（バグが無ければ現在でも同じオイラー角のはずですが、まだ未検証の状態。）

	物理IK時のERPを再び小さくしました。

2018/07/07_1
　　　　　動画　【オイラーグラフ】まめばけ３Dのオイラーグラフ【使い方】
　　　　　　https://youtu.be/s0nHIjjcGdY

　　　　　（物理のときのオイラー角は現在仕様変更中）

2018/07/02_1
	ジョイントを右クリックしたときの回転禁止の処理のデバッグ
		０度に固定ではなくて、現在の角度に固定するように修正。
		PhysicsRotを押してから回転禁止メニューを実行すると機能します。

	ERPを0.9くらいにしないと（デフォルトは1.0）、長時間PhysicsRotを実行した際にモデルが振動して破綻する場合がありました。
		操作する場所にもよるようです。

2018/07/01_1
	PhysicsRotの改良
		物理IK時のERPをメインウインドウのスライダーで設定可能にしました。
		
		今までERPをきわめて小さい値にしていましたが
		これはそうしないと物理IKをオンにしただけで乱れが出たからです。
		しかし、ERPが小さいと物理IKにより関節部分に大きな隙間ができることがありました。
		その状態でBtApplyボタンを押してさらに物理IKをしたときに乱れが出ました。
		今回はERPを最大の１．０にしてもなんとか持ちこたえているようです。（多少揺れますが）
		これは今までの剛体関係のデバッグにより可能になったことだと思います。
		ERP１．０で多少揺れますが、関節間に大きな隙間が出来にくいのでBtApply以降の物理IKも乱れが少ないです。
		
2018/06/23
	PhysicsRotとBtApplyボタンのデバッグ。
		まだ選択範囲の先頭フレームに対してだけですが、物理IKを保存できるようになりました。
		PhysicsRotボタン-->ジョイントドラッグ-->BtApplyボタン-->（さらに操作するならPhysicsRotボタン-->...）

2018/06/19_1
	物理IKがある条件でぐるんぐるん回転してしまう症状が直りました。（ただしBtApplyボタンを押さない場合）
	現在、物理IKはワンドラッグ限定で、ドラッグ後のワンクリックで元の状態に戻ります。

2018/06/07_1
	マニピュレータの座標軸を修正　その２。
		IKによる座標軸からの姿勢の変化分が視覚的にわかるようにした（とりあえずバージョン）。
			デフォルトの座標系をペアレントに。
			親ボーンの向きを水色の線で表示し、赤色のラインからの変化分で姿勢の変化分が分かるようにした。

2018/06/06
	マニピュレータの座標軸を修正。
		通常IKのときと物理IKのときと両方修正。
		カレント座標系をデフォルトの座標系にした。
			将来、制限角度を可視化する際に、
			カレント座標系でカレントボーンの回転が初期状態の時のマニピュレータが必要なので今の仕様に。
	
	ボーン軸を「X軸」に合わせるための座標軸の再計算メニューを追加。			

	Model.cppのPhyicsRot関数を引き続きデバッグ中。
		ソースのコメント参照。

2018/06/02_1
	ボーンを右クリックしたときのメニューに
		ボーンの回転禁止設定をいくつか増やしました。
		物理IKではない普通のIKのときにも回転禁止フラグを参照するようにしました。

		IK伝達禁止ではなく、回転禁止フラグなので
		IK階層数を2以上にして使用する場合、回転禁止のボーンを飛び越えてIK結果を適用することが可能です。
		つまり、角度コンストレイント効果があります。

		物理IKの際にも回転禁止は参照されますが、直接操作した場合や大きい力がかかった場合には回転することがあります。

	PhysicsRot関数が不安定になるケースをコメントに書きました。

	サンプルデータを更新しました。
		Media/TestData_20180531_1

2018/05/28_1
	複数キャラ読み込み時にエラーになる不具合の修正。
		OrgWindowの多重regist不具合の修正。
		タイムライン作成タイミングの修正。

2018/05/23_1
	ツールウインドウのボタンの1回目のクリック位置がずれることがあったのを修正。
		例えば補間ボタンの１回目のクリックはクリック位置がずれて実行されていなかった。

	オイラーグラフの更新関数(UpdateEditedEuler)内で、選択ボーンがない場合には一番親のボーンのグラフを表示するようにした。
		どのボーンもクリックしない状態で（階層ウインドウでは一番親ボーンがデフォルトで選択されている）、
		ツールウインドウの補間ボタンを押した際に、一番親のボーンのオイラーが更新されるようにした。
		（今までは更新されていなかった。）

2018/05/21_1
	ツールボタンのコマンドを実行した際にもオイラーグラフを更新するようにしました。

2018/05/17_2
	マニピュレータの中央ドラッグによるIK処理の改良。
		ボーン位置がマウス位置に決まるようになりました（可動範囲にマウスがある場合）。

2018/05/17_1
	CModel::IKRotateのデバッグと改良中
		IKRotateはマニピュレータの中央をドラッグしたときに呼ばれる（複数フレームを選択していることが前提）。
		IKの階層数やIK伝達率のGUIを操作しながらテストして調整中。
		RokDeBone2のように伸ばして回す方式とは異なる方式で、伸ばさないでも多階層操作できるようになってきたところ。

2018/05/15_1
	オイラーグラフのアップデート
		オイラーグラフにペアレントチェックボックスを追加。
			ペアレントチェックボックスにチェックを入れると選択しているボーンの親のボーンのオイラー角をグラフに表示します。

2018/05/14_2
	オイラーグラフアップデート
		IK操作結果をオイラーグラフに反映するようにしました。

		IK操作は実際には操作しているボーンの親のボーンに姿勢が設定されるので
		現状では結果を確認するには親のボーンをクリックしてグラフを確認します。

2018/05/14_1
	タイムライン操作の調整
		ホイール処理の調整
			Mボタンーー＞ホイールーー＞Mボタン　で時間範囲選択
			ホイールだけ　で時間移動だけ

2018/05/13_1
	タイムライン関係の調整

	stdafx.hに以下の４つのスイッチを追加しました。
		//fpsチェックの時に有効にする。垂直同期をしないモード。
		//DXUT.cpp のCreate()内でm_state.m_OverrideForceVsync = 0;//(通常1)
		//DXUT.cpp のDXUTRender3DEnvironment10()でhr = pSwapChain->Present(0, 0);//(通常はPresent(1, dwFlags);
		//#define SPEEDTEST__

		//ロングタイムラインとオイラーグラフの描画をスキップするときに有効にする。
		//#define SKIP_EULERGRAPH__

		//物理シミュをスキップする。
		//#define SKIP_BULLETSIMU__

		//モーションポイント検索時にキャッシュを有効にする。
		//SPEEDTEST__とSKIP_EULERGRAPH__とSKIP_BULLETSIMU__の３つを有効にした状態で以下を有効にすると効果が分かる。
		//こちらの環境では１２キャラクターのモーション再生時で比べて、５倍から１５倍速になる。
		#define USE_CACHE_ONGETMOTIONPOINT__

2018/05/09_2
	タイムラインのデバッグ
		
2018/05/09
	描画の高速化
		タイムラインとオイラーグラフのSetCurrentTimeが再帰していました。
		再帰を解消したところ高速化しました。

	姿勢編集結果のオイラーグラフへの反映は次回以降にやります。

2018/05/06_1
	オイラーグラフの続き。
		前回の角度を考慮して、角度が連続するようにしました。
		目盛りを付けました。
		プラスが上でマイナスが下になるようにしました。


	姿勢編集結果のオイラーグラフへの反映は次回以降にやります。


2018/05/04_1
	ロングタイムラインを上下2段にして下段にオイラーグラフを描画しました（途中）。
		モーションを読み込んでボーンをクリックするとそのボーンのオイラーグラフが表示されます。
		まだ途中なので
			角度の目盛りはありません。
			最大角度と最小角度に応じて拡大表示しています。
		色はXYZ軸それぞれR、G、Bの色になっています。

2018/04/22_1
	ウインドウタイトル設定。
	ウインドウタイトルと主なメニューに日本語表記と英語表記の両方を設定。
	（UNICODEです。）


2018/04/21_1
	ウインドウ一体化の続き。
		メニューを３Dウインドウからアプリのウインドウに移動。
		３Dウインドウからキャプションを取り除いた。
		少し体裁を整えた。

		物理シミュレーション中に他のウインドウ（メモ帳やタスクマネージャ）をアクティブにしてもfpsが落ちなくなった。

2018/04/17
	ウインドウの一体化開始。
		一体化の途中。
		現在1240 x 830くらいの大きさ。
		ウインドウを増やしたいので1700 x 1000くらいまで大きくなる予定。

2018/04/12_1
	MameBake3DLibにstdafx.hを導入しました。
		MameBake3DLibのビルド時の4005番のワーニングがstdafx.cppでしか出なくなりました。
		MameBake3D本体のほうはstdafx.hの設定をしていないのでプロジェクトの設定で4005番を無視するようにしています。


2018/03/29_2
	制限角度ファイル(fbxファイルと同じディレクトリ内の拡張子がlmtのファイル)を編集しました。
		ツインテール以外は自由に動くようにしました。


2018/03/29
	静的リンク用ライブラリとしてまめばけ３Dの機能をまとめました（MameBake3DLib.lib）。
		まだインターフェイスなどそのままでとりあえずライブラリにしました。
		MameBake3DLib.libをLGPLのライブラリとして使用する他のライセンス形式のアプリ作成が可能です。
			ただしMameBake3DLib.libの内容に手を加える場合にはLGPLになります。

	MameBake3D.exeと同じディレクトリに以下の４つのDLLを置く必要があります。
		libfbxsdk.dll (FBX SDK)
		tbb.dll (Intel TBB)
		tbbmalloc.dll (Intel TBB)
		tbbmalloc_proxy.dll (Intel TBB)

2018/03/07_1
	bvh2FBXのファイル選択ダイアログでbvhの拡張子のフィルターを選択するようにしました。


2018/03/05
	コンボボックスの選択ができなかった不具合を修正。

	DXUT.cppの変更箇所を追記（上記）。
		物理が安定するように垂直同期することにしました。
		描画速度が速すぎて物理が動かなくなる不具合が解消しました。


2018/03/04_1
	開発環境を変えました（上記）。

	ハードウェア環境でメインウインドウのボタンが押せなかった不具合が直りました。
	それに伴い描画速度も速くなりました。

	ブートキャンプ環境にした場合
	物理計算のスレッド数をCPUの数よりも多くするとエラー終了するようである。

2018/02/14_1
	レジストしないと使用できない機能が中途半端な状態になっていました。
	初期状態でレジスト済みにしました。

	レジスト済みの場合はFileメニューのbvh2FBXが使用できます。
		ファイルダイアログのファイルの種類でbvhを選択して倍率を適当に入力してOKボタンを押すと
		bvhファイルと同じディレクトリにFBXファイルが出来ます。
		手元の記録によるとこちらで使用したbvhの場合には倍率は0.294倍にしていました。参考までに。
		この時、まめばけ３Dにはbvhが読み込まれていない状態となっています。
		表示したいときには通常のFBX読み込みで読み込みます。

		
		作成したFBXファイルは形状付きのFBXファイルにリターゲット可能です。
		面倒なので最近テストしてませんが以前試したときにはリターゲットに成功していました。
		そのときの動画もあります。
		何か不具合が見つかった場合は連絡していただければ暇なときに対応するかもしれません。

	モーション再生中でも、ボーンを表示するチェックボックスにチェックが入っている場合には
	ボーンマークを表示するようにしました。


2018/02/13_1
	FBX SDKのマルチスレッドでの利用時に出た問題と解決法のメモをDocumentsフォルダに書きました。


2018/02/12_1
	複数キャラクター読み込み時にエラー終了することがある不具合を修正しました。

	複数キャラクターの物理シミュレーションに関する不具合を修正しました。

	フレーム範囲指定での物理シミュレーションに関する不具合を修正しました。

	動作環境に追記しました。


2018/02/04_1
	フレーム選択範囲の間を繰り返し物理シミュレーションすることが可能になりました。
		物理IKのときの範囲考慮は次回以降対応予定。

	使用しているDirectX SDKのバージョンはMicrosoft DirectX SDK (June 2010)です。

	おちゃっこLAB (https://moo-ochakkolab.ssl-lolipop.jp/)にてバイナリを配布しています。


2018/02/03_1
	フレーム選択の改良をしました。
		アニメーション再生によってフレーム選択が解除されないようにしました。
		マウスで複数フレームを選択後、その同じ選択状態で何回もプレビュー開始と停止が可能です。

2018/01/27_2
	地面の格子の線と軸の線がZファイトでちらついていたのを修正。
		Media/MameMedia/ground2.mqoを修正。

2018/01/27_1
	アニメーションの再生を０フレームからではなく１フレームからにしました。
		物理シミュの開始時にバインドポーズとアニメーション姿勢との差異による大きな揺れが大幅に軽減されました。
		（それでも開始時に少しは揺れます。）

2018/01/24_2
	シェーダー定数へのテクスチャ設定の最適化
		変更があったときのみ設定するようにしました。
		2018/01/24_1で実装済みでしたが忘れた処理があって不完全でした。

2018/01/24_1
	Ctrl + 1 (one) キーでメインウインドウのツール部分の表示がトグルします。
	「表示」メニューの「モーションウインドウ」の選択で
	階層構造エディタとロングタイムラインウインドウの両方の表示がトグルするようにしました。

	（こちらの環境では、）
	Ctrl + 1でツール部を非表示にしてモーションウインドウを非表示にした場合
	9体読み込み後BtStart（物理シミュ開始）後のfpsが35fpsだったのが100fpsになりました。


2018/01/23_1
	DirectX10対応の続き。
		Lineの描画に対応しました。
		Zバッファーテストを常に合格させるステートを作成しマニピュレータ描画時に適用しました。


2018/01/22_1
	DirectX10対応の続き。
		Shader(Media/Ochakko.fx)のテクニック部分でZバッファを有効にしました。

	メインウインドウのスライダーとチェックボックスをクリックしたときにGUIが消える症状を解消しました。
		readme.txt(このファイル)の「外部ライブラリのソースの場所」に追記しました。

2018/01/21_1
	DirectX10への対応を開始。[まだ途中です。]
		とりあえずDX10で動いています。
		シェーダーはfx_4_0です。

		Lineの描画がまだです。
		SetRenderStateの代わりの手段もまだやっていません。
		現在、メインウインドウのGUI部品をクリックすると画面がブリンクするバグがあります。
		tgaファイルの読み込みサポートがなくなったのでリソースの修正も必要です（ボーンの丸マーク）。

	readme.txt(このファイル)の「外部ライブラリのソースの場所」に追記しました。

2018/01/16_1
	readme.txt(このファイル)の「外部ライブラリのソースの場所」に追記しました。

2018/01/15_1
	D3DXをChaVecCalcに更に置き換え。
		DXUT部分にも適用。（適用の仕方はこのテキスト上方の外部ライブラリのソースの場所の部分に書きました）

	CQuaternionとBonePropはChaVecCalcに統合しました。


2018/01/13_2
	2018/01/13_1の更新のソースをGitHubにアップ

2018/01/13_1
	`バイナリ先行公開（おちゃっこLABにて）
		FBX SDK 2018に対応しました。

		Visual Studio 2015に対応しました。

		AMD Bullet PhysicsとIntel TBBもVisual Studio 2015に対応しました。

		Mediaフォルダの中のテストデータを更新しました。
			アニメーションの０フレームはバインドポーズです。
				昔の０フレームがバインドポーズではないテストデータからの移行方法を説明します。
					０フレームの１フレーム区間をマウスで選択します。
					ツールウインドウの姿勢の初期化を押します。
					全ボーンの回転と位置両方を初期化します。
					このままだと０フレームの初期姿勢が不完全です。
					Fileメニューのsaveからプロジェクトを新しい場所に保存します。
					それを読み込みなおします。
					もう一度新しい場所にプロジェクトを保存します。
					移行データの出来上がり。

2018/01/08_1
	D3DX9の代わりのChaVecCalcの更新。
		まめばけ３DではフレームワークにDXUTを使用しているのでそこでのD3DX9の利用は今の段階では続けます。
		D3DX9を利用せざる得ない部分ではChaVector3やChaMatrixクラスのD3DX()関数で変換してます。
		プログラムに「プリプロセッサCONVD3DX9」を定義するとChaVecCalcからD3DX9への変換部分が有効になります。
		
		D3DX9の利用部分を局所化できたので異なるグラフィックライブラリを利用する道が開けたと思います。
		DirectX10や10.1を検討中ですが優先順位的にすぐに取り掛かるかどうかは未定です。

2018/01/07_1
	D3DX(DirectX9の算術サポート)の代わりにChaVecCalc.hとChaVecCalc.cppを用意しました。
		DirectXに渡す部分以外はほとんどChaVecCalcで計算しています。
		描画部分を別DLLにして違う方式(現在はDirectX9です)でも描画できるようにする準備です。


2017/12/30_1
	通常モーション再生、物理シミュレーション、物理IKの高速化
		キャッシュ機能を付けました。
		fpsは倍速になりました。
		１２キャラ同時に物理シミュしてもこちらでは60fps出ています。

2017/12/27
	動画紹介
		動画　【まめばけ３D】物理シミュレーションのマルチスレッド化
　　　　　　　　（今回はしゃべりました）
　　　　　　　　https://youtu.be/cW7TWNiX-uk

		動画　【まめばけ３D】大きな揺れを制限角度で小さな揺れに
		https://youtu.be/3GxO7jPOdlY

		動画　【まめばけ】角度制限ファイルを物理に適用
		https://youtu.be/q0bY6-jNk20

2017/12/23_1
	bullet physicsのファイルへの変更部分を書きました（上記）。

	プロジェクトファイル(chaファイル)の読み込み時にはマウスカーソルを砂時計にしました。

	メインウインドウにスレッド数のスライダーを追加しました。
		＃＃＃＃＃
			スレッド数のスライダーを動かす際にはbtStopボタンを押してシミュレーションを停止してからにしてください。
		＃＃＃＃＃

	６０ｆｐｓのサンプルプロジェクトファイルに
	3chara, 6chara, 9chara, 12charaの複数キャラクター読み込み用のファイルを加えました。
		ロード時間はexeを直に起動したほうが速いです（待ち時間がかなり違います）。
		exeはx64フォルダの１つ上の階層のフォルダにコピーしてからダブルクリックで起動します。
		ｆｐｓが小さくなるとメインウインドウのbtCalcCnt（計算回数）のスライダーの値を大きくする必要があります。
			計算回数を増やさないとツインテールなどの物理部分が不自然に伸びたりします。
			３０ｆｐｓより遅くなると制御は難しいです。
		こちらでテストしたところ、１２キャラクター読み込みをして６４スレッドで計算回数１２回の場合
			３０ｆｐｓほどで動作し、４つのCPUに負荷が分散されました。
			１２キャラの読み込み時間は１分弱でした。



2017/12/22_1
	chaファイルにキャラクターの位置情報を追加。
	サンプルのchaファイルで複数キャラのテスト。
	複数キャラの同時物理シミュレーションのデバッグ。
	
	複数キャラを読み込んだ場合にはモデルメニューでモデルを選択すると選択したモデルのボーンをピック可能になります。


2017/12/17_1
	Bullet Physicsのバージョンを上げてマルチスレッドで利用しました。
		TBB利用などについては上記参照のこと。

		物理シミュレーションがマルチスレッドで動くようになりました。
		こちらの環境で４コアでテストしたところ、デフォルトの状態ではCPU0 --> 70%, CPU1--> 20%, CPU2,3 --> 10%という状況でしたが
		計算回数のスライダーで計算回数を多くしたところ、CPU0-->70%, CPU1,2,3 --> 30%のような利用状態になりました。
		（マルチスレッド版以前のもので調べると、CPU0-->80%, CPU1,2,3-->10%という状態でした。）


2017/12/09_5
	PhysicsRotとPhysicsRotAxisDeltaの姿勢計算を修正しました。
	
	物理再生時の角度制限をオンにしました。

	制限角度はシミュレーション開始時の剛体の軸の角度です。
	（コンストレイントの軸(FrameAの回転)には親の剛体の軸を設定してあります。）

2017/12/09_2
	PhysicsRotAxisDeltaの姿勢計算を修正しました。
		精度が向上しました。

	現在、角度制限は物理IK時のみ機能していて、物理再生時には角度制限をオフにしています。
	（テストのためです）


2017/12/09_1
	PhysicsRotAxisDeltaの姿勢計算を修正しました。
		この関数はPhysicsRotボタンを押してマニピュレータの球もしくはリング部分をドラッグした時に
		軸個別の回転として機能します。
		物理なので他の剛体からの影響も受けますが、だいたい軸個別の回転が出来ます。
		精度が向上しました。

2017/12/03_1
	PhysicsRotとPhysicsRotAxisDeltaの姿勢計算を修正しました。
			
	60fps用のlmtファイルのひじの部分が-60度から60度になっています。	


2017/12/02_1
	PhysicsRotとPhysicsRotAxisDeltaの姿勢計算を修正しました。
		マニピュレータでの軸指定捜査の精度が上がりました。

	６０ｆｐｓのFBX用のlmtファイルには制限角度のテスト数値が設定されています（－１５度から１５度）。



2017/11/27_1
	コンストレイントの軸と剛体の軸の設定を一致させました。
		少し検証が足りていませんが、これでparent modeのマニピュレータの軸と角度制限の軸が一致したと思われます。
	
	PhysicsRotで各軸の操作（PhysicsRotDelta関数）の時にも角度制限が効くようにしました。
		（今まではマニピュレータのセンターをドラッグしたときのみの制限でした）


2017/11/26_1
	bullet physicsのオイラー角計算結果とMameBake3Dのオイラー角計算結果の比較検証をしました。
		いろいろなボーンと姿勢でテストした結果、bullet physicsの回転順序はZYXではなくXYZのようです。
		PhysicsRotの際のbullet physicsの計算結果とQ2EulXYZの結果を比べた結果ほぼ一致しました。
		その誤差は大抵は小数点以下6位のdegreeの誤差程度でした（たまに小数点以下３位の誤差になることもありました）。


2017/11/24_1
	CQuaternion::Q2EulZYXbtの修正とbullet physicsのオイラー角計算との比較
		PhysicsRotのIK時にオイラー角を比較した。
		（Physics Rotでのボーンドラッグ時にVisualStudioの出力タブにデバッグ情報を出力している）

		Q2EulZYXbtの結果とbullet physicsのオイラー角計算結果は概ね一致したが、
		0.2degree程の誤差はあった。場合によっては２degree違う場合もあった。
		Q2EulZYXbtの途中の計算はdoubleで行う必要があるかもしれない。

	60fps用のサンプルのlmtファイルはテストのため全ボーンの制限角度を-10度から10度に設定してあります。
	これは現状ではシミュレーション開始時からの角度として機能します。
		

2017/11/23_1
	物理IK（PhysicsRotボタンを押してボーンをドラッグするIK）のときにも
	「(FBX名).lmt」ファイルの制限角度を考慮するようにしました（途中）。

	現在はシミュレーション開始時からの変化角度分に対して制限角度を適用しています。
	軸は親剛体の座標系です。rollがXです。
	親ボーンとの角度によって制限を掛けることも試してみたので、今後モードの切り替えなどで対応する予定です。

	PhysicsRotを押さないでFKする場合にも制限角度がかかっていますが、こちらはまだ手を入れていない昔の状態です。



2017/11/12_1
	物理シミュレーションへの制限角度導入を開始。
	途中です。
	物理の制限角度はマニピュレータの軸が基準です。
	FKの際の制限角度との整合性はまだ取っていません。

	制限角度設定ファイルはFBXファイル名の後ろに「.lmt」を付けたファイルとなります。
	lmtファイルはプロジェクトファイルの読み込み時に自動的に読み込まれます。


2017/11/06_1
	コンストレイントをコーンツイストからGeneric6DofSpringConstraintに戻しました。
		コーンツイストでデバッグした結果をバネにマージしたような形になったので
		以前よりさらに安定しました。


2017/10/09_1
	剛体パラメータの切り替え周りのデバッグ
		剛体を切り替えてパラメータを変えた場合に、他の剛体へ影響が出ることがあったバグを修正。
		Physics Rot, Physics Mvをすると、剛体のパラメータが変化してしまうことがあるバグを修正。

		（mapのmapの使い方とダイアログ表示のための切り替え処理の修正。）


2017/09/02_1
	マニピュレータモードのデバッグ
		メインウインドウのコンボボックスでCurrent, Parent, Globalで切り替え可能にしました。
		

2017/08/25_1
	マニピュレータ行列の検証とデバッグ
		カレント系の動きを修正

		現在の仕様
			カプセルマット　選択ジョイントの親と選択ジョイントを結ぶベクトルをX軸とする直行座標
			カレント系　カプセルマットに選択ジョイントの行列の変化分を掛けたもの
			ペアレント系（現在コメントアウト）　カプセルマットに選択ジョイントの親のジョイントの行列の変化分を掛けたもの
			初回のジョイントクリック（どれでも）時にカプセルマットを計算


2017/08/23_1
	マニピュレータ行列デバッグ中
		カレント系とペアレント系の動きを確認。
			PhysicsRotAxisDeltaでペアレント系はコメントアウト中。

		カレント系とペアレント系の切り替えは現在動いていない
			カレント系になっている


2017/08/21_2
	FBX保存でアニメーションがおかしくなるバグを修正しました。
	つい最近生じたバグでした。		

2017/08/21_1
	注意！！！
		現在、FBXの保存をするとFBXの基本行列に関する不具合がありましてアニメーションが崩れてしまいます。
		2017/08/20の変更が原因です。
		出来るだけ早いうちに直す予定です。
		CalcAxisMatZとNodeMatのあたりの不具合です。


2017/08/20_2
	マニピュレータの軸をX軸ツイストにしました。
		ジョイントをマウスでクリックした際に（初回のクリック）軸の計算がされます。

	注意
		現在、Physics*ボタンを押してからボーンドラッグまでに時間がかかると
		物理シミュが止まってしまい関節の動きが連携しないことがあります。
		そんなときはボタンを押しなおしてやり直してください。
		DeactivationTime関係の症状だと思いますので、そのうちに直したいと思います。


2017/08/20_1
	剛体の軸を変更しました。
		Y軸ツイストからX軸ツイストに変更しました。
		この変更により、bullet physicsによるConeTwistConstraint->SetLimitを使用しても破綻しなくなりました。

		剛体用の形状ファイル更新。

		マニピュレータの軸はまだ変えていません。

	ツインテール物理シミュレーションが改善しました。
		パラメータ調整（ハードコード）。
		ConeTwistに変更してから一時的に収まりが悪くなっていました。



2017/08/19_1
	角度コントロールの準備としてオイラー角計算を補強しました。
		CQuaternionクラスに
			Q2EelZXY, Q2EulYXZ, Q2EulXYZ関数を追加。
			クォータニオンを3種類の回転順序でオイラー角に変換します。

			動作確認はMameBake3Dの機能でオイラー角に変換したものを
			FBXSDKの機能にてXYZに直して出力したFBXをMayaで読み込んで確認しました。
			アニメーションも正しく再生されました。

		FBX出力機能の変更。
			FBX出力時(BVHからの変換を除く)に最初からXYZの回転順序で処理をしました。


2017/08/12_1
	物理シミュレーションのパラメータをConeTwist用に調整
		３０ｆｐｓでより安定しました

	誤解を招くといけないので、GZero*表記（無重力編集の意）をPhysics*表記に変更


2017/08/10_3
	ReadMe.txt
		動画　【物理コンストレイント】しゃがんだりかかとを上げたり
　　　　　　　　https://youtu.be/JIXG7u1RciU

2017/08/10_2
	ボーン右クリックメニューに「除外MV」メニューを追加

	使用例：
		GZeroMVボタンを押した後に
			つま先とかかとに「PosConstraint」を設定
			ひざとももの付け根に「除外MV」を設定
			カメラをキャラの真横に
			ボーンをどれでも１つつかんで、キャラの前にドラッグしてからキャラの下にドラッグしてさらにキャラの後ろにドラッグ（ゆっくり）

			ーーー＞　足を設置したままキャラがしゃがむ


2017/08/10_1
	コンストレイントを6DofSpringからConeTwistに変更中
		
	無重力IKボタンを押している間
		クリックのたびに開始時のポーズに戻る
		ポーズを確定したい場合にはメインウインドウ上部の「Bt Apply」スプライトをクリック
	
	無重力IKではない通常の編集モードの戻す「STOP Bt」ボタン追加


	ボーン右クリックのメニュー
		「一時Mass0」メニュー　：　一時的に質量をゼロにしてシミュレーション効果を激減させる。
						基本的には動かないが全く動かないというわけではない。

		「Pos Constraint」メニュー　：　位置コンストレイント。位置は動きにくいが角度は変わる。

		使用例：
			GZeroRotボタンを押した後に
				つま先にPosConstraintを設定。
				両方のももの付け根をMass0設定。
				ももの付け根の親のボーンもMass0設定。
				ひざをドラッグ。
				
				ーー＞ひざとももが持ち上がってつま先位置が固定されてかかとが上がる。


	まだダイアログにバネの項目が残っているがこの方針がうまくいけば取り除いていく方向で。


2017/08/03_2
	ReadMe.txt
		動画　【まめばけ３D】編集に使える(or Not?)無重力IK
		https://youtu.be/EU9owDSkDtw

2017/08/03_1
	GZeroMV(無重力k移動)を修正しました。
		編集可能にすることに重きを置いて修正しました。
		GZeroMVでの物理効果はコンストレイント（ボーンを右クリックして出てくるメニューで設定）の部分だけに絞りました。
		物理的な動きは制限されたものの、編集可能になりました。

2017/08/02_1
	GZeroMVを修正しました。
		個々の剛体の移動の計算方法を変更しました。
		ドラッグしている剛体の一番親の剛体まで移動させるようにしました（操作による形崩れを防止するため）

	位置コンストレイントの設定について変更しました。
		ボーンを右クリックしてGZeroConstraintメニューを選択すると
		まだコンストレイント設定していなかった場合は設定され、
		すでにコンストレイント設定していた場合には設定が解除されます。

		この設定のフラグはGZeroMV, GZeroRotボタンを押しても解除されません。
		
		GZeroRot, GZeroMVボタンを押したときにコンストレイントフラグが設定されているボーンに対して
		「ボタンを押したときのボーンの位置」に拘束するように設定します。

2017/07/31_1
	GZeroモードのときのエンドジョイントの扱いのデバッグ
		エンドジョイントをドラッグできるようになりました。

2017/07/30_1
	GZeroRotとGZeroMVのときの剛体タイプをコーンにすることにより
	「操作に対する耐性」が上がりました。

	しかし、素早く操作すると思わぬ結果になり得ることに変わりはありません。


2017/07/24_1
	`ReadMe.txt更新
		動画紹介　【まめばけ３D】そっと位置コンストレイント実験中
　　　　　　　　https://youtu.be/LgX_15AMt1o

	GZeroMVの位置コンストレイントは上記リンクの動画をみるとわかりますが
	ボーン右クリックで行います。
	GZeroMVのボタンを押すとコンストレイントは解除されるので
	「GZeroMVのボタンを押してから」ボーン右クリックでコンストレイント指定します。

	この位置コンストレイントの設定タイミングは仮のものです。


2017/07/22_1
	GZeroRotとGZeroMVの2種類の無重力下での編集を実験中。
		GZeroRotは回転、GZeroMVは移動です。
		メインウインドウにそれぞれのスタートボタンを配置。
		GZeroMVは特にそっとゆっくり操作しないとビジュアルが大変なことになります。

		調整中。


2017/07/18_1
	無重力IKの際にキャラクター全体の向きを考慮していなかったのを修正。
		（通常のIKの際にはすでに考慮していたので、同じ方法を採用。）

	
	動画　【まめばけ３D】パラメータ調整と60fpsでのキャプチャ
　　　　https://youtu.be/4FwFRuiF-D8	


2017/07/15_2
	パラメータ試行錯誤。

	メインウインドウに物理シミュ開始のためのボタンと無重力IK開始のためのボタンを付けました。

	「選択部を注視点にする」チェックボタンをチェックした時には、アニメーションの再生時にもカメラを追尾させるようにしました。


2017/07/15_1
	無重力ラグドール（F10キー）高速化。
		こちらの環境では無重力ラグドール時に30fpsほどだったのが80fpsほどの描画速度になりました。	

	F9キーでの物理シミュがうまくいかなくなっていたのを修正。
		主に剛体の長さ問題。

	既知の問題と課題。
		無重力ラグドールをするとF9のシミュレーション用のバネのパラメータまで変わってしまう。
			(2017/07/18時点で再現せず。勘違いの可能性あり。)
		
		IKに制限角度がない。
		
		無重力IKは回転よりもコンストレイントしながらの移動に力を発揮しそうである（現在実装しているのは回転のみ）。

2017/0710_1
	無重力IKの更新。
	無重力IK開始ボタンをメインウインドウ中央下に配置。
	物理のパラメータを調整したことにより、意図しない動き（何もしていないのにモデルが泳いだり）が激減しました。

2017/07/09_1
	readme.txt更新。
	2017/07/08のアップデートでモーションスピードのデフォルト値が０になっていますが
	これは無重力IKのテストのためです（F10キーで開始）。

	最近の動画を紹介します。
	
	動画　【まめばけ３D】バネモデルIK
　　　　https://youtu.be/puPVYJrn0gs

	動画　【まめばけ３D】30fps物理パフォーマンス改善
	https://youtu.be/lIvNlK7rlqI

	動画　【まめばけ３D】30fpsで破綻しない物理
	https://youtu.be/eLM-ha3nQdI


2017/07/08_1, 2
	`無重力ラグドールIK機能作成スタート。
	F10キーを押すと無重力ラグドール状態になります。
	その状態でボーンの関節部分をドラッグするとボーンのIK回転が可能です。
	これを使用してアニメーションの編集ができるまでにしていく予定です。


2017/06/29_1
	デバッグとMotion2Btの改良。
	低いFPS、少ない計算回数でより安定するようになりました。
	６０ｆｐｓ用のサンプルはbtCalcCnt 1でも安定しています。
	３０ｆｐｓ用のサンプルはbtCalcCnt 3で安定しており、あまり剛体が暴れなくなりました。


2017/06/25_1
	デバッグしました。
	30fpsの環境で実行する際の安定度が少し増しました。
	Mediaフォルダにサンプルを追加しました。

	本テキストに　物理とキーボード　の記述を追加しました。

2017/06/17_1
	剛体設定モードで表示している剛体の位置がずれていたのを修正しました。
	これは、2017/06/13の修正による不具合の解消です。
	MameMediaフォルダの中の３Dデータの原点を修正し、さらに表示時の計算を修正しました。


2017/06/15_1
	Motion2Btの修正。

	60fpsと30fpsで破綻しない設定例のサンプルを追加。
	サンプルフォルダ名にパラメータの値を書いたのでその値を設定してお試しください。



2017/06/13_1
	剛体の原点と変換行列の原点が一致していなかったのを修正。
	
	原点の修正によりbullet physicsの計算回数が少なくて済むようになりました。
	しかし、計算回数や剛体のパラメータなどのバランスは、fpsに依存している状態です。
	例えば、60fpsで再生できる場合にはbtcalccnt 2, motion speed 3で動きますが
	キャプチャソフトを動かすなどして30fpsの場合は、
	btcalccnt 4、 motion speed 2くらいの設定にしないときれいに動きません。

	btcalccnt, erpの設定をメインウインドウのスライダーでできるようにしました。


	こちらで調整した感想ですが
	１フレーム当たりの計算回数(btcalccnt)を１回にするなど小さくする場合
	バネ定数を小さくする必要がありました。
	しかし、バネ定数を小さくすると今度は早い動きで伸びてしまう現象が起こりました。
	この辺のバランスの取り方はこれからの課題かもしれません。

	分かっているバグ
	剛体の原点修正により現在、
	剛体設定モードでDirectXで３D表示される剛体（カプセル形状など）の
	中心位置がずれています。
	（表示のバグ）


2017/06/10
	bullet physicsのバージョンを2.80から2.85に変更しました。
	Motion2Btのデバッグをしてドキュメントも更新しました。
	デバッグにより、今まで物理計算は１フレームにつき６０回していましたが
	５回で済むようになりました。

2017/05/26, 27
	bulletのタイムステップなどの調整とそれに伴うサンプルのパラメータ調整。
	バインドポーズを2重に適用していたバグを修正。
	仕様メモなどのドキュメントの追加。
	本テキストへの追記（bullet physicsのリンクを追加、VisualStudioについて追記, bullet physicsのバージョン）


2016/07/17 その１
	MameBake3D上での法線とMaya上での法線が一致しない原因は、
	裏面があった時(同じメッシュ内に)にMaya上では表と裏の表示が混ざることがあることが原因でした。
	データから裏面を削除してデータを修正しました。
	
	MameBake3DとMayaとで法線の状態が一致しました。

	Media/BtTest160717_8を追加。


2016/07/16 その１
	FBX出力を修正しました。
	以前の出力結果ですと、Mayaで読み込んで法線をエディットする際にMayaがエラーになることがありました。
	修正したFBXですと、Mayaで読み込んでも正常に操作可能です。
	しかし、まだMameBake3D上での法線とMaya上での法線が一致していないようです。
	後程調査します。

	（今までMameBake3Dで保存したFBXも、新しいMameBake3Dで読み込んで出力しなおすことで
	Mayaで正常に操作可能なFBXになります。）

	Media/BtTest160716_5をaddしました。


2016/07/13 その１
	髪の毛の法線がおかしくなっていたのをMayaで直しました。
	バインドポーズも作り直しました。
	
	このバージョンから
	MameBake3Dでは、複数のバインドポーズが見つかった場合、最後のバインドポーズを採用します。

	昨日のcommitのコメントにdebug(rigid null)
	とありましたが、修正はこのバージョンから掛かります。
	(push前だったので、間違ったcommitコメントを打ち直すためにrevertしたのですが、
	そのときのミスで修正がcommitされていませんでした。)

2016/07/12 その１
	F9キーを押してbtシミュレーションをした場合、
	モーションのフレームが最終フレームから最初のフレームに戻った時に
	全体移動量などが大きいことから、物理が破たんすることがありました。

	最終フレームから最初のフレームに戻った時には、物理の処理も最初に戻すようにしました。
	今後、この機能はオプションにする予定です。

	サンプルデータをMedia/BtTest160712_2にバージョンアップしました。
	モーションの最後の方で髪の毛が少しずれることがあります。
	これは顔の影響度と髪の毛の影響度を合わせることで対応予定です。


2016/07/11 その３
	ラグドールの改善。

	剛体設定ウインドウに「回転禁止」チェックボックスを追加。
	通常時の方の剛体の設定でRHip, LHip, RCollar, LCollar, Neckなどを回転禁止にすると、
	ラグドール時に形状破たんしなくなります。

	F9キーを押してから好きなタイミングでF10キーを押すとその時点からラグドールが始まります。
	股の部分などが破たんする状況はなくなりましたが、まだ制限角度なく曲がる状態ですので
	ラグドール開始の姿勢によってはみたくない光景になりますのでご注意してください。

2016/07/11 その２
	【物理が】 MameBake3D 【使えそうな？】
	https://youtu.be/gh_ymrdA8tM

2016/07/11 その１
	CBtObject周りの検証と調整をしました。

	サンプルMedia/BtTest160711_3を追加。
	更に調整しました。
	読み込み後にF9を押すと物理がはじまります。
	F10キーでのラグドールもだいぶ使えそうになってきました。
	しかし、直立に近い姿勢からのラグドールで股の部分に不具合が出ることがあるため、
	変な映像を見たくない人はF10は押さないほうが良いと思います。


2016/07/10 その１
	Bulletを使う周辺のデバッグをしました。
	バグの中にはCBtObjectの階層構造にかかわる部分もありました。

	サンプルにMedia/BtTest160710を足しました。
	読み込んでF9を押すとツインテールの物理が動きます。
	動かす階層を１階層増やしました。
	大きな動きの時にもびよーーんと伸びなくなりました。
	まだ調整の余地がありますが、とりあえずこの辺でpush。


2016/07/08　その２
	CQuaternion::RotationMatrixのソースを検証しました。
	行列をスケールについて正規化する処理を加える必要がありました。
	MakeRotMatXを転置する必要がありました。
	修正を加えてRotationMatrixの計算部分をD3DX非依存にしました。

2016/07/08
	昨日、ネットで見つけたCQuaternion::RotationMatrixのD3DX非依存版を導入して試してみましたが
	今日、自分で検証計算をしてみてどうも合わないので一度ロールバックします。


2016/06/22 その１
	exeファイルをgitからrmしました。
	コンパイル環境がない方で実行ファイルを試したい方は
	info@ochakkolab.moo.jp までメールにてお問合せください。

2016/06/18 その３
	補間機能における内分率を修正。


2016/06/18 その２
	【ガウシアンで】 MameBake3D 【平滑化】
	https://youtu.be/6iUwef1ZiYM

2016/06/18 その１
	平滑化機能のデバッグをしました。
	平滑化のフィルタータイプとフィルターサイズをセットし忘れているバグがありました。
	デバッグにより平滑化が機能するようになりました。
	複数回実行することにより、より効果が現れることを確認しました。


2016/06/17 その５
	【補間で】 MameBake3D 【ノイズ除去】
	https://youtu.be/l4EuXXCwXcw


2016/06/17 その４
	ツールウインドウに「補間」ボタンを追加しました。
	選択フレーム範囲の最初のフレームと最後のフレームの姿勢を保ちつつその間の姿勢を補間します。
	
	Media/RigTest8の足がぴくっとなる部分はbvhのデータの値が５フレーム間ほど１８０度近く変化していることが原因でした。
	このフレーム間、２３２フレーム辺りから６フレームくらいの間に対して補間機能を使用したところ滑らかになりました。
	Media/RigTest8のデータを更新しました。


2016/06/17 その３
	コピーペーストをアップデートしました。
	１、ペーストの際にフレーム範囲を選んでいないときは、カレントのフレームの場所からコピーしたフレーム数分ペーストします。
	２、ペースト先が選んであってコピー元のフレーム数より少ない場合は、ペースト先のフレーム範囲にのみペーストします。
	３、ペースト先が選んであってコピー元のフレーム数より多い場合は、コピー元と同じフレーム数をペーストした後、
		コピー元の最終フレームの姿勢をペースト先の残りのフレームにペーストします。


2016/06/17 その２
	CBone::CalcLocalTraAnimを修正。
	コピーペースト時に２つ目以降のボーンの移動がおかしくなるのを修正。
	
　　メインウインドウの選択ボーンのオイラー角表示の下に選択ボーンの移動成分を表示しました。

	DirectXのデバイス作成時に倍精度のFPUを指定。

2016/06/17 その１
	１フレーム１ボーンの指定でコピーペーストすると姿勢がおかしくなるバグを修正。

2016/06/16 その１
	ツールウインドウに平滑化ボタンを追加。
	Media/TestRig8のモーションの足がぴくっとなる現象の解決策にはなりませんでした。
	平滑化は３種類ありますが、複数回実行しないとあまり効果が出ないこともあります。

2016/06/15 その３
	【RigのRig】 MameBake3D 【peace!】
	https://youtu.be/B_zkbayPECc


2016/06/15 その２
	RigのRigのサンプルを作りました。
	Media/RigTest8/RigTest8.chaを読み込むと指のRigが読み込まれます。
	各指のDistal_endを右クリックすると指を１本ずつ開閉するRigにアクセスできます。
	それぞれの腕のElbowを右クリックすると腕のRigと指を５本まとめて開閉するRig(RigのRigの例)にアクセス可能です。
	Enableのチェックボックスを使用することで、３本ずつでも４本ずつでも(５本以下なら)開閉できます。
	５本まとめて開閉するRigを他のRigの１要素として登録することも可能です。


2016/06/15 その１
	RigのRigを可能にしました。
	Rigの設定画面で通常のボーンのところにRigを指定できます。
	ですのでRigのRigが出来るということは、RigのRigのRigも可能です。
	無限ループ防止にdepth(再帰呼び出し)が１０を超えるとreturnします。

	サンプルデータが必要だと思われるので、後程準備します。

2016/06/14 その７
	更にコピーと対称コピーのデバッグをしました。
	ペーストする際に、ルート以外の親や子供ボーンを除外できるようになりました。
	操作対象ボーンはツールウインドウの「コマンド対象ボーン」ボタンを押して設定します。


2016/06/14 その５
	普通のコピー時(対称コピーではなく)にもルートボーンを除外してペーストした場合に、全体移動が正しくなるようにしました。

2016/06/14 その４
	ツールボタンのモーションのプロパティでモーション長を変えてからCtrl + ZでUndoした場合に
	タイムラインが更新されていなかったのを修正しました。

2016/06/14 その３
	メインウインドウの水色球のスプライトをクリックしてRigモードをオフにした場合に
	CustomRigDlgを閉じるようにしました。

2016/06/14 その２
	ボーンの選択外しが出来るようにしました。
	ボーンの○マーク以外をクリックすると選択が外されてマニピュレータの表示が消えます。
	メインウインドウのGUIパーツやスプライト操作の時には選択を復活させました。

2016/06/14 その１
	ツールウインドウの「姿勢初期化」ボタンにサブサブメニューを付けました。
	サブメニューで、全ボーン、選択ボーン１つだけ、選択ボーンと子供ボーンの中から選ぶと
	サブサブメニューで、回転と移動を初期化、回転を初期化、移動を初期化の中から選ぶことが出来ます。

	CBone::SetWorldMatFromEulにinittraflagを追加。


2016/06/13 その２
	対称コピーの修正。
	対称コピーをして、処理対象ボーン選択でルートを除外してからペーストした場合の処理を修正。
	除外したボーンの子供のボーンのアニメ情報が失われていた不具合が直りました。


2016/06/13 その１
【モーション】 MameBake3D 【対称コピー】動画
https://youtu.be/YeM43To-xCg


2016/06/12 その２
	【リターゲット】 MameBake3D 【改良しました】 動画
　　https://youtu.be/JYT5BAdj5Q4

2016/06/12
	主に移動アニメについてのソースを整理しました。

	モーションのリターゲットの移動アニメを修正しました。
	モーション側とモデル側のスケールが違っていても、モーションの最初のフレームのポーズとモデルのバインドポーズが同じであれば
	移動アニメも含めて再現率の高いリターゲットが可能になりました。


2016/06/11 その１
	複数フレーム選択時のバグを修正。
		ホイールを回してからA D キーのモードで操作しようとすると、以前のホイール量だけフレームが移動してしまうバグを修正。

	Ctrl+Shiftのフレーム選択モードの合図とマウス中ボタンクリックでのフレーム選択モードの合図を混合できるようにしました。

	フレーム選択モード中において
	A Dキーでの選択フレーム移動とマウスホイールでの選択フレーム移動を混合できるようにしました。
	Ctrlを押しながら選択フレーム移動すると１フレームずつ移動します。
	（Ctrlを押しながらではないときには、５フレームずつ移動します。）


2016/06/10 その２
	対称コピーをボーンの位置移動に対応させました。
	
	ルートのボーンに関しては、対称コピーボタンを押したときにメニューを出して
	コピー元と同じ、位置と向き対称、向きだけ対称、位置だけ対称
	から選べるようにしました。

	ルートボーンの位置と向きをペースト先の姿勢のままにしたい場合は
	ペーストする前にツールボタンの「コマンド対象ボーン」でルートの子供のボーンを対象に指定してから
	（つまり、ペースト対象にルートボーンを入れないようにしてから）
	ペーストしてください。


2016/06/10 その１
	対称コピーとペーストがうまくいっていなかったのを修正しました。
	その過程でいろいろデバッグしました。


2016/06/08 その４
	Media/RigTest8を更新。
	BVH2FBXで作成されたFBXをRigTest8の形状にリターゲットしました。
	その際のボーンの対応がわかるスクリーンショットをMedia/RigTest8/MotionRetarget20160608.pngに撮りました。
	モーション途中で足がぴくぴくっとなる部分がありますが、これはモーションキャプチャ(BVH)のノイズによるものと思われます。
	（平滑化フィルター導入の予定があります。）


2016/06/08 その２　その３
	Media/RigTest7を削除してMedia/RigTest8を作成しました。
	Metasequoia 4でボーンと影響度を設定しました。
	(体はBVHテンプレートを元に、指はHumanoidテンプレートを元にしました。)

	リグは
		両足のAnkleと
		両手のElbow_branchと
		両手の各指のDistal_end
		に設定してあります。
		右クリックで呼び出します。


2016/06/08 その１
	ホイール操作時にCtrlを押していた場合に５倍速の操作にしていましたが
	Ctrlを押していた場合にはゆっくりとした操作にすることにしました。
	マウスによる「IK」、「FK」や「カメラの操作」時にもCtrlを押すとゆっくり操作できます。

2016/06/05
	全機能テストしてデバッグしました。
		モーションリターゲットのデバッグ
		絶対IKオプションのデバッグ
		エンドジョイントの姿勢編集のデバッグ
		RigTest6のデータに不具合があったのでRigTest7に差し替え

	既知の問題
		F10キーでのラグドールはまだ調整が足りない。
			（モデルデータの変な状態を見たくない人はF10を押さないほうが良いです。）

		再現する条件が絞り込めていないが、F9での物理シミュレーション時にFPSが極端に下がってカクカクになることがある。

2016/06/03 その３
	マニピュレータデータを更新しました。
	円盤形状を足してから、各色の選択がしずらくなっていました。
	円盤形状をマウスとの衝突判定から除外することで、各色の外側のリングと球を選択しやすくしました。


2016/06/03 その２
	ボーンを右クリックしたときに出るメニューにサブメニューを付けました。
	サブメニューでRig設定かRig実行かを選ぶことが出来ます。
	Rig設定を選んでもRigを操作することは可能です。
	Rig実行のときには設定ダイアログが出ないという違いです。

	その代わり、Rigの設定ダイアログを閉じてもRigモードは終わりません。

	Rigモードと通常モードの切り替えは、メインウインドウのXYZのスプライトの右の「水色の球のスプライト」
	をクリックして切り替えます。


2016/06/01 その２
　　【使い方説明】 MameBake3D 【Rig機能が付いちゃいました】
　　https://youtu.be/gkqCJNsUsKE

2016/06/01 その１
	Rigのテストのためのサンプルデータをaddしました。
	Media/RigTest6/RigTest6.chaです。
	
	両足のFoot（先端から２個目）と両手の先端のボーンにテストのRigを仕込んであります。

	メインウインドウでボーンの○マークを右クリックするか、もしくは左のウインドウのボーン名を右クリックして
	Rig名を選びます。

	そして水色の球をドラッグします。
	足については足を前に上げるためのRigと足を後ろに曲げるためのRigを、それぞれFrontとBackという名前で登録しています。

	自分でパラメータを設定する場合は新規Rigを選んで設定後Applyボタンを押します。


	解説動画を作成予定です。


2016/05/31 その２
	ボーン座標軸回転ダイアログを１度閉じたら次回以降開くことが出来ないバグを修正しました。
	ボーン座標軸の回転実行は、モーション長が長いと時間がかかります。すべてのオイラー角の再計算をするためです。


2016/05/31 その１
　　MameBake3D CustomRig ダイジェスト動画。
　　https://youtu.be/o7UUAqea6io

　　CustomRigを実装しました。GUIで設定可能なRigです。

	準備として「編集・変換」-->「オイラー角　角度制限」メニューでボーンの座標軸をCurrentAxisにします。
	設定するRigに関係するボーンの座標軸を「編集・変換」-->「ボーン座標軸回転」でRigに関係するボーンの座標軸を平行にします。
	Rigに関係するボーンが３つある場合、３のボーンともXYZが一致する必要はありません。
	あるボーンにとってのX軸があるボーンにとってのY軸やZ軸でも構いません。
	しかし、対応付ける軸同士が並行であるほうがきれいに曲がります。
	(現在、ボーン座標軸回転ダイアログは、バグのため一度閉じると再びは開きません。対応予定です。)

	軸を平行にする準備が出来たら
	Rigを設定する一番子供のボーンを右クリックします。もしくは左のウインドウのボーン名を右クリックします。
	そのボーンにまだRigを作成したことがなければ、「新規Rig」だけがメニューに出ます。
	Rigを設定してApplyボタンを押すとRigが登録されて、次回からは右クリックのメニューに作成したRigが出現します。

	Rigの設定ダイアログを出したら
	ボーンの階層数を指定します。
	何個上の階層まで設定するかの指定です。
	自分自身も１と数えるので、設定するボーンとその親のボーンの２個のボーンのRigを作成する場合は、階層数は２を選びます。

	階層数を選ぶとグループボックスに自動的に親方向にさかのぼったボーンの名前が表示されます。

	ボーンの名前が表示されているグループボックス内の設定をします。
	「横」「縦」とは、Rigマークの水色の球を横と縦にマウスでドラッグした時のことです。

	マウスをドラッグした時の回転軸と倍率を指定します。
	倍率はマイナスの数値も許されます。
	マイナスの数値は逆回転します。
	-100.0から100.0までの値が有効です。

	設定したらApplyボタンを忘れずに。

	後は水色の球をマウスでドラッグしながら数値や軸の調整をします。

	設定したデータを保存する場合にはFileメニューのSaveで「プロジェクトファイル」を選択して保存してください。
	開くときは*.chaファイルを開くとプロジェクト全体が読み込まれます。


	追記：
		RigコントロールはCustomRigDlgを開いていないと出来ません。
		CustomRigDlgを右上のｘで閉じると水色の球も消えて通常動作になります。


2016/05/29 その１
	姿勢編集結果を山状に適用する部分のデバッグをしました。
	選択フレーム数が２または３のときに姿勢の編集が出来ないバグを修正。
	applyframeのoffsetは必要ないことを確認。
	適用率の式を修正。

	toolWndの姿勢初期化を選んだ際にメニューを出します。
	メニューで全ボーンの姿勢初期化、選択ボーン１つの姿勢初期化、選択ボーンと子供ボーンの姿勢初期化
	の中から処理を選ぶことが出来ます。


2016/05/28 その２
	アンドゥーの高速化をしました。

2016/05/28 その１
	制限角度設定の保存と読み込み機能を付けました。
	制限角度設定ファイルは「FBXファイル名.fbx.lmt」です。
	FBXファイルの書き出し時またはプロジェクトファイルの書き出し時に保存されます。
	FBXを読み込む際にlmtファイルも読み込まれます。

	プロジェクトファイルには記述しませんが、FBXのファイル名に.lmtを付けたファイルを
	FBXの読み書き時に自動的に読み書きするということです。


2016/05/27
	Angle Limit DlgにVia180_X, Via180_Y, Via180_Zのチェックボックスを付けました。
	Viaとは電車のアナウンスでおなじみの「経由」の意味です。
	通常ならlower <= degree <= upperが可動範囲になるところを
	Via180のチェックを入れると ((-180 <= degree <= lower) || (upper <= degree <= 180))が可動範囲になります。
	つまり180を経由するような逆回りが可動範囲になります。

	ボーンの初期のオイラー角は0度です。
	可動範囲に０度が含まれていない場合、IKやFKで動かすことが出来ないと困るので
	メインウインドウに「回転角度制限をする」チェックボックスを追加しました。
	ボーンを可動範囲に入れるまでは「回転角度制限をする」のチェックをはずして、
	可動範囲に入れてからチェックをして、制限角度内で操作することが可能です。

	今回からWin10でビルドしています。
	VisualStudioは明示的には変更していません。

2016/05/26
	「編集・変換」-->「ボーン座標軸回転」メニューで
	ボーンの座標軸を回転可能にしました。

	長いフレームのモーションを読み込んでいる状態で実行すると
	全てのフレーム、全てのボーンに対してオイラー角の計算をし直すので
	メインウインドウのマニピュレータの向きに反映されるまで少し時間がかかります。

	ボーンの座標軸はFBXファイルに保存されるので
	変更を保存したい場合はFBXファイルの保存をしてください。


2016/05/24 その４
	Angle Limit Dlgにボーン座標系指定のコンボボックスを付けました。
	ボーンごとに座標系の指定が出来ます。
	座標系によって制限角度の掛かり方も変わります。
	
	制限角度設定時「以外」でもボーンを選択すると
	Angle Limit Dlgで指定した座標系に自動的に切り替わります。

	座標系によってメインウインドウ左上のオイラー角表示も変わります。

	メインウインドウの座標系指定コンボボックスは、Angle Limit Dlgで指定した座標系と異なるものも選ぶことが出来ます。
	そしてオイラー角表示はメインウインドウのコンボボックスでの指定に従います。
	しかし、回転の制限角度の掛かり方は、メインウインドウのコンボボックス指定にかかわらず
	Angle Limit Dlgで指定した座標系をもとに制限します。


2016/05/24 その３
	カレント座標系チェックボックスをやめて
	メインウインドウにコンボボックスを追加し、
	CurrentBoneAxis, ParentBoneAxis, GlobalAxis
	の３つの座標系から選択できるようになりました。
	
	選択したボーン座標系でIK, FK操作が可能です。

	角度制限はバインドポーズの座標系で行っています。

2016/05/24 その２
	メインウインドウに「カレント座標系」チェックボックスを追加してデフォルトでチェックしました。
	チェックを入れていない状態ではボーンの座標系はparent座標系(親のボーンの座標系)であり、
	チェックを入れるとcurrent座標系(現在の選択ボーンの座標系)になります。
	制限角度(の軸の把握)との相性はcurrent座標系の方が良い気がしたので、デフォルトをカレント座標系に変えました。


2016/05/24　その１
	Angle Limit Dlgをモードレスダイアログにしました。
	つまり、ダイアログを出してから他のウインドウのコントロールに対して操作可能です。
	スライダーで1度単位の設定をする場合には、スライダーをクリックしてからマウスホイールを回転させると簡単です。
	Angle Limit Dlgの設定はApplyボタンを押さないと正式に反映されないので注意してください。
	
2016/05/23
	「編集・変換」メニューに「オイラー角　角度制限」メニューを追加しました。
	ボーンを選択してからメニューを実行します。
	とりあえずモーダルダイアログです。
	スライダーでLower（下限）とUpper(上限)の角度を設定してApplyボタンを押すと適用されます。
	制限角度は-180度から180度の間で指定します。
	角度の基準はマニピュレータとして表示されている座標軸です。
	Lowerに-180、Upperに180を指定した場合に制限「なし」となります。

	選択ボーンの現在のオイラー角はメインウインドウの左上に表示されます。

	IK, FKの際に指定角度でクランプします。
	XYZどれか１つでも範囲からはみ出しているとIK,FKの際にボーンは動きません。
	（滑って動くタイプではなく、止まるタイプです）

	まだ制限角度の保存と読み込みが付いて「いません」のでテスト程度にしておいてください。


2016/05/19
	メインウインドウに選択ボーンのローカル座標系でのオイラー角を表示しました。
	XYZの回転はZ,X,Yの順番に回転します。
	複数の軸に対して回転を行うと操作中の軸以外の軸の角度が変化することがありますが
	これは回転順序の仕様でありバグではありません。

	もう少し詳しく説明します。
	回転角度の算出はZXYの順番で回転したと仮定して逆計算で求めます。
	それに対して、モーションの編集操作は
	操作している軸の回転が一番最初の回転になるように処理します(効果が直感的に分かるように)。
	ですからある軸の回転により他の軸の回転角度が変化することがあります。


2016/05/18
	操作をしているとアプリケーションエラーで落ちることが多かったのを修正。
	bonecntの後処理、Null Pointerのチェック。
	読み込み、姿勢編集、アンドゥー、fbx保存、モデル削除、読み込み、、、などを繰り返しても落ちなくなりました。

2016/05/17
	メタセコイアデータのZの向きの修正をしました。

	編集-->ボーン軸をZに再計算メニューを実行して保存しなおしてください。
	今後のIK,FKの角度制限設定のためにも、ボーン軸をZに再計算メニューを実行して保存しなおしてください。

2016/05/15
	新データと旧データの切り替えを自動にしました。
	
	旧データ
		データにボーンの座標軸が記録されていないがマニピュレータ表示時に軸を計算して表示していた。
	新データ
		データにボーンの座標軸が記録されており、マニピュレータ表示時にそれを使用する。

	ただし、2016/05/12から2016/05/15のこれより前のバージョン(RokDeBone2も含む)で保存した過渡期のファイルについては
	読み込み時に出る救済ダイアログでチェックボックスにチェックを入れて読み込む必要があります。
	過渡期ファイルである場合には、読み込み後速やかに保存しなおしてください。
	

2016/05/12
	ボーンの回転軸を考慮したバインドポーズに対応しました。
	RokDeBone2でボーンの回転軸に対応したFBXを書き出し、それを読み込むことが出来ます。

	古いRokDeBone2や古いMameBake3Dで出力したFBXを読み込むとボーンの位置がおかしくなることがあります。
	その場合はメインウインドウの右下の旧データ互換のチェックボックスをチェックして
	正常に表示されることを確認してから編集作業をする前にFBXを保存してください。
	新しいMameBake3Dで保存したFBXはボーンの回転軸を考慮したデータになります。
	ですので、それを読み込みなおす場合には旧データ互換のチェックをはずしてください。

	bvh2FBXでのFBXデータはbvhから変換しなおしてください。

2016/05/03
	動画。MameBake3D [新しいフレーム選択]。
	https://youtu.be/CfaFvEL2grM

2016/05/02
	フレーム選択
		「マウス中ボタンクリック-->マウスホイール-->マウス中ボタンクリック」
		で複数フレームを選択しますが、この代替方法を追加しました。
		「Ctrl+Shiftキー --> Aキー(マイナス側)Dキー(プラス側) -->Ctrl+Shiftキー」
		という方法です。
		マウスホイール中やA Dキー中にCtrlキーを押すと選択速度が5倍速になります。

	フレーム選択ヒストリー
		フレーム範囲選択を記憶して呼び出せるようにしました。
		最大１００００「種類」のフレーム選択範囲を記憶します。
		選択ヒストリーは登録隅の選択状態と同じ選択状態を記録しません。
		プレイヤーボタンの下印ボタンで過去方向のヒストリーを復元し、
		上印ボタンで未来方向のヒストリーを復元します。
		新たに登録した選択が解除された状態からの1回目の下印ボタンは現状復帰の機能をします。


2016/04/29
	フレーム選択：ホイールを回すときにCtrlキーを押している場合は選択速度を5倍速にしました。
	FBX出力：モーション長でソートすることにより複数モーション格納のFBXも正しく出力と入力ができるようになりました。
		テキスト形式からバイナリ形式にしました。読み込み速度が速くなりました。
	サンプル：サンプルのFBXをバイナリ形式にしました。

2016/04/27
	操作の見直し
	カメラの操作：メインウインドウの「移」「回」「拡」のスプライトでそれぞれ移動、回転、拡大します。
	フレーム選択：マウス中クリック-->マウスホイール-->マウス中クリック　の3アクションで選択します。
		　　　最初の中クリックはスタートフレームを決めます。クリック位置がスタートになります。
		　　　ホイールで選択範囲が伸びていきます。
		　　　最後の中クリックで選択範囲を確定します。このときのクリック位置は関係しません。

2016/04/25
	アイドリングが長いと剛体のシミュが終了して剛体が体についてこないことがある不具合を修正。
	時間制限を無効にしました。

	(test12のサンプルは読み込みに１０秒近くかかります(Visual Studioから起動しているときはもっとかかります。)。
	FBX書き出しをバイナリにすることを検討中。)

	(複数モーションを書き出して読み込んだ際に、モーションの長さがおかしい場合があります。
	FBX書き出し時にモーションの長さでソートして出力する必要があることを忘れていました。
	対応する予定です。)

2016/04/23
	最近の動画。

	MameBake3D 編集オプションの説明
	https://youtu.be/cA7ceFJ2oLY

	MameBake3D bvhそっと歩き＋物理ツインテ
	https://youtu.be/upwvM6yBGgw

2016/04/22
	Main.cppの相対IKと絶対IKの説明文を修正しました。
	test12の剛体パラメータでサンプルのキックのモーションを読み込んでF9したとき
	ツインテールが最初の位置付近に残りますが、
	これはバグではなくアイドリングタイムと運動停止の関係です。
	つまり長く動かないでいると停止状態になるのです。
	この辺も設定できるようにする予定です。

	その他、タイムラインの数値表示の間隔を調整しました。
	１０００フレームを越える場合も読み取れるようになりました。
	GetBefNextMPの使い方がわかりずらいので、
	置き換えることが出来る部分はGetMotionPointに置き換えました。

	bvhのモーション変換メニュー実行時に
	全体移動量は
	Hipsという名前のボーン限定ではなく、最初のボーンに限定して取得するようにしました。


2016/04/21
	全体回転考慮のオプションを拡張して、親の回転を考慮したIKとFKのオプションとしました。
	名付けて「PseudoLocal(疑似ローカル)」です。
	複数フレームに対する処理なのですが、それぞれのフレームの操作ボーンの親の回転を考慮して
	操作を反映させます。
	メインウインドウの右下に「PseudoLocal(疑似ローカル)」のチェックボックスがあります。
	デフォルトでオンになっています。

2016/04/20
	IK,FK操作結果にルートボーンの回転を考慮するようにしました。
	メインウインドウの右下に「全体回転考慮」というボタンがあります。
	例をあげますと、
	ルートボーンつまり全体が１回転するようなモーションに対する通常のIK,FK操作では
	前向き時に操作した結果がグローバル効果として横向き時にも適用されてしまいます。
	つまり前向き時に足を広げる操作をすると横向き時には足がクロスしてしまいます。
	「全体回転考慮」にチェックを入れると
	前向きのときに足を開く操作をすると横向きの時にも足を開く効果があります。

	マニピュレータの向きのバグを直しました。
	マニピュレータの軸の向きと回転操作の軸は一致します。
	RotateAxisDelta関数はshadow方式から座標系変換方式に変えました。	

2016/04/18
	タイムラインの「<<」ボタンで先頭フレームへジャンプ。
	タイムラインの「>>」ボタンで最終フレームへジャンプ。
	タイムラインの「| |」のマークのボタンを押すと現在のフレームから最後のフレームまでを選択します。
	選択した後に適用位置の％スライダーの値に対応するフレームに移動します。
	InitMPを変更しました。
	test10の剛体パラメータファイルrefファイルを修正しました。

2016/04/14
	F9を押したときのBtシミュレートが動かなくなっていたのを修正。
	bvhモーション＋btシミュ用のサンプルを追加。MameBake3D/Media/test12。
	test12.chaをOpenメニューで開いてF9を押すとbvhの歩き＋btシミュが再生されます。
	test12のパラメータでは、メインウインドウのmotion speedは2.5くらいまでが限界かもしれません。
	スピードが速くなると体の向きが１８０度変わるところでツインテールがぶるぶる震えます。
	ぷるぷる震えたらスペースキーを押してモーションを再生しながらbtをリセットすることも可能です。


2016/04/13
	bvhのコンバートテスト用のデータを追加しました。
	MameBake3D/Media/testBvhConv　です。
	
2016/04/12
	FBX読み込み時に一番親のボーンの位置が正しくなかったのを修正しました。

	BVHモーションコンバートを更新しました。
	bvh側のルートボーン指定をHipsに戻しました。

	BVHモーションコンバートを使用する際には
	BVHの最初のフレームのポーズと、モデル側の初期姿勢が同じ必要があります。
	一番はまりやすいのは、一番親のボーンの位置だと思います。
	BVHのモーションを適用していない初期姿勢は原点にありますが
	BVHのモーションを適用した初期姿勢はLHip,RHipと同位置にあることが多いようです。
	そのためモデル側のボーンを用意する際にはモデル側の一番親のボーンの位置を
	LHip, RHipと同位置にしたほうが良いようです。
	これを間違えると体全体の回転の中心がずれた変換結果になることが多いです。


2016/04/11
	BVHモーションコンバートのバージョンアップ。
	初期姿勢変換にbvh側の全体回転を考慮するようにしました。
	bvh側のルートボーン指定をHips_bunki***に限定する（想定している）ようにしています。
	以前の動画では、BVHモーションコンバートの対応ボーン指定時に
	最低限の指定をしたほうが良いと言っていましたが、すべてに指定したほうが良いようになっています。


2016/04/10
	BVHモーションコンバートのバージョンアップ。
	bvhとモデル側の位置や拡大率を合わせなくてもうまく変換できるようになりました。

2016/04/09
	BVHモーションコンバートのバージョンアップ。
	BVHの１フレーム目にモーションが設定してあっても、
	１フレーム目の姿勢とモデルの初期姿勢が同じであれば変換できるようになりました。

2016/04/07 
	法線をスムージングしてあるデータを読み込んだ場合のFBX書き出しをスムージングに対応させました。
	bvh用のFBXサンプルは準備中です。

2016/02/15 19:00頃のバージョン
	BVH2FBXで作成したデータをMakeBake3Dで読み込むことが出来、分岐ボーンも改善された。
	そのことに起因するかは調べ切れていないが
	ボーンのリストの選択と３Dのマニピュレータの選択がずれるようになっているのに気が付いた。
	以降のバージョンで修正予定である。

2016/02/15 19:50
	ボーンのリストとマニピュレータのずれは直った。
	BVH2FBXメニュー実行時の書き出し時に大きすぎる変化をカットするフィルターが付いているのであるが
	データによって可変であるべきことが分かった。
	以降のバージョンで修正予定である。

2016/02/16 19:00頃のバージョン
	分岐処理見直しと修正。
	bvhの回転順序の適用。
	ルートの回転移動修正。
	バインドポーズ修正。
	回転順序変換部分追加。
	Siteという同一名によるリスト選択のバグを修正。

2016/02/17 01:10頃のバージョン
	回転順序　再調整。
	bvh rotation filterの改良。
	BVH2FBXのダイアログにフィルター角度エディット欄を追加。

2016/02/17 09:45頃のバージョン
	BVH2FBXのオイラー角を抜本的に修正。
		再現率がかなりあがりました。
	
2016/02/17 11:20頃のバージョン
	複数のFBXを読み込んだ時のボーン表示のための機能を追加。
		複数読み込んだ後にCtrl + Aを押すと、読み込みモデルすべてのボーンを表示します。
		Ctrl + Aを押すたびにオンオフを切り替えます。
		普段はModelメニューで選択したモデルにしかボーンは表示されません。
		Ctrl + Aが有効の時には一番親からのボーンは表示されません。

2016/03/13
	モデルのFBXにモーションのFBXを適用する機能の追加。
	モデルとモーションとのボーン構造が多少異なっていても対応できます。

	モデルのボーンの位置にモーションのボーンの位置を合わせるような手作業は必要ありません。
	（初期姿勢が同じ必要はあります。例えばT字型の姿勢であるとか。）

	使い方の説明動画は
		https://youtu.be/ERaqn16GsU0



/////////// 以下、説明動画

【モーキャプツール】EditMot 1.0.0.31【１分紹介】
https://youtu.be/3Azcq_we_B0

【告知】EditMot 1.0.0.30【モーキャプ編集ソフト】
動画リンク
https://youtu.be/1hE2d4g_Ijw

動画　EditMot アップデート2022/09
https://youtu.be/4J-sD2t_qG8

動画　EditMotの使いどころの使い方
https://youtu.be/1Fa7Y5Ojidk

動画　EditMotの基本操作
https://youtu.be/ZHDBf-QSjl4

動画　MotionBrushFreeフリーウェア化して公開開始
https://youtu.be/qFGGUuA21O8

2021/07/07
動画【モーションツール】ver1.0.0.7完成版のいいところ【完成しますた】
https://youtu.be/nApvd9___mQ

2021/06/19
動画【MotionBrush】リターゲット結果を簡単修正【流し込み】
https://youtu.be/AEMJ_Yp5Fig

2021/06/11
動画【MotionBrush】リターゲットバッチ処理機能付きました【ver1.0.0.3】
https://youtu.be/YW5fu4SdU1k

2021/05/01_3
動画【物理シミュのベイク】MameBake3Dで物理をベイク【フレーム範囲指定OK】
https://youtu.be/E-w6tpksu4I

2021/04/30
動画【物理IK】MameBake3Dの物理IKが保存可能に【位置コンストレイント】
https://youtu.be/EOqRf35o7u8

2021/04/05_3
動画【MameBake3D】モーションツールをゲームパッドで動かしてみた【DualSence】
https://youtu.be/AdJPZ3Uv6-c

2021/02/24_2
動画【カエルボタン】プレートメニューでGUIを切り替え【プレートメニュー】
https://youtu.be/GW0OVv7_0SA

2021/02/22_2
動画【MameBake3D】煩雑なGUIをスイッチでオンオフ【プレートスイッチ】
https://youtu.be/DhcikN88D5A

2021/02/06
動画【MameBake3D】モーションにアクセントを【モーションブラシ】
https://youtu.be/WqB03hMqbgc

2021/02/02_2
動画　
【MameBake3D】モーションをブラシで盛る！！【モーションブラシ】
https://youtu.be/rZFTNVLYXPU
(公開終了しました)

【MameBake3D】相対的編集とはどういうことかについて【PseudoLocal Edit】 
複数フレームに対する編集だから	モデルの向きが変わったら
編集結果がおかしくなるんじゃないかという
疑問に応える動画
PseudoLocalと呼ぶ技術で解決しています
https://youtu.be/iyQS7ZPEQLk
(公開終了しました)

【MameBake3D】エディットスケール【編集の山】
https://youtu.be/uqMUoQo7egE
(公開終了しました)

【物理IK】質量０とポスト処理による位置コンストレイント
（1分くらいのテスト動画）
https://youtu.be/q9z_DhzZQYU

【オイラーグラフ】まめばけ３Dのオイラーグラフ【使い方】
https://youtu.be/s0nHIjjcGdY

【まめばけ３D】物理シミュレーションのマルチスレッド化
（今回はしゃべりました）
https://youtu.be/cW7TWNiX-uk

動画　【まめばけ３D】大きな揺れを制限角度で小さな揺れに
https://youtu.be/3GxO7jPOdlY

【まめばけ】角度制限ファイルを物理に適用
https://youtu.be/q0bY6-jNk20

【物理コンストレイント】しゃがんだりかかとを上げたり
https://youtu.be/JIXG7u1RciU

【まめばけ３D】編集に使える(or Not?)無重力IK
https://youtu.be/EU9owDSkDtw

【まめばけ３D】そっと位置コンストレイント実験中
https://youtu.be/LgX_15AMt1o

まめばけ３D】バネモデルIK
https://youtu.be/puPVYJrn0gs

【まめばけ３D】30fps物理パフォーマンス改善
https://youtu.be/lIvNlK7rlqI

【まめばけ３D】30fpsで破綻しない物理
https://youtu.be/eLM-ha3nQdI

【物理が】 MameBake3D 【使えそうな？】
https://youtu.be/gh_ymrdA8tM

【ガウシアンで】 MameBake3D 【平滑化】
https://youtu.be/6iUwef1ZiYM

【補間で】 MameBake3D 【ノイズ除去】
https://youtu.be/l4EuXXCwXcw

【RigのRig】 MameBake3D 【peace!】
https://youtu.be/B_zkbayPECc

【モーション】 MameBake3D 【対称コピー】動画
https://youtu.be/YeM43To-xCg

【リターゲット】 MameBake3D 【改良しました】 動画
https://youtu.be/JYT5BAdj5Q4

【使い方説明】 MameBake3D 【Rig機能が付いちゃいました】
https://youtu.be/gkqCJNsUsKE

MameBake3D CustomRig ダイジェスト動画。
https://youtu.be/o7UUAqea6io

MameBake3Dの使い方。ベイクの話。
ベイク済とは。ベイク済モーションの編集方法について。
https://youtu.be/Z0UacTlXs7w

MameBake3D 編集オプションの説明
https://youtu.be/cA7ceFJ2oLY

MameBake3D [新しいフレーム選択]。
https://youtu.be/CfaFvEL2grM

MameBake3D説明動画。モデルFBXにモーションFBXを適用する話。
https://youtu.be/ERaqn16GsU0

MameBake3Dの使い方。BVHからFBXへの変換の話。
オイラー角出力を抜本的に見直した結果、モーション再現率がかなり上昇しました。
perfumeの３人が踊る動画。
https://youtu.be/ONnB1jDVi6k

MameBake3D bvhそっと歩き＋物理ツインテ
https://youtu.be/upwvM6yBGgw

MameBake3Dの使い方。
(MameBake3DでbvhをFBXにしてMayaでモデルにバインドして
再びMameBake3Dで読み込み、アニメーションを修正するまでの説明動画)
http://youtu.be/EjGnHuXPZm0


MameBake3Dでの物理の設定の仕方。
http://youtu.be/56kIA5OsZ0M

///////////////////
サンプルについて
2016/03/30

Mediaフォルダの中にtest10を入れました。
test10.chaをオープンすると全部読み込まれます。
モーションの選択で「For_F9_Test」という名前のモーションを選んでから
F9を押すと物理シミュレートが動きツインテールが揺れます。
F10を押すと同じく物理でラグドールします。
バグが取れてきているので以前よりはかなり扱いやすくなっています。

2016/03/31

test10の中のfbxファイルのボーンはメタセコイアのBVHテンプレートの階層を少しいじって使用したものです。
先ほど気が付いたのですが、一般的なBVHと比べて足りないボーンがあるようです。
現在のmain.cppのConvBoneRotation関数は、モデル側のボーンがbvh側よりも多い時にはうまくいきますが
モデル側のボーンの方が少ない時にはうまくいきません。
モデルデータのアップデートをする予定ですが、少し時間がかかるかもしれません。
古いfbxもgitには含まれているはずですので、そちらで試してもらうのも良いかもしれません。


2016/04/08 18:30

bvhの適用がうまくいくものとうまくいかないものがありました。
うまくいかない原因は上記では、ボーンが足りないせいだということにしていました。
しかし、その後の調査でbvhの最初のフレームにもモーションが記述してある場合があることがわかりました。
つまり、初期姿勢が想定していたものと異なっていたのです。
（最初のフレームの姿勢は足を平行にしていたが、モーションを削除してみたら足は大の字だった。）
次の更新はこの辺の変換をしてみようと思います。

