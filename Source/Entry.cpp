//########################################################################################
//
//					チョピン バスター for PSP(2009/11/19)
//								  Entry File
//
//				Copyright(C) 2009 ichicat.com All right reserved.
//							http://pspd.ichicat.com
//
//########################################################################################

//****************************************************************************************
//	インクルード等 準備
//****************************************************************************************

// ライブラリ
#include "ichicatpsplib.h"

// 画像ファイル情報
extern unsigned char CharImg[];
#define TEXSIZE 512

// 音声ファイル情報
static unsigned int size_bomb_wav = 225338;
extern unsigned char bomb_wav[];
static unsigned int size_shoot_wav = 208954;
extern unsigned char shoot_wav[];

// アプリケーション情報
PSP_MODULE_INFO( "ModelApp", 0, 1, 1 );
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define ENM_NUM 10
#define TIME_LEFT 120
#define START_WAIT 240
#define TITLE 0
#define GAME 1


//****************************************************************************************
//	エントリポイント
//****************************************************************************************
int main(int argc, char* argv[]){


	// 初期化 *********************************************************
	SetupCallbacks();
	void* fbp0 = GetStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	ScreenInitM(fbp0);

	// パッドの初期化 -------------------------------------------------
	PadM Pad;
	Pad.Init();


	// ゲームで使う変数・構造体の初期化 *****************************
	int i=0;		// ループカウンター用
	srand(time(0));

	// タイトル
	int TitleTime=0;
	SpriteM Title;
	Title.Init(22*16,32,0,19*16,CharImg,TEXSIZE);
	Title.Pos.x=240;
	Title.Pos.y=80;
	int TitleAnmSwithch=0;

	SpriteM StartButton;
	StartButton.Init(16*16,16,0,21*16,CharImg,TEXSIZE);
	StartButton.Pos.x=240;
	StartButton.Pos.y=220;

	SpriteM ichicat;
	ichicat.Init(22*16,16,0,22*16,CharImg,TEXSIZE);
	ichicat.Pos.x=240;
	ichicat.Pos.y=256;


	// 自機の初期化
	SpriteM MyChar;
	MyChar.Init(32,64,160,0,CharImg,TEXSIZE);
	MyChar.Pos.x=240;
	MyChar.Pos.y=200;

	// 弾
	SpriteM Bullet;
	Bullet.Init(3,5,128,0,CharImg,TEXSIZE);
	Bullet.Pos.x=-100;
	Bullet.Scale.x=Bullet.Scale.y=2;
	int BulletLife=0;

	// 弾発射音
	int hShoot[4] = {
		SoundInit(size_shoot_wav),
		SoundInit(size_shoot_wav),
		SoundInit(size_shoot_wav),
		SoundInit(size_shoot_wav)
	};
	int ShootNum=0;
	int buttonCircleUp=0;

	// 爆発アニメーション
	SpriteM bombAnm[6];
	for(i=0;i<6;i++){
		bombAnm[i].Init(48,48,i*48,128,CharImg,TEXSIZE);
		bombAnm[i].Pos.x=-100;
	}
	int bombAnmCount=6 , bombAnmWait=0;
	

	// 爆発音
	int hBomb[4] = {
		SoundInit(size_bomb_wav),
		SoundInit(size_bomb_wav),
		SoundInit(size_bomb_wav),
		SoundInit(size_bomb_wav)
	};
	int bombNum=0;

	// 敵
	SpriteM enm[10];
	int enmLife[10]={0};
	for(i=0;i<ENM_NUM;i++){
		enm[i].Init(32,32,0,176,CharImg,TEXSIZE);
		enm[i].Pos.x=rand()%480;
		enm[i].Pos.y=-16;
		if(rand()%100==1){enmLife[i]=1;}else{enmLife[i]=0;}
	}
	SpriteM enmAnm;
	enmAnm.Init(32,32,0,176+32,CharImg,TEXSIZE);

	// 数値表示
	SpriteM num[10];
	for(i=0;i<10;i++){
		num[i].Init(16,32,i*16,240,CharImg,TEXSIZE);
		num[i].Pos.y=-16;
	}
	int TimeLeft=TIME_LEFT , TimeLeftWait=0;		// 残り時間
	
	int enmDeath=0;			// 撃墜数

	// ライフバー
	SpriteM LifeGauge;
	LifeGauge.Init(10,4,161,272,CharImg,TEXSIZE);
	LifeGauge.Pos.y=16;
	LifeGauge.Scale.x=TIME_LEFT/10;

	// GAME START
	int StartWait=0;
	float CountScale[4]={10,10,10,10};
	SpriteM Start;
	Start.Init(160,32,160,240,CharImg,TEXSIZE);

	// GAME SET
	SpriteM GameSet;
	GameSet.Init(160,32,0,272,CharImg,TEXSIZE);
	GameSet.Pos.x=240;
	GameSet.Pos.y=136;
	int ResultTime=0;

	int Scene=TITLE;


	// メインループ ****************************************************
	while( ENDFLUG==0 ){

		// 入力処理 ---------------------------------------------
		Pad.Get();		//パッド値を取得

		switch(Scene){

			case TITLE:

				enm[0].Pos.x=enmAnm.Pos.x=240;
				enm[0].Pos.y=enmAnm.Pos.y=165;

				if( Pad.Start ){
					MyChar.Pos.x=240;
					MyChar.Pos.y=200;
					MyChar.Angle=0;
					TimeLeft=TIME_LEFT;//残り時間
					TimeLeftWait=0;
					enmDeath=0;//撃墜数
					Bullet.Pos.x=-100;
					BulletLife=0;
					for(i=0;i<ENM_NUM;i++)	enm[i].Pos.y=-16;
					StartWait=0;
					for(i=0;i<4;i++) CountScale[i]=10;
					ResultTime=0;
					Scene=GAME;
				}

				break;

			// Game Main
			case GAME:
				// 再スタート処理
				if( Pad.Start && TimeLeft<=0)	{
					MyChar.Pos.x=240;
					MyChar.Pos.y=200;
					MyChar.Angle=0;
					TimeLeft=TIME_LEFT;//残り時間
					TimeLeftWait=0;
					enmDeath=0;//撃墜数
					Bullet.Pos.x=-100;
					BulletLife=0;
					for(i=0;i<ENM_NUM;i++)	enm[i].Pos.y=-16;
					StartWait=0;
					for(i=0;i<4;i++) CountScale[i]=10;
					ResultTime=0;
				}

				if(StartWait<START_WAIT)StartWait++;

				// ゲームの処理
				if(TimeLeft>0 && StartWait>=START_WAIT*0.8){
					if(TimeLeftWait>9){
						TimeLeft--;
						TimeLeftWait=0;
					}else{
						TimeLeftWait++;
					}


					// トリガ			
					if( Pad.Rtrigger ) MyChar.Angle+=0.05f;	// Rキー
					if( Pad.Ltrigger ) MyChar.Angle-=0.05f;	// Lキー

					// ↑キー
					if( Pad.Up ){
						MyChar.Pos.x+=sinf(MyChar.Angle*-1)*-2;
						MyChar.Pos.y+=cosf(MyChar.Angle*-1)*-2;
					}

					// ↓キー
					if( Pad.Down )	{
						MyChar.Pos.x-=sinf(MyChar.Angle*-1)*-2;
						MyChar.Pos.y-=cosf(MyChar.Angle*-1)*-2;
					}

					// →キー
					if( Pad.Right )	{
						MyChar.Pos.x+=sinf((MyChar.Angle-(3.14*1.5))*-1)*-1;
						MyChar.Pos.y+=cosf((MyChar.Angle-(3.14*1.5))*-1)*-1;
					}

					// ←キー
					if( Pad.Left )	{
						MyChar.Pos.x+=sinf((MyChar.Angle-(3.14*0.5))*-1)*-1;
						MyChar.Pos.y+=cosf((MyChar.Angle-(3.14*0.5))*-1)*-1;
					}

					// 外に出れない
					if( MyChar.Pos.x < 16    ) MyChar.Pos.x+=2;
					if( MyChar.Pos.x > 480-16) MyChar.Pos.x-=2;
					if( MyChar.Pos.y < 16    ) MyChar.Pos.y+=2;
					if( MyChar.Pos.y > 272-16) MyChar.Pos.y-=2;

					// ○ボタン　弾発射処理
					if( (Pad.Circle) && BulletLife==0 && buttonCircleUp==0){
						Bullet.Angle = MyChar.Angle;
						Bullet.Pos.x  = MyChar.Pos.x;
						Bullet.Pos.y  = MyChar.Pos.y;
						BulletLife=1;
						SoundPlay( hShoot[ShootNum] , shoot_wav );
						ShootNum = (ShootNum+1)&3;
						buttonCircleUp=1;
						MyChar.Pos.x-=sinf(MyChar.Angle*-1)*-2;
						MyChar.Pos.y-=cosf(MyChar.Angle*-1)*-2;
					}
					if(Pad.Circle){}else{
						buttonCircleUp=0;
					}

					// 弾発射中
					if(BulletLife==1){
						Bullet.Pos.x+=sinf(Bullet.Angle*-1)*-5;
						Bullet.Pos.y+=cosf(Bullet.Angle*-1)*-5;
					}
					if( Bullet.Pos.x<-5 || Bullet.Pos.x>485 || Bullet.Pos.y<5 || Bullet.Pos.y>277 )BulletLife=0; //画面外に出たら弾は死ぬ
					if( BulletLife==0)	Bullet.Pos.x=-100;	//弾が死んだら見えない場所へ

					// 敵処理 -----------------------------------
					for(i=0;i<ENM_NUM;i++){

						// 敵生まれる
						if(enmLife[i]==0){
							enm[i].Pos.x=rand()%480;
							if(rand()%100 == 1 )enmLife[i]=1;
						}

						// 敵移動処理
						if(enmLife[i]>0)enm[i].Pos.y+=rand()%4;

						// 下までいったら死ぬ
						if(enm[i].Pos.y>272+16){
							enmLife[i]=0;
							enm[i].Pos.y=-16;
						}

						// 敵と弾の当たり判定
						if(enm[i].Pos.x-16<Bullet.Pos.x && enm[i].Pos.x+16>Bullet.Pos.x && enm[i].Pos.y-16<Bullet.Pos.y && enm[i].Pos.y+16>Bullet.Pos.y){
							bombAnm[0].Pos.y=enm[i].Pos.y;
							bombAnm[0].Pos.x=enm[i].Pos.x;
							bombAnmCount=0;

							enmLife[i]=0;
							enm[i].Pos.y=-16;
							BulletLife=0;
							enmDeath+=2;
							SoundPlay( hBomb[bombNum] , bomb_wav );
							bombNum = (bombNum+1)&3;

						}

						// 敵と自機の当たり判定
						if(enm[i].Pos.x-16<MyChar.Pos.x+16 && enm[i].Pos.x+16>MyChar.Pos.x-16 && enm[i].Pos.y-16<MyChar.Pos.y+16 && enm[i].Pos.y+16>MyChar.Pos.y-16){
							bombAnm[0].Pos.y=enm[i].Pos.y;
							bombAnm[0].Pos.x=enm[i].Pos.x;
							bombAnmCount=0;

							enmLife[i]=0;
							enm[i].Pos.y=-16;
							enmDeath+=1;
							TimeLeft-=10;
							SoundPlay( hBomb[bombNum] , bomb_wav );
							bombNum = (bombNum+1)&3;
						}

						if(TimeLeft<0)TimeLeft=0;

					}


				}else if(TimeLeft<=0){
					ResultTime++;
					if(ResultTime>180)Scene=TITLE;
				}
				break;

		}

		// 描画処理 ---------------------------------------------
		ScreenStart();			// ここ↓↓↓に描画を入れる

		switch(Scene){

			// TITLE 描画
			case 0:
				Title.Render();
				if(rand()%16==0) TitleAnmSwithch=(TitleAnmSwithch+1)&1;
				if(TitleAnmSwithch==0){
					enm[0].Render();//敵
				}else{
					enmAnm.Render();//敵
				}
				TitleTime=(TitleTime+1)&31;
				if(TitleTime>8) StartButton.Render();
				ichicat.Render();
				break;


			// GAME 描画
			case 1:
				Bullet.Render();						// 弾
				MyChar.Render();						// 自キャラ
				for(i=0;i<ENM_NUM;i++)	enm[i].Render();//敵

				if(bombAnmCount>=0 && bombAnmCount<6){
					bombAnm[bombAnmCount].Render();//爆発
					bombAnmWait=(bombAnmWait+1)&1;
					if(bombAnmWait==1){
						bombAnmCount++;
						if(bombAnmCount<6){
							bombAnm[bombAnmCount].Pos.x=bombAnm[bombAnmCount-1].Pos.x;
							bombAnm[bombAnmCount].Pos.y=bombAnm[bombAnmCount-1].Pos.y;
						}
					}
				}

				if(StartWait<START_WAIT){

					if(StartWait<START_WAIT*0.25){
						num[3].Pos.x=240;
						num[3].Pos.y=136;
						num[3].Scale.x=num[3].Scale.y=CountScale[3];
						num[3].Render();
						CountScale[3]-=0.1f;
					}

					if(StartWait>START_WAIT*0.25 && StartWait<START_WAIT*0.5){
						num[2].Pos.x=240;
						num[2].Pos.y=136;
						num[2].Scale.x=num[2].Scale.y=CountScale[2];
						num[2].Render();
						CountScale[2]-=0.1f;
					}

					if(StartWait>START_WAIT*0.5 && StartWait<START_WAIT*0.75){
						num[1].Pos.x=240;
						num[1].Pos.y=136;
						num[1].Scale.x=num[1].Scale.y=CountScale[1];
						num[1].Render();
						CountScale[1]-=0.1f;
					}

					if(StartWait>START_WAIT*0.75 && StartWait<START_WAIT){
						Start.Pos.x=240;
						Start.Pos.y=136;
						Start.Render();
					}
				}

				if(StartWait>=START_WAIT*0.9){
					for(i=0;i<4;i++)num[i].Scale.x=num[i].Scale.y=1.0f;

					// 残り時間
					for(i=0;i<10;i++)num[i].Pos.x=10+8;
					num[TimeLeft/100].Pos.y=10+16;
					num[TimeLeft/100].Render();
					num[TimeLeft/100].Pos.y=-16;

					for(i=0;i<10;i++)num[i].Pos.x=10+8+16;
					num[((TimeLeft/10)-((TimeLeft/10)/10)*10)].Pos.y=10+16;
					num[((TimeLeft/10)-((TimeLeft/10)/10)*10)].Render();
					num[((TimeLeft/10)-((TimeLeft/10)/10)*10)].Pos.y=-16;

					for(i=0;i<10;i++)num[i].Pos.x=10+8+32;
					num[TimeLeft-((TimeLeft/10)*10)].Pos.y=10+16;
					num[TimeLeft-((TimeLeft/10)*10)].Render();
					num[TimeLeft-((TimeLeft/10)*10)].Pos.y=-16;

					// 撃墜数
					for(i=0;i<10;i++)num[i].Pos.x=450;
					num[enmDeath/10].Pos.y=10+16;
					num[enmDeath/10].Render();
					num[enmDeath/10].Pos.y=-16;

					for(i=0;i<10;i++)num[i].Pos.x=450+16;
					num[enmDeath-((enmDeath/10)*10)].Pos.y=10+16;
					num[enmDeath-((enmDeath/10)*10)].Render();
					num[enmDeath-((enmDeath/10)*10)].Pos.y=-16;

					for(i=0;i<10;i++)num[i].Pos.y=-16;

					LifeGauge.Pos.x=64+(float)TimeLeft;
					LifeGauge.Scale.x=(float)TimeLeft/5;
					LifeGauge.Render();

				}

				// GAME SETを表示
				if(TimeLeft<=0) GameSet.Render();
				break;

		}

		fbp0 = ScreenRender();	// 描画ここ↑↑↑まで

	}
	// メインループここまで ********************************************

	sceGuTerm();

	//解放 
	//sceAudioChRelease(hShoot); 

	sceKernelExitGame();
	return 0;
}

